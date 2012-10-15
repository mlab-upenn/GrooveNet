/***************************************************************************
 *   Copyright (C) 2005, Carnegie Mellon University.                       *
 *   Maintained by: Daniel Weller                                          *
 *                  Rahul Mangharam                                        *
 *                  and the rest of the GrooveNet Team                     *
 *                                                                         *
 *   Email: dweller@ece.cmu.edu or rahulm@ece.cmu.edu                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "TIGERProcessor.h"

#include "Logger.h"

#include <unistd.h>
#include <fcntl.h>
#include <qfileinfo.h>
#include <qdir.h>

#define STREETNAME_SIZE 30
#define STREETTYPE_SIZE 4

void WriteMemory(const void * pMem, int fd, const unsigned int length)
{
	write(fd, pMem, length);
}

void WriteCoords(const Coords * src, int fd)
{
	WriteMemory(&src->m_iLong, fd, sizeof(long));
	WriteMemory(&src->m_iLat, fd, sizeof(long));
}

void WriteRect(const Rect * src, int fd)
{
	WriteMemory(&src->m_iLeft, fd, sizeof(long));
	WriteMemory(&src->m_iTop, fd, sizeof(long));
	WriteMemory(&src->m_iRight, fd, sizeof(long));
	WriteMemory(&src->m_iBottom, fd, sizeof(long));
}

void WriteAddressRange(const AddressRange * src, int fd)
{
	WriteMemory(&src->iFromAddr, fd, sizeof(unsigned short));
	WriteMemory(&src->iToAddr, fd, sizeof(unsigned short));
	unsigned int tmpZip = ((unsigned int)src->iZip) & 0x7fffffff;
	if (src->bOnLeft)
		tmpZip |= 0x80000000;
	WriteMemory(&tmpZip, fd, sizeof(unsigned int));
}

void WriteString(const char * src, int fd, const unsigned int length)
{
	write(fd, src, (length + 1) * sizeof(char));
}

bool ReadMemory(void * pMem, int fd, const unsigned int length)
{
	ssize_t sz = read(fd, pMem, length);
	return (sz == (int)length);
}

bool ReadCoords(Coords * dest, int fd)
{
	if (!ReadMemory(&dest->m_iLong, fd, sizeof(long))) return false;
	return ReadMemory(&dest->m_iLat, fd, sizeof(long));
}

bool ReadRect(Rect * dest, int fd)
{
	if (!ReadMemory(&dest->m_iLeft, fd, sizeof(long))) return false;
	if (!ReadMemory(&dest->m_iTop, fd, sizeof(long))) return false;
	if (!ReadMemory(&dest->m_iRight, fd, sizeof(long))) return false;
	return ReadMemory(&dest->m_iBottom, fd, sizeof(long));
}

bool ReadAddressRange(AddressRange * dest, int fd)
{
	unsigned int tmpZip;
	if (!ReadMemory(&dest->iFromAddr, fd, sizeof(unsigned short))) return false;
	if (!ReadMemory(&dest->iToAddr, fd, sizeof(unsigned short))) return false;
	if (!ReadMemory(&tmpZip, fd, sizeof(unsigned int))) return false;
	dest->bOnLeft = ((tmpZip & 0x80000000) == 0x80000000);
	dest->iZip = (int)(tmpZip & 0x7fffffff);
	return true;
}

void ReadString(char * dest, int fd, const unsigned int destLength)
{
	unsigned int index;
	for(index = 0; index < destLength - 1; index++) {
		if (read(fd, dest + index, sizeof(char)) < (int)sizeof(char)) break;
		if (dest[index] == 0) return;
	}
	dest[index] = 0;
}

TIGERProcessor::TIGERProcessor(){
	m_nRecords = 0;
	m_pRecords = NULL;
	ProcessingWay = NULL;
}

TIGERProcessor::~TIGERProcessor(){
	Cleanup();
}

void TIGERProcessor::Cleanup()
{
	unsigned int iRec;


	for (iRec = 0; iRec < m_nRecords; iRec++)
	{
		if (m_pRecords[iRec].pAddressRanges != NULL) delete[] m_pRecords[iRec].pAddressRanges;
		if (m_pRecords[iRec].pFeatureNames != NULL) delete[] m_pRecords[iRec].pFeatureNames;
		if (m_pRecords[iRec].pFeatureTypes != NULL) delete[] m_pRecords[iRec].pFeatureTypes;
		if (m_pRecords[iRec].pShapePoints != NULL) delete[] m_pRecords[iRec].pShapePoints;
		if (m_pRecords[iRec].pVertices != NULL) delete[] m_pRecords[iRec].pVertices;
	}


	if (m_pRecords != NULL) {
		delete[] m_pRecords;
		m_pRecords = NULL;
	}
	printf("m_pRecords cleared\r\n");
/*	if(ProcessingWay[0].pShapePoints != NULL) delete[] ProcessingWay[0].pShapePoints;
	if(ProcessingWay[0].pAddressRanges != NULL) delete[] ProcessingWay[0].pAddressRanges;
	if(ProcessingWay[0].pFeatureNames != NULL) delete[] ProcessingWay[0].pFeatureNames;
	if(ProcessingWay[0].pFeatureTypes != NULL) delete[] ProcessingWay[0].pFeatureTypes;
	if(ProcessingWay != NULL)
	{
		delete ProcessingWay;
		ProcessingWay = NULL;
	}
	printf("ProcessingWay cleared\r\n");
*/
	m_osmWayIDtoRecord.clear();
	m_mapCoordinateToVertex.clear();
	m_WaterPolygons.clear();
	m_mapTLIDtoRecord.clear();
	m_mapPolyIDtoRecords.clear();
	m_mapAdditionalNameIDtoRecord.clear();
	m_Strings.clear();
	m_mapStringsToIndex.clear();
	m_Vertices.clear();
	vShapePoints.clear();
	m_mapNodeIDandCoord.clear();
	vAdditionalWays.clear();
	m_nRecords = 0;
}

bool TIGERProcessor::LoadType1(const QString & strFilename)
{
	FILE * hFile;
	char szLine[2048];
	int nNewRecords;
	MapRecord * psOldSegments = NULL;

	hFile = fopen(strFilename, "rb");
	if (!hFile)
	{
		g_pLogger->LogError("GrooveNet - Error", QString("Error opening %1 (type 1)!").arg(strFilename));
		return true;
	}

	nNewRecords = 0;
	fgets(szLine, sizeof(szLine), hFile);
	while (!feof(hFile))
	{
		nNewRecords++;
		fgets(szLine, sizeof(szLine), hFile);
	}

	if (m_nRecords)
	{
		psOldSegments = m_pRecords;
	}
	m_pRecords = new MapRecord[m_nRecords + nNewRecords];
	if (m_nRecords)
	{
		memcpy(m_pRecords, psOldSegments, sizeof(MapRecord) * m_nRecords);
		delete psOldSegments;
	}

	fseek(hFile, 0, SEEK_SET);
	fgets(szLine, sizeof(szLine), hFile);
	int iTLID, iCFCCCode, iFL, iTL, iFR, iTR, iZipL, iZipR, i, n = 0;
	char szFeatureName[STREETNAME_SIZE + 1], szFeatureRecordType[STREETTYPE_SIZE + 1], szCFCC[5];
	RecordTypes eRT;
	AddressRange * psOldAddressZipRanges;
	while (!feof(hFile))
	{
		iTLID = atoi(ExtractField(szLine, 6, 15));

		strcpy(szFeatureName, ExtractField(szLine, 20, 49));
		m_pRecords[m_nRecords].pFeatureNames = new unsigned int[1];
		m_pRecords[m_nRecords].pFeatureNames[0] = AddString(QString(szFeatureName).stripWhiteSpace());

		strcpy(szFeatureRecordType, ExtractField(szLine, 50, 53));
		m_pRecords[m_nRecords].pFeatureTypes = new unsigned int[1];
		m_pRecords[m_nRecords].pFeatureTypes[0] = AddString(QString(szFeatureRecordType).stripWhiteSpace());

		m_pRecords[m_nRecords].nFeatureNames = 1;

		strcpy(szCFCC, ExtractField(szLine, 56, 58));
		eRT = RecordTypeDefault;
		iCFCCCode = atoi(szCFCC + 1);
		switch(*szCFCC)
		{
			case 'A': // roads
			case 'P': // provisional roads
				eRT = RecordTypeTwoWaySmallRoad; // default to small roads

				if (iCFCCCode >= 11 && iCFCCCode <= 14)
					eRT = RecordTypeTwoWayHighway;
				if (iCFCCCode >= 15 && iCFCCCode <= 18)
					// FIXME: one way highways are all messed up in the tiger/line files
					// don't bother counting them as one way
					eRT = RecordTypeTwoWayHighway;
				if (iCFCCCode >= 19 && iCFCCCode <= 24)
					eRT = RecordTypeTwoWayPrimary;
				if (iCFCCCode >= 25 && iCFCCCode <= 28)
					// FIXME: ditto here
					eRT = RecordTypeTwoWayPrimary;
				if (iCFCCCode >= 29 && iCFCCCode <= 29)
					eRT = RecordTypeTwoWayPrimary;

				if (iCFCCCode >= 31 && iCFCCCode <= 34)
					eRT = RecordTypeTwoWayLargeRoad;
				if (iCFCCCode >= 35 && iCFCCCode <= 38)
					// FIXME: here too
					eRT = RecordTypeTwoWayLargeRoad;
				if (iCFCCCode >= 39 && iCFCCCode <= 39)
					eRT = RecordTypeTwoWayLargeRoad;

				if (iCFCCCode >= 45 && iCFCCCode <= 48)
					// FIXME: and here
					eRT = RecordTypeTwoWaySmallRoad;

				// traffic circle, seems to listed in the correct direction
				if (iCFCCCode >= 62 && iCFCCCode <= 62)
					eRT = RecordTypeOneWaySmallRoad;
				// ramp
				if (iCFCCCode >= 63 && iCFCCCode <= 63)
					eRT = RecordTypeTwoWaySmallRoad;

				break;

			case 'B': // railroads
				eRT = RecordTypeRailroad;
				break;

			case 'D': // landmark
				eRT = RecordTypeLandmark;
				break;

			case 'E': // physical feature
				eRT = RecordTypePhysicalFeature;
				break;

			case 'F': // county lines and such
				eRT = RecordTypeInvisibleMiscBoundary;

				if (iCFCCCode >= 10 && iCFCCCode <= 12)
					eRT = RecordTypeInvisibleLandBoundary;
				break;

			case 'H': // water
				eRT = RecordTypeWater;
				if (iCFCCCode >= 70 && iCFCCCode <= 79)
					eRT = RecordTypeInvisibleWaterBoundary;
				break;

		}

		if(eRT == RecordTypeInvisibleLandBoundary)
		{
		//	printf("RecordTypeInvisibleLandBoundary detected\r\n");
		}
		m_pRecords[m_nRecords].eRecordType = eRT;
		m_pRecords[m_nRecords].nVertices = 0;
		m_pRecords[m_nRecords].pVertices = NULL;
		m_pRecords[m_nRecords].nAddressRanges = 0;
		m_pRecords[m_nRecords].pAddressRanges = NULL;

		iFL = atoi(ExtractField(szLine, 59, 69));
		iTL = atoi(ExtractField(szLine, 70, 80));
		iFR = atoi(ExtractField(szLine, 81, 91));
		iTR = atoi(ExtractField(szLine, 92, 102));
		iZipL = atoi(ExtractField(szLine, 107, 111));
		iZipR = atoi(ExtractField(szLine, 112, 116));

		if (iFL || iTL || iZipL)
		{
			psOldAddressZipRanges = m_pRecords[m_nRecords].pAddressRanges;
			m_pRecords[m_nRecords].pAddressRanges = new AddressRange[m_pRecords[m_nRecords].nAddressRanges + 1];

			for (i = 0; i < m_pRecords[m_nRecords].nAddressRanges; i++)
				m_pRecords[m_nRecords].pAddressRanges[i] = psOldAddressZipRanges[i];

			m_pRecords[m_nRecords].pAddressRanges[i].iFromAddr = iFL;
			m_pRecords[m_nRecords].pAddressRanges[i].iToAddr = iTL;
			m_pRecords[m_nRecords].pAddressRanges[i].iZip = iZipL;

			if (m_pRecords[m_nRecords].nAddressRanges)
				delete psOldAddressZipRanges;

			m_pRecords[m_nRecords].nAddressRanges++;
		}
		if (iFR || iTR || iZipR)
		{
			psOldAddressZipRanges = m_pRecords[m_nRecords].pAddressRanges;
			m_pRecords[m_nRecords].pAddressRanges = new AddressRange[m_pRecords[m_nRecords].nAddressRanges + 1];

			for (i = 0; i < m_pRecords[m_nRecords].nAddressRanges; i++)
				m_pRecords[m_nRecords].pAddressRanges[i] = psOldAddressZipRanges[i];

			m_pRecords[m_nRecords].pAddressRanges[i].iFromAddr = iFR;
			m_pRecords[m_nRecords].pAddressRanges[i].iToAddr = iTR;
			m_pRecords[m_nRecords].pAddressRanges[i].iZip = iZipR;

			if (m_pRecords[m_nRecords].nAddressRanges)
				delete psOldAddressZipRanges;

			m_pRecords[m_nRecords].nAddressRanges++;
		}

		m_pRecords[m_nRecords].pShapePoints = new Coords [2];
		m_pRecords[m_nRecords].pShapePoints[0].m_iLong = atoi(ExtractField(szLine, 191, 200));
		m_pRecords[m_nRecords].pShapePoints[0].m_iLat = atoi(ExtractField(szLine, 201, 209));
		m_pRecords[m_nRecords].pShapePoints[1].m_iLong = atoi(ExtractField(szLine, 210, 219));
		m_pRecords[m_nRecords].pShapePoints[1].m_iLat = atoi(ExtractField(szLine, 220, 228));
		m_pRecords[m_nRecords].nShapePoints = 2;

		if (m_pRecords[m_nRecords].eRecordType == RecordTypeInvisibleLandBoundary || m_pRecords[m_nRecords].eRecordType == RecordTypeInvisibleMiscBoundary || m_pRecords[m_nRecords].eRecordType == RecordTypeInvisibleWaterBoundary) { // update county lines
			for (i = 0; i < m_pRecords[m_nRecords].nShapePoints; i++) {
			//	printf("finding boundingRect\r\n");
				if (!areaLeft)
					areaLeft = new Coords(m_pRecords[m_nRecords].pShapePoints[i]);
				else if (areaLeft->m_iLong > m_pRecords[m_nRecords].pShapePoints[i].m_iLong)
					*areaLeft = m_pRecords[m_nRecords].pShapePoints[i];
				if (!areaRight)
					areaRight = new Coords(m_pRecords[m_nRecords].pShapePoints[i]);
				else if (areaRight->m_iLong < m_pRecords[m_nRecords].pShapePoints[i].m_iLong)
					*areaRight = m_pRecords[m_nRecords].pShapePoints[i];
				if (!areaTop)
					areaTop = new Coords(m_pRecords[m_nRecords].pShapePoints[i]);
				else if (areaTop->m_iLat < m_pRecords[m_nRecords].pShapePoints[i].m_iLat)
					*areaTop = m_pRecords[m_nRecords].pShapePoints[i];
				if (!areaBottom)
					areaBottom = new Coords(m_pRecords[m_nRecords].pShapePoints[i]);
				else if (areaBottom->m_iLat > m_pRecords[m_nRecords].pShapePoints[i].m_iLat)
					*areaBottom = m_pRecords[m_nRecords].pShapePoints[i];
			}
		}

		m_pRecords[m_nRecords].bWaterL = false;
		m_pRecords[m_nRecords].bWaterR = false;

		m_mapTLIDtoRecord[iTLID] = m_nRecords;

		m_nRecords++;
		n++;
		fgets(szLine, sizeof(szLine), hFile);
	}
	fclose(hFile);
	return false;
}

bool TIGERProcessor::LoadType2(const QString & strFilename)
{
	FILE * hFile;
	char szLine[2048];

	hFile = fopen(strFilename, "rb");
	if (!hFile)
	{
		g_pLogger->LogError("GrooveNet - Error", QString("Error opening %1 (type 2)!").arg(strFilename));
		return true;
	}

	fgets(szLine, sizeof(szLine), hFile);
 	int iRecord, nPoints, iPoint, iTLID;
	Coords sPoints[10], sEndPoint, * psNewPoints;
	bool bDone = false;
	while (!feof(hFile))
	{
		iTLID = atoi(ExtractField(szLine, 6, 15));

		if (m_mapTLIDtoRecord.find(iTLID) != m_mapTLIDtoRecord.end())
		{
			bDone = false;
			iRecord = m_mapTLIDtoRecord[iTLID];
			for (nPoints = 0; nPoints < 10 && !bDone; nPoints++)
			{
				sPoints[nPoints].m_iLong = atoi(ExtractField(szLine, 19 + 19 * nPoints, 28 + 19 * nPoints));
				sPoints[nPoints].m_iLat = atoi(ExtractField(szLine, 29 + 19 * nPoints, 37 + 19 * nPoints));

				if (sPoints[nPoints].m_iLong == 0 && sPoints[nPoints].m_iLat == 0) {
					bDone = true;
					break;
				}
			}
			if (nPoints > 0) {
				psNewPoints = new Coords [m_pRecords[iRecord].nShapePoints + nPoints];
				if (m_pRecords[iRecord].nShapePoints > 0) {
					memcpy(psNewPoints, m_pRecords[iRecord].pShapePoints, sizeof(Coords) * m_pRecords[iRecord].nShapePoints);
					delete[] m_pRecords[iRecord].pShapePoints;
				}
				m_pRecords[iRecord].pShapePoints = psNewPoints;

				sEndPoint = m_pRecords[iRecord].pShapePoints[m_pRecords[iRecord].nShapePoints - 1];

				for (iPoint = 0; iPoint < nPoints; iPoint++)
				{
					m_pRecords[iRecord].pShapePoints[m_pRecords[iRecord].nShapePoints - 1] = sPoints[iPoint];
					m_pRecords[iRecord].nShapePoints++;
				}
				m_pRecords[iRecord].pShapePoints[m_pRecords[iRecord].nShapePoints - 1] = sEndPoint;

				if (m_pRecords[iRecord].eRecordType == RecordTypeInvisibleLandBoundary || m_pRecords[iRecord].eRecordType == RecordTypeInvisibleMiscBoundary || m_pRecords[iRecord].eRecordType == RecordTypeInvisibleWaterBoundary) { // update county lines
					for (iPoint = 0; iPoint < m_pRecords[iRecord].nShapePoints; iPoint++) {
						if (!areaLeft)
							areaLeft = new Coords(m_pRecords[iRecord].pShapePoints[iPoint]);
						else if (areaLeft->m_iLong > m_pRecords[iRecord].pShapePoints[iPoint].m_iLong)
							*areaLeft = m_pRecords[iRecord].pShapePoints[iPoint];
						if (!areaRight)
							areaRight = new Coords(m_pRecords[iRecord].pShapePoints[iPoint]);
						else if (areaRight->m_iLong < m_pRecords[iRecord].pShapePoints[iPoint].m_iLong)
							*areaRight = m_pRecords[iRecord].pShapePoints[iPoint];
						if (!areaTop)
							areaTop = new Coords(m_pRecords[iRecord].pShapePoints[iPoint]);
						else if (areaTop->m_iLat < m_pRecords[iRecord].pShapePoints[iPoint].m_iLat)
							*areaTop = m_pRecords[iRecord].pShapePoints[iPoint];
						if (!areaBottom)
							areaBottom = new Coords(m_pRecords[iRecord].pShapePoints[iPoint]);
						else if (areaBottom->m_iLat > m_pRecords[iRecord].pShapePoints[iPoint].m_iLat)
							*areaBottom = m_pRecords[iRecord].pShapePoints[iPoint];
					}
				}
			}
		}
		fgets(szLine, sizeof(szLine), hFile);
	}
	fclose(hFile);
	return false;
}

bool TIGERProcessor::LoadType4(const QString & strFilename)
{
	FILE * hFile;
	char szLine[2048];

	hFile = fopen(strFilename, "rb");
	if (!hFile)
	{
		g_pLogger->LogError("GrooveNet - Error", QString("Error opening %1 (type 4)!").arg(strFilename));
		return true;
	}

	fgets(szLine, sizeof(szLine), hFile);
	int iTLID, iRecord, nNewNames, iID;
	while (!feof(hFile))
	{
		iTLID = atoi(ExtractField(szLine, 6, 15));

		if (m_mapTLIDtoRecord.find(iTLID) != m_mapTLIDtoRecord.end())
		{
			iRecord = m_mapTLIDtoRecord[iTLID];

			for (nNewNames = 0; nNewNames < 5 && atoi(ExtractField(szLine, 19 + nNewNames * 8, 26 + nNewNames * 8)); nNewNames++)
			{
				iID = atoi(ExtractField(szLine, 19 + nNewNames * 8, 26 + nNewNames * 8));
				m_mapAdditionalNameIDtoRecord[iID].push_back(iRecord);
			}
		}
		fgets(szLine, sizeof(szLine), hFile);
	}
	fclose(hFile);
	return false;
}

bool TIGERProcessor::LoadType5(const QString & strFilename)
{
	FILE * hFile;
	char szLine[2048];

	hFile = fopen(strFilename, "rb");
	if (!hFile)
	{
		g_pLogger->LogError("GrooveNet - Error", QString("Error opening %1 (type 5)!").arg(strFilename));
		return true;
	}

	fgets(szLine, sizeof(szLine), hFile);
	unsigned int iEntry;
	int iID, iRecord, i;
	unsigned int * pszOldFeatureNames, * pszOldFeatureRecordTypes;
	char szFeatureName[STREETNAME_SIZE + 1];
	char szFeatureRecordType[STREETTYPE_SIZE + 1];
	while (!feof(hFile))
	{
		iID = atoi(ExtractField(szLine, 11, 18));
		for (iEntry = 0; iEntry < m_mapAdditionalNameIDtoRecord[iID].size(); iEntry++)
		{
			iRecord = m_mapAdditionalNameIDtoRecord[iID][iEntry];
			pszOldFeatureNames = m_pRecords[iRecord].pFeatureNames;
			pszOldFeatureRecordTypes = m_pRecords[iRecord].pFeatureTypes;

			m_pRecords[iRecord].pFeatureNames = new unsigned int[m_pRecords[iRecord].nFeatureNames + 1];
			m_pRecords[iRecord].pFeatureTypes = new unsigned int[m_pRecords[iRecord].nFeatureNames + 1];

			for (i = 0; i < m_pRecords[iRecord].nFeatureNames; i++)
			{
				m_pRecords[iRecord].pFeatureNames[i] = pszOldFeatureNames[i];
				m_pRecords[iRecord].pFeatureTypes[i] = pszOldFeatureRecordTypes[i];
			}

			strcpy(szFeatureName, ExtractField(szLine, 21, 50));
			strcpy(szFeatureRecordType, ExtractField(szLine, 51, 54));

			m_pRecords[iRecord].pFeatureNames[i] = AddString(QString(szFeatureName).stripWhiteSpace());
			m_pRecords[iRecord].pFeatureTypes[i] = AddString(QString(szFeatureRecordType).stripWhiteSpace());

			m_pRecords[iRecord].nFeatureNames++;

			delete pszOldFeatureNames;
			delete pszOldFeatureRecordTypes;
		}
		fgets(szLine, sizeof(szLine), hFile);
	}
	fclose(hFile);
	return false;
}

bool TIGERProcessor::LoadType6(const QString & strFilename)
{
	FILE * hFile;
	char szLine[2048];

	hFile = fopen(strFilename, "rb");
	if (!hFile)
	{
		g_pLogger->LogError("GrooveNet - Error", QString("Error opening %1 (type 6)!").arg(strFilename));
		return true;
	}

	fgets(szLine, sizeof(szLine), hFile);
	int iTLID, iRecord, iFL, iTL, iFR, iTR, iZipL, iZipR, i;
	AddressRange * psOldAddressZipRanges;
	while (!feof(hFile))
	{
		iTLID = atoi(ExtractField(szLine, 6, 15));

		if (m_mapTLIDtoRecord.find(iTLID) != m_mapTLIDtoRecord.end())
		{
			iRecord = m_mapTLIDtoRecord[iTLID];

			iFL = atoi(ExtractField(szLine, 19, 29));
			iTL = atoi(ExtractField(szLine, 30, 40));
			iFR = atoi(ExtractField(szLine, 41, 51));
			iTR = atoi(ExtractField(szLine, 52, 62));
			iZipL = atoi(ExtractField(szLine, 67, 71));
			iZipR = atoi(ExtractField(szLine, 72, 76));

			if (iFL || iTL || iZipL)
			{
				psOldAddressZipRanges = m_pRecords[iRecord].pAddressRanges;
				m_pRecords[iRecord].pAddressRanges = new AddressRange[m_pRecords[iRecord].nAddressRanges + 1];

				for (i = 0; i < m_pRecords[iRecord].nAddressRanges; i++)
					m_pRecords[iRecord].pAddressRanges[i] = psOldAddressZipRanges[i];

				m_pRecords[iRecord].pAddressRanges[i].iFromAddr = iFL;
				m_pRecords[iRecord].pAddressRanges[i].iToAddr = iTL;
				m_pRecords[iRecord].pAddressRanges[i].iZip = iZipL;

				if (m_pRecords[iRecord].nAddressRanges)
					delete psOldAddressZipRanges;

				m_pRecords[iRecord].nAddressRanges++;
			}
			if (iFR || iTR || iZipR)
			{
				psOldAddressZipRanges = m_pRecords[iRecord].pAddressRanges;
				m_pRecords[iRecord].pAddressRanges = new AddressRange[m_pRecords[iRecord].nAddressRanges + 1];

				for (i = 0; i < m_pRecords[iRecord].nAddressRanges; i++)
					m_pRecords[iRecord].pAddressRanges[i] = psOldAddressZipRanges[i];

				m_pRecords[iRecord].pAddressRanges[i].iFromAddr = iFR;
				m_pRecords[iRecord].pAddressRanges[i].iToAddr = iTR;
				m_pRecords[iRecord].pAddressRanges[i].iZip = iZipR;

				if (m_pRecords[iRecord].nAddressRanges)
					delete psOldAddressZipRanges;

				m_pRecords[iRecord].nAddressRanges++;
			}

		}
		fgets(szLine, sizeof(szLine), hFile);
	}
	fclose(hFile);
	return false;
}

bool TIGERProcessor::LoadTypeI(const QString & strFilename)
{
	FILE * hFile;
	char szLine[2048];

	hFile = fopen(strFilename, "rb");
	if (!hFile)
	{
		g_pLogger->LogError("GrooveNet - Error", QString("Error opening %1 (type I)!").arg(strFilename));
		return true;
	}

	fgets(szLine, sizeof(szLine), hFile);
	unsigned int iTLID, iRecord, iPolyIDL, iPolyIDR;
	std::map<unsigned int, unsigned int>::iterator recTLID;
	std::map<unsigned int, std::list<std::pair<unsigned int, bool> > >::iterator polyID;
	while (!feof(hFile))
	{
		iTLID = atoi(ExtractField(szLine, 11, 20));
		recTLID = m_mapTLIDtoRecord.find(iTLID);

		if (recTLID != m_mapTLIDtoRecord.end())
		{
			iRecord = recTLID->second;

			iPolyIDL = atoi(ExtractField(szLine, 46, 55));
			iPolyIDR = atoi(ExtractField(szLine, 61, 70));

			polyID = m_mapPolyIDtoRecords.insert(std::pair<unsigned int, std::list<std::pair<unsigned int, bool> > >(iPolyIDL, std::list<std::pair<unsigned int, bool> >())).first;
			polyID->second.push_back(std::pair<unsigned int, bool>(iRecord, false));
			polyID = m_mapPolyIDtoRecords.insert(std::pair<unsigned int, std::list<std::pair<unsigned int, bool> > >(iPolyIDR, std::list<std::pair<unsigned int, bool> >())).first;
			polyID->second.push_back(std::pair<unsigned int, bool>(iRecord, true));
		}

		fgets(szLine, sizeof(szLine), hFile);
	}
	fclose(hFile);
	return false;
}

bool TIGERProcessor::LoadTypeP(const QString & strFilename)
{
	FILE * hFile;
	char szLine[2048];

	hFile = fopen(strFilename, "rb");
	if (!hFile)
	{
		g_pLogger->LogError("GrooveNet - Error", QString("Error opening %1 (type P)!").arg(strFilename));
		return true;
	}

	fgets(szLine, sizeof(szLine), hFile);
	int iPolyID, iWater, i;
	long fLong, fLat;
	std::map<unsigned int, std::list<std::pair<unsigned int, bool> > >::iterator recPolyID;
	std::set<unsigned int> waterPolys;
	std::list<std::pair<unsigned int, bool> >::iterator record;
	while (!feof(hFile))
	{
		iPolyID = atoi(ExtractField(szLine, 16, 25));
		recPolyID = m_mapPolyIDtoRecords.find(iPolyID);
		if (recPolyID != m_mapPolyIDtoRecords.end())
		{
			iWater = atoi(ExtractField(szLine, 45, 45));
			fLong = atoi(ExtractField(szLine, 26, 35));
			fLat = atoi(ExtractField(szLine, 36, 44));

			for (record = recPolyID->second.begin(); record != recPolyID->second.end(); ++record) {
				if (record->second) {
					m_pRecords[record->first].bWaterR = iWater;
					m_pRecords[record->first].ptWaterR.Set(fLong, fLat);
				} else {
					m_pRecords[record->first].bWaterL = iWater;
					m_pRecords[record->first].ptWaterL.Set(fLong, fLat);
				}
			}
			if (iWater) waterPolys.insert(iPolyID);

		}
		fgets(szLine, sizeof(szLine), hFile);
	}
	fclose(hFile);

	std::set<unsigned int>::iterator iterWaterPolys;
	std::list<std::list<Coords> > polygon;
	std::map<Coords, std::list<std::list<Coords> >::iterator> vertices;
	std::list<std::list<Coords> >::iterator iterPolygon, iterWholePolygon;
	std::map<Coords, std::list<std::list<Coords> >::iterator>::iterator iterVertex;
	Rect boundingRect;
	for (iterWaterPolys = waterPolys.begin(); iterWaterPolys != waterPolys.end(); ++iterWaterPolys) {
		recPolyID = m_mapPolyIDtoRecords.find(*iterWaterPolys);
		for (record = recPolyID->second.begin(); record != recPolyID->second.end(); ++record) {
			if (m_pRecords[record->first].nShapePoints < 2) continue;
			iterPolygon = polygon.insert(polygon.end(), std::list<Coords>());
			for (i = 0; i < m_pRecords[record->first].nShapePoints; i++)
				iterPolygon->push_back(m_pRecords[record->first].pShapePoints[i]);
		}

		for (iterPolygon = polygon.begin(); iterPolygon != polygon.end(); ++iterPolygon) {
			iterVertex = vertices.insert(std::pair<Coords, std::list<std::list<Coords> >::iterator>(iterPolygon->back(), iterPolygon)).first;
			if (iterVertex->second != iterPolygon) {
				if (iterPolygon->back() == iterVertex->second->front()) {
					iterVertex->second->pop_front();
					iterPolygon->insert(iterPolygon->end(), iterVertex->second->begin(), iterVertex->second->end());
				} else {
					iterVertex->second->pop_back();
					iterPolygon->insert(iterPolygon->end(), iterVertex->second->rbegin(), iterVertex->second->rend());
				}
				if (!(iterPolygon->back() == iterVertex->first))
					vertices[iterPolygon->back()] = iterPolygon;
				polygon.erase(iterVertex->second);
				vertices.erase(iterVertex);
			}
			iterVertex = vertices.insert(std::pair<Coords, std::list<std::list<Coords> >::iterator>(iterPolygon->front(), iterPolygon)).first;
			if (iterVertex->second != iterPolygon) {
				if (iterPolygon->front() == iterVertex->second->back()) {
					iterVertex->second->pop_back();
					iterPolygon->insert(iterPolygon->begin(), iterVertex->second->begin(), iterVertex->second->end());
				} else {
					iterVertex->second->pop_front();
					iterPolygon->insert(iterPolygon->begin(), iterVertex->second->rbegin(), iterVertex->second->rend());
				}
				if (!(iterPolygon->front() == iterVertex->first))
					vertices[iterPolygon->front()] = iterPolygon;
				polygon.erase(iterVertex->second);
				vertices.erase(iterVertex);
			}
		}
		vertices.clear();
		for (iterWholePolygon = polygon.begin(); iterWholePolygon != polygon.end(); ++iterWholePolygon) {
			if (iterWholePolygon->size() > 2) {
				boundingRect = Rect::BoundingRect(iterWholePolygon->begin(), iterWholePolygon->end());
				m_WaterPolygons.push_back(std::pair<Rect, std::list<Coords> >(boundingRect, *iterWholePolygon));
			}
		}
		polygon.clear();
	}
	return false;
}



bool TIGERProcessor::LoadTypeOSM(const QString & strFilename)
{
	FILE * hFile;
	char szLine[256];
	unsigned long numRecords = 0;
	unsigned long numNodes = 0;
	MapRecord * psOldSegments = NULL;

	//printf("print sth");
	hFile = fopen(strFilename, "rb");
	if (!hFile)
	{
		g_pLogger->LogError("GrooveNet - Error", QString("Error opening %1 (type 1)!").arg(strFilename));
		return true;
	}
	fgets(szLine,sizeof(szLine),hFile);
	while(!feof(hFile))
	{
	  if(strstr(szLine,"<node id")!=NULL)
		  numNodes++;
	  if(strstr(szLine,"</way>")!=NULL)
		  numRecords++;
	  fgets(szLine,sizeof(szLine),hFile);
	//  printf("record Num :%ld\r\n",numRecords);
	}
	if (m_nRecords)
	{
		psOldSegments = m_pRecords;
	}
	m_pRecords = new MapRecord[m_nRecords + numRecords];
	if (m_nRecords)
	{
		memcpy(m_pRecords, psOldSegments, sizeof(MapRecord) * m_nRecords);
		delete psOldSegments;
	}
	

	//printf("Osm count done\r\n");
	m_pRecords = new MapRecord[numRecords];
	fseek(hFile, 0, SEEK_SET);
	char nodeID[11];
	char latTemp[12];
	char lonTemp[12];
	char zipCode[5];
	int iZipL, iZipR;
	
	AddressRange * psOldAddressZipRanges;

	char wayID[11];
	long wayIDnum;
	//printf("osm init done");

	
	long numNode = 0;;
	Coords  nodeCoord;
	RecordTypes eRT;
	bool bRail = false;
	nodeCoord.m_iRefCnt = 0;
	char tagValue[32];
	int tagValueDigitCnt = 0;
	fgets(szLine,sizeof(szLine),hFile);
	while(!feof(hFile))
	{
		if(strstr(szLine,"<node id")!=NULL)
		{
			strcpy(nodeID,ExtractField(strstr(szLine,"<node id"),11,20));
			strcpy(latTemp,ExtractField(strstr(szLine,"lat="),6,16));
			strcpy(lonTemp,ExtractField(strstr(szLine,"lon="),6,16));
			

			nodeID[10] = '\0';
			latTemp[11] = '\0';
			lonTemp[11] = '\0';
			nodeCoord.m_iLat = getCoord(latTemp,11)/10;
			nodeCoord.m_iLong = getCoord(lonTemp,11)/10;
			m_mapNodeIDandCoord.insert(std::pair<unsigned long,Coords>(atol(nodeID),nodeCoord));
			numNode++;

			fgets(szLine,sizeof(szLine),hFile);	
		}
		else if(strstr(szLine,"<nd ref")!= NULL)
		{
			strcpy(nodeID,ExtractField(strstr(szLine,"<nd ref"),10,19));
			nodeID[10] = '\0';
			if(m_mapNodeIDandCoord.find(atol(nodeID))!= m_mapNodeIDandCoord.end())
			{			
				(*m_mapNodeIDandCoord.find(atol(nodeID))).second.m_iRefCnt++;				
			}

			fgets(szLine,sizeof(szLine),hFile);
		}
		else
			fgets(szLine,sizeof(szLine),hFile);
	}
	
	
	fseek(hFile, 0, SEEK_SET);
	fgets(szLine,sizeof(szLine),hFile);
	while(!feof(hFile))
	{

	/*  if(strstr(szLine,"<node id")!=NULL)
	  {
	    strcpy(nodeID,ExtractField(strstr(szLine,"<node id"),11,20));
	    strcpy(latTemp,ExtractField(strstr(szLine,"lat="),6,16));
	    strcpy(lonTemp,ExtractField(strstr(szLine,"lon="),6,16));
	    

	    nodeID[10] = '\0';
	    latTemp[11] = '\0';
	    lonTemp[11] = '\0';
	    nodeCoord.m_iLat = getCoord(latTemp,11)/10;
	    nodeCoord.m_iLong = getCoord(lonTemp,11)/10;
	    m_mapNodeIDandCoord.insert(std::pair<unsigned long,Coords>(atol(nodeID),nodeCoord));
	    numNode++;

	    fgets(szLine,sizeof(szLine),hFile);
	    
	  }*/
	  if(strstr(szLine,"<way id")!=NULL)
	  {
	   // printf("processing ways\r\n");
	    strcpy(wayID,ExtractField(strstr(szLine,"<way id="),10,20));
	    wayID[10] = '\0';
	    wayIDnum = atol(wayID);
	    eRT = RecordTypeDefault;
	 //   printf("wayIDstr:%s\r\n",wayID);
	  //  printf("wayIDnum:%ld\r\n",wayIDnum);
	//    m_osmWayIDtoRecord.insert(std::pair<unsigned long, unsigned int>(wayIDnum,m_nRecords));
	 //   printf("wayID to Record insert done\r\n");
	    m_pRecords[m_nRecords].nShapePoints = 0;

	 //   printf("wayID to Record done\r\n");
	   
	    fgets(szLine,sizeof(szLine),hFile);
	    while(strstr(szLine,"</way>") == NULL)
	    {

	      

	      if(strstr(szLine,"<nd ref")!= NULL)
	      {
		
		strcpy(nodeID,ExtractField(strstr(szLine,"<nd ref"),10,19));
		nodeID[10] = '\0';
		if(m_mapNodeIDandCoord.find(atol(nodeID))!= m_mapNodeIDandCoord.end())
		{

			vShapePoints.push_back((*m_mapNodeIDandCoord.find(atol(nodeID))).second);


		}
		m_pRecords[m_nRecords].nShapePoints = vShapePoints.size();
		fgets(szLine,sizeof(szLine),hFile);
	      }
	      else if(strstr(szLine,"tiger:name_base"))//OSM feature name and feature type
	      {
		      strcpy(tagValue,ExtractField(strstr(szLine,"v="),4,32));
			
		 //     m_pRecords[m_nRecords].pFeatureNames = new unsigned int[1];
		      vFeatureNames.push_back(AddString(QString(tagValue).stripWhiteSpace().remove("\"/>")));
		//	m_pRecords[m_nRecords].pFeatureNames[0] = AddString(QString(tagValue).stripWhiteSpace().remove("\"/>"));
		//	m_pRecords[m_nRecords].nFeatureNames = 1;
		      for(int i = 0; i < 32; i++)
			      tagValue[i] = ' ';
		      fgets(szLine,sizeof(szLine),hFile);	       
		
	      }
	      else if(strstr(szLine,"tiger:name_type"))
	      {
		      strcpy(tagValue,ExtractField(strstr(szLine,"v="),4,32));
		//      m_pRecords[m_nRecords].pFeatureTypes = new unsigned int[1];
		      vFeatureTypes.push_back(AddString(QString(tagValue).stripWhiteSpace().remove("\"/>")));
		      
		 //     m_pRecords[m_nRecords].pFeatureTypes[0] = AddString(QString(tagValue).stripWhiteSpace().remove("\"/>"));
		      for(int i = 0; i < 32; i++)
			      tagValue[i] = ' ';
		      fgets(szLine,sizeof(szLine),hFile);
	      }
	      else if(strstr(szLine,"tiger:zip_left"))
	      {
		      int i;
		      strcpy(tagValue,ExtractField(strstr(szLine,"v="),4,8));
		      iZipL = atoi(tagValue);
		      psOldAddressZipRanges = m_pRecords[m_nRecords].pAddressRanges;
		      m_pRecords[m_nRecords].pAddressRanges = new AddressRange[m_pRecords[m_nRecords].nAddressRanges + 1];
		      for (i = 0; i < m_pRecords[m_nRecords].nAddressRanges; i++)
			      m_pRecords[m_nRecords].pAddressRanges[i] = psOldAddressZipRanges[i];
		
		      m_pRecords[m_nRecords].pAddressRanges[i].iZip = iZipL;
		      m_pRecords[m_nRecords].pAddressRanges[i].iFromAddr = 0000;
		      m_pRecords[m_nRecords].pAddressRanges[i].iToAddr = 9999;
		      if (m_pRecords[m_nRecords].nAddressRanges)
			      delete psOldAddressZipRanges;

		      m_pRecords[m_nRecords].nAddressRanges++;

		//      printf("zipcodeL: %d\r\n",iZipL);
		      fgets(szLine,sizeof(szLine),hFile);
	      }
	      else if(strstr(szLine,"tiger:zip_right"))
	      {
		      int i;
		      strcpy(tagValue,ExtractField(strstr(szLine,"v="),4,8));
		      iZipR = atoi(tagValue);
		      psOldAddressZipRanges = m_pRecords[m_nRecords].pAddressRanges;
		      m_pRecords[m_nRecords].pAddressRanges = new AddressRange[m_pRecords[m_nRecords].nAddressRanges + 1];
		      for (i = 0; i < m_pRecords[m_nRecords].nAddressRanges; i++)
			      m_pRecords[m_nRecords].pAddressRanges[i] = psOldAddressZipRanges[i];
		
		      m_pRecords[m_nRecords].pAddressRanges[i].iZip = iZipL;
		      m_pRecords[m_nRecords].pAddressRanges[i].iFromAddr = 0000;
		      m_pRecords[m_nRecords].pAddressRanges[i].iToAddr = 9999;
		      if (m_pRecords[m_nRecords].nAddressRanges)
			      delete psOldAddressZipRanges;

		      m_pRecords[m_nRecords].nAddressRanges++;

	//	      printf("zipcode R: %d\r\n",iZipR);
		      fgets(szLine,sizeof(szLine),hFile);
	      }	    
	      /*OSM road type*/
	      else if(strstr(szLine,"k=\"highway"))
	      {
		eRT = RecordTypeTwoWaySmallRoad;
		if(strstr(szLine,"residential")||strstr(szLine,"trunk")||strstr(szLine,"service")||strstr(szLine,"tertiary"))
		{
			eRT = RecordTypeTwoWaySmallRoad;
		}
		else if(strstr(szLine,"motorway"))
		{
			eRT = RecordTypeTwoWayHighway;
		}
		else if(strstr(szLine,"primary"))
		{
			eRT = RecordTypeTwoWayPrimary;
		}
		else if (strstr(szLine,"secondary"))
		{
			eRT = RecordTypeTwoWayLargeRoad;
		}	
		else if (strstr(szLine,"pedestrian") || strstr(szLine,"living_street") || strstr(szLine,"footway") )
		{
			eRT = RecordTypePedestrian;
		}
				
		fgets(szLine,sizeof(szLine),hFile);
	      }
	      
		 
	      else if(strstr(szLine,"k=\"oneway\" v=\"yes\""))
	      {
	//	printf("2 or 1 way tags\r\n");
		switch(eRT)
		{
		  case RecordTypeTwoWaySmallRoad:
		    eRT = RecordTypeOneWaySmallRoad;
		    break;
		  case RecordTypeTwoWayHighway:
		    eRT = RecordTypeOneWayHighway;
		    break;
		  case RecordTypeTwoWayPrimary:
		    eRT = RecordTypeOneWayPrimary;
		    break;
		  case RecordTypeTwoWayLargeRoad:
		    eRT = RecordTypeOneWayLargeRoad;
		    break;
		  default:
			  break;
		}
		fgets(szLine,sizeof(szLine),hFile);		
	      }
	      else if(strstr(szLine,"k=\"waterway"))
	      {
		eRT = RecordTypeWater;
		fgets(szLine,sizeof(szLine),hFile);
	      }
	      else if(strstr(szLine,"k=\"railway"))
	      {		
		//eRT = RecordTypeRailroad;
		bRail = true;
		fgets(szLine,sizeof(szLine),hFile);
	      }
	      else if(strstr(szLine,"k=\"boundary"))
	      {
		//printf("processing boundary\r\n");
		
		eRT = RecordTypeInvisibleLandBoundary;
		fgets(szLine,sizeof(szLine),hFile);
	      }
	      else
		fgets(szLine,sizeof(szLine),hFile);
	     
	      
	    }
	    while(vFeatureNames.size() != vFeatureTypes.size())
	    {
		if(vFeatureNames.size() > vFeatureTypes.size())
		{
			vFeatureTypes.push_back(AddString(QString("    ").stripWhiteSpace()));
		}
		else if(vFeatureNames.size() < vFeatureTypes.size())
		{
			vFeatureNames.push_back(AddString(QString("    ").stripWhiteSpace()));
		}
		    
	    }
	    if(vFeatureNames.size()>0)
	    {
		    m_pRecords[m_nRecords].pFeatureNames = new unsigned int[vFeatureNames.size()];
		    m_pRecords[m_nRecords].pFeatureTypes = new unsigned int[vFeatureTypes.size()];
		    for(int iFN = 0; iFN < vFeatureNames.size(); iFN ++)
		    {
			    m_pRecords[m_nRecords].pFeatureNames[iFN] = vFeatureNames[iFN];
			    m_pRecords[m_nRecords].pFeatureTypes[iFN] = vFeatureTypes[iFN];
		    }
		    m_pRecords[m_nRecords].nFeatureNames = vFeatureNames.size();
			
            }


	    if(m_pRecords[m_nRecords].pFeatureNames == NULL)
	    {
		    m_pRecords[m_nRecords].pFeatureNames = new unsigned int[1];
		    m_pRecords[m_nRecords].pFeatureNames[0] = AddString(QString("    ").stripWhiteSpace());
		    m_pRecords[m_nRecords].nFeatureNames = 1;
	    }
	    if(m_pRecords[m_nRecords].pFeatureTypes == NULL)
	    {
		    m_pRecords[m_nRecords].pFeatureTypes = new unsigned int[1];
		    m_pRecords[m_nRecords].pFeatureTypes[0] = AddString(QString("    ").stripWhiteSpace());
            }
            
	    m_pRecords[m_nRecords].eRecordType = eRT;
	    if(!IsRoad(m_pRecords + m_nRecords) && bRail)
	    {
		m_pRecords[m_nRecords].eRecordType = RecordTypeRailroad;    
	    }
	    bRail = FALSE;
	
	    m_pRecords[m_nRecords].pShapePoints = new Coords[m_pRecords[m_nRecords].nShapePoints];
	    for(unsigned int i = 0 ; i < m_pRecords[m_nRecords].nShapePoints; i++)
	    {
	   //   m_pRecords[m_nRecords].pShapePoints[i] = vShapePoints[m_pRecords[m_nRecords].nShapePoints - i -1];
	      m_pRecords[m_nRecords].pShapePoints[i] = vShapePoints[i];
	    }
	    vShapePoints.clear();
	  
	    m_pRecords[m_nRecords].pVertices = NULL;
	    m_pRecords[m_nRecords].nVertices = 0;
	    if(m_pRecords[m_nRecords].eRecordType == RecordTypeInvisibleLandBoundary)
	    //if(eRT == RecordTypeInvisibleLandBoundary)
	    {
		 
		    
		for(unsigned int i = 0; i < m_pRecords[m_nRecords].nShapePoints; i++)
		{
			//printf("processing boundingRect\r\n");  
			if(!areaLeft)
				areaLeft = new Coords(m_pRecords[m_nRecords].pShapePoints[i]);
			else if(areaLeft->m_iLong > m_pRecords[m_nRecords].pShapePoints[i].m_iLong)
				*areaLeft = m_pRecords[m_nRecords].pShapePoints[i];
			if(!areaRight)
				areaRight = new Coords(m_pRecords[m_nRecords].pShapePoints[i]);
			else if(areaRight->m_iLong < m_pRecords[m_nRecords].pShapePoints[i].m_iLong)
				*areaRight= m_pRecords[m_nRecords].pShapePoints[i];
			if(!areaTop)
				areaTop = new Coords(m_pRecords[m_nRecords].pShapePoints[i]);
			else if(areaTop->m_iLat < m_pRecords[m_nRecords].pShapePoints[i].m_iLat)
				*areaTop = m_pRecords[m_nRecords].pShapePoints[i];
			if(!areaBottom)
				areaBottom = new Coords(m_pRecords[m_nRecords].pShapePoints[i]);
			else if(areaBottom->m_iLat > m_pRecords[m_nRecords].pShapePoints[i].m_iLat)
				*areaBottom = m_pRecords[m_nRecords].pShapePoints[i];		  
		//	printf("areaLeft long: %ld\r\n",areaLeft->m_iLong);
		//	printf("mapRec :%ld, shapePoint long: %ld\r\n",m_nRecords,m_pRecords[m_nRecords].vShapePoints[i].m_iLong);
			
		}
	    }
	    vFeatureNames.clear();
	    vFeatureTypes.clear();
    	 //   printf("way node 1 coord %ld\r\n",m_pRecords[m_nRecords].vShapePoints[0].m_iLat);

	  //  if(eRT == RecordTypeInvisibleLandBoundary)
	//	    printf("RT = RecordTypeInvisibleLandBoundary\r\n");
	 /*   if(m_pRecords[m_nRecords].pFeatureNames[0] == 7100 && m_pRecords[m_nRecords].pFeatureTypes[0] == 8)
	    {
		for(int Fi = 0; Fi < m_pRecords[m_nRecords].nShapePoints; Fi++)
		{
			printf("NumRef: %d,	Coords: %ld	%ld\r\n",m_pRecords[m_nRecords].pShapePoints[Fi].m_iRefCnt,m_pRecords[m_nRecords].pShapePoints[Fi].m_iLat,m_pRecords[m_nRecords].pShapePoints[Fi].m_iLong);
		}
	    }*/
	    m_nRecords++ ;
	  
	    
	  }
	
	  else
	    fgets(szLine,sizeof(szLine),hFile);
	  
	  
	}
	
	long typeLandBoundCnt = 0;
	long typeWayCnt = 0;
	long typeDefaultCnt = 0;
/*	for(int i = 0; i < m_nRecords; i++)
	{
	  switch(m_pRecords[m_nRecords].eRecordType)
	  {
	    case RecordTypeInvisibleLandBoundary:
	      typeLandBoundCnt++;
	      break;
	    case RecordTypeTwoWayHighway:
	      typeWayCnt++;
	      break;
	    case RecordTypeTwoWayLargeRoad:
	      typeWayCnt++;
	      break;
	    case RecordTypeTwoWayPrimary:
	      typeWayCnt++;
	      break;
	    case RecordTypeTwoWaySmallRoad:
	      typeWayCnt++;
	      break;
	    case RecordTypeOneWayHighway:
	      typeWayCnt++;
	      break;
	    case RecordTypeOneWayLargeRoad:
	      typeWayCnt++;
	      break;
	    case RecordTypeOneWayPrimary:
	      typeWayCnt++;
	      break;
	    case RecordTypeOneWaySmallRoad:
	      typeWayCnt++;
	      break;
	    case RecordTypeDefault:
	      typeDefaultCnt++;
	      break;
	  }   
	  
	}*/
	//printf("\r\ntotal record: %ld\r\n",m_nRecords);
	//printf("Land Bound: %ld  Default Record: %ld  highways: %ld\r\n",typeLandBoundCnt,typeDefaultCnt,typeWayCnt);
	//printf("boundingrect areaLeft: %ld %ld\r\n:",areaLeft->m_iLat,areaLeft->m_iLong);
/*	std::map<unsigned long, Coords>::iterator nodeItr;
	nodeItr = m_mapNodeID.find(1558356649);
	printf("node %ld\r\n",(*nodeItr).first);
	printf("->>");
	printf("Lat :%ld, Long :%ld\r\n",(*nodeItr).second.m_iLat,(*nodeItr).second.m_iLong);
*/
/*	unsigned int offset = 81371;
	std::map<unsigned long, unsigned int>::iterator wayIDItr;
	wayIDItr = m_osmWayIDtoRecord.begin();
	for(unsigned int i = 0; i < offset;i++)
		wayIDItr.operator++();
*/	

	//printf("numNodes: %ld\r\n",numNode);
	//printf("numWays: %d\r\n",m_nRecords);
/*	for(int i = 0; i < m_pRecords[549].nShapePoints; i ++)
	{
		printf("Coords No.%d: %ld  %ld\r\n",i,m_pRecords[549].pShapePoints[i].m_iLat,m_pRecords[549].pShapePoints[i].m_iLong);
	}*/
	//printf("Rec No 26614 nShapePoints:%d\r\n",m_pRecords[26614].nShapePoints);

	BreakWays();
	for(int i = 0; i < m_nRecords; i++)
	{
		printf("Record No.%d\r\n",i);
		for(int j = 0; j < m_pRecords[i].nShapePoints; j++)
		{
			printf("Coord: %ld	%ld\r\n", m_pRecords[i].pShapePoints[j].m_iLat, m_pRecords[i].pShapePoints[j].m_iLong);
		}
	}
/*	for(unsigned int i = offset; i < offset+5;i++)
	{
	 
		printf("Way ID: %ld\r\n",(*wayIDItr).first);
		printf("Way Connections: %d\r\n",m_pRecords[i].nShapePoints);
		for(unsigned int j =0; j < m_pRecords[i].nShapePoints;j++)
		{
		printf("%ld	%ld\r\n",m_pRecords[i].pShapePoints[j].m_iLat,m_pRecords[i].pShapePoints[j].m_iLong);
		}
		printf("-------------------------------------------------------\r\n");
		wayIDItr.operator++();
		
	  

	  
	}*/
	//printf("print sth\r\n");
	fclose(hFile);
	return false;


}




// new breakway
bool TIGERProcessor::BreakWays()
{
	printf("start break\r\n");
	int numBreakPoints = 0;
	std::vector<unsigned int> vBreakPoints;
	std::vector<unsigned int>::iterator bpItr;

	MapRecord * psOldSegments = NULL;
	ProcessingWay = new MapRecord[1];
	unsigned int numShapePnts = 0;
	if (m_nRecords)
	{
		psOldSegments = new MapRecord[m_nRecords];
		memcpy(psOldSegments, m_pRecords, sizeof(MapRecord) * m_nRecords);
	}
	for(unsigned int iRec = 0; iRec < m_nRecords; iRec++)
	{
// 		//clear ProcessingWay
// 		if(ProcessingWay[0].pShapePoints != NULL) delete[] ProcessingWay[0].pShapePoints;
// 		if(ProcessingWay[0].pAddressRanges != NULL) delete[] ProcessingWay[0].pAddressRanges;
// 		if(ProcessingWay[0].pFeatureNames != NULL) delete[] ProcessingWay[0].pFeatureNames;
// 		if(ProcessingWay[0].pFeatureTypes != NULL) delete[] ProcessingWay[0].pFeatureTypes;
// 		if(ProcessingWay != NULL)
// 		{
// 			delete[] ProcessingWay;
// 			ProcessingWay = NULL;
// 		}
// 		printf("ProcessingWay cleared\r\n");

		numShapePnts = m_pRecords[iRec].nShapePoints;
		ProcessingWay[0].nFeatureNames = m_pRecords[iRec].nFeatureNames;
		ProcessingWay[0].eRecordType = m_pRecords[iRec].eRecordType;
		ProcessingWay[0].pFeatureNames = new unsigned int[ProcessingWay[0].nFeatureNames];
		ProcessingWay[0].pFeatureTypes = new unsigned int[ProcessingWay[0].nFeatureNames];
		for(unsigned int i = 0; i < ProcessingWay[0].nFeatureNames; i++)
		{
			ProcessingWay[0].pFeatureNames[i] = m_pRecords[iRec].pFeatureNames[i];
			ProcessingWay[0].pFeatureTypes[i] = m_pRecords[iRec].pFeatureTypes[i];			
		}
		ProcessingWay[0].nAddressRanges = m_pRecords[iRec].nAddressRanges;
		ProcessingWay[0].pAddressRanges = new AddressRange[ProcessingWay[0].nAddressRanges];
		for(unsigned int i = 0; i < ProcessingWay[0].nAddressRanges; i++)
		{
			ProcessingWay[0].pAddressRanges[i].bOnLeft = m_pRecords[iRec].pAddressRanges[i].bOnLeft;				
			ProcessingWay[0].pAddressRanges[i].iFromAddr = m_pRecords[iRec].pAddressRanges[i].iFromAddr;
			ProcessingWay[0].pAddressRanges[i].iToAddr = m_pRecords[iRec].pAddressRanges[i].iToAddr;
			ProcessingWay[0].pAddressRanges[i].iZip = m_pRecords[iRec].pAddressRanges[i].iZip;				
		
		
		
		}
	

		if(IsRoad(m_pRecords + iRec) && (numShapePnts >2))
		{
		//	printf("this is rec No.%d\r\n",iRec);
			for(unsigned int iShape = 1; iShape < m_pRecords[iRec].nShapePoints-1 ; iShape++)
			{
				if((m_pRecords[iRec].pShapePoints[iShape].m_iRefCnt > 1) )
				{
					bpItr = vBreakPoints.end();
					vBreakPoints.insert(bpItr,iShape);

				}
			}

			if(vBreakPoints.size() > 0)
			{
			//	printf("iRec: %d\r\n",iRec);
			//	memcpy(&ProcessingWay[0], m_pRecords + iRec, sizeof(MapRecord));
			//	if(ProcessingWay[0].pShapePoints != NULL) delete[] ProcessingWay[0].pShapePoints;
			//	ProcessingWay[0].nShapePoints = 0;addsasam

		//		printf("breakpoint inserted\r\n");
				/*Processing the first break point*/
				ProcessingWay[0].nShapePoints = vBreakPoints[0] + 1;
		//		printf("ProcessingWay nShapePoints:%d\r\n",ProcessingWay[0].nShapePoints);
				ProcessingWay[0].pShapePoints = new Coords[ProcessingWay[0].nShapePoints];
		//		printf("buf created\r\n");
			//	memcpy(ProcessingWay[0].pShapePoints, m_pRecords[iRec].pShapePoints,sizeof(Coords) * ProcessingWay[0].nShapePoints);
		  		for(int iCopyShape = 0; iCopyShape < ProcessingWay[0].nShapePoints; iCopyShape++)
				{
					ProcessingWay[0].pShapePoints[iCopyShape].m_iLat = m_pRecords[iRec].pShapePoints[iCopyShape].m_iLat;
					ProcessingWay[0].pShapePoints[iCopyShape].m_iLong = m_pRecords[iRec].pShapePoints[iCopyShape].m_iLong;
				//	printf("copied Coords: %ld	%ld\r\n",ProcessingWay[0].pShapePoints[iCopyShape].m_iLat,ProcessingWay[0].pShapePoints[iCopyShape].m_iLong);
				//	printf("Orig Coords: %ld %ld\r\n",m_pRecords[iRec].pShapePoints[iCopyShape].m_iLat,m_pRecords[iRec].pShapePoints[iCopyShape].m_iLong);

				}

				vAdditionalWays.insert(vAdditionalWays.end(), ProcessingWay[0]);
			//	if(ProcessingWay[0].pShapePoints != NULL) delete[] ProcessingWay[0].pShapePoints;
			//	ProcessingWay[0].nShapePoints = 0;
		//		printf("first seg processed\r\n");
				/*Processing the other break points except the 1st one*/
				for(int i = 1; i < vBreakPoints.size() ; i++)
				{
					ProcessingWay[0].nShapePoints = vBreakPoints[i] - vBreakPoints[i-1] + 1;
					ProcessingWay[0].pShapePoints = new Coords[ProcessingWay[0].nShapePoints];
				//	memcpy(ProcessingWay[0].pShapePoints, m_pRecords[iRec].pShapePoints + vBreakPoints[i-1], sizeof(Coords) * ProcessingWay[0].nShapePoints);
					
					for(int iCopyShape = 0; iCopyShape < ProcessingWay[0].nShapePoints; iCopyShape++)
					{
						ProcessingWay[0].pShapePoints[iCopyShape].m_iLat = m_pRecords[iRec].pShapePoints[iCopyShape + vBreakPoints[i-1]].m_iLat;
						ProcessingWay[0].pShapePoints[iCopyShape].m_iLong = m_pRecords[iRec].pShapePoints[iCopyShape + vBreakPoints[i-1]].m_iLong;
					//	printf("copied Coords: %ld	%ld\r\n",ProcessingWay[0].pShapePoints[iCopyShape].m_iLat,ProcessingWay[0].pShapePoints[iCopyShape].m_iLong);
						
					}
					vAdditionalWays.insert(vAdditionalWays.end(), ProcessingWay[0]);


				//	if(ProcessingWay[0].pShapePoints != NULL) delete[] ProcessingWay[0].pShapePoints;
				//	ProcessingWay[0].nShapePoints = 0;


				}
		//		printf("middle segs processed\r\n");
				/*Processing the last way segment*/
			
				ProcessingWay[0].nShapePoints = m_pRecords[iRec].nShapePoints  - vBreakPoints[vBreakPoints.size() - 1] ;
				ProcessingWay[0].pShapePoints = new Coords[ProcessingWay[0].nShapePoints];
			//	memcpy(ProcessingWay[0].pShapePoints, m_pRecords[iRec].pShapePoints + vBreakPoints[vBreakPoints.size()-1], sizeof(Coords) * ProcessingWay[0].nShapePoints);
				for(int iCopyShape = 0; iCopyShape < ProcessingWay[0].nShapePoints; iCopyShape++)
				{
					ProcessingWay[0].pShapePoints[iCopyShape].m_iLat = m_pRecords[iRec].pShapePoints[iCopyShape + vBreakPoints[vBreakPoints.size() - 1]].m_iLat;
					ProcessingWay[0].pShapePoints[iCopyShape].m_iLong = m_pRecords[iRec].pShapePoints[iCopyShape + vBreakPoints[vBreakPoints.size() - 1]].m_iLong;
				//	printf("copied Coords: %ld	%ld\r\n",ProcessingWay[0].pShapePoints[iCopyShape].m_iLat,ProcessingWay[0].pShapePoints[iCopyShape].m_iLong);

				}

				vAdditionalWays.insert(vAdditionalWays.end(), ProcessingWay[0]);
				

				vBreakPoints.clear();
				
			}
			else
			{
				ProcessingWay[0].nShapePoints = m_pRecords[iRec].nShapePoints;
				ProcessingWay[0].pShapePoints = new Coords[ProcessingWay[0].nShapePoints];
				for(int iCopyShape = 0; iCopyShape < ProcessingWay[0].nShapePoints; iCopyShape++)
				{
					ProcessingWay[0].pShapePoints[iCopyShape].m_iLat = m_pRecords[iRec].pShapePoints[iCopyShape].m_iLat;
					ProcessingWay[0].pShapePoints[iCopyShape].m_iLong = m_pRecords[iRec].pShapePoints[iCopyShape].m_iLong;
				}
				vAdditionalWays.insert(vAdditionalWays.end(),ProcessingWay[0]);				
			}
		//	numBreakPoints = 0;

			
			
			
		}
		else
		{
			ProcessingWay[0].nShapePoints = m_pRecords[iRec].nShapePoints;
			ProcessingWay[0].pShapePoints = new Coords[ProcessingWay[0].nShapePoints];
			for(int iCopyShape = 0; iCopyShape < ProcessingWay[0].nShapePoints; iCopyShape++)
			{
				ProcessingWay[0].pShapePoints[iCopyShape].m_iLat = m_pRecords[iRec].pShapePoints[iCopyShape].m_iLat;
				ProcessingWay[0].pShapePoints[iCopyShape].m_iLong = m_pRecords[iRec].pShapePoints[iCopyShape].m_iLong;
			}
			vAdditionalWays.insert(vAdditionalWays.end(),ProcessingWay[0]);
		}

		
	}
	printf("finished break\r\n");

	for (unsigned int iRec = 0; iRec < m_nRecords; iRec++)
	{
		if (m_pRecords[iRec].pAddressRanges != NULL) delete[] m_pRecords[iRec].pAddressRanges;
		if (m_pRecords[iRec].pFeatureNames != NULL) delete[] m_pRecords[iRec].pFeatureNames;
		if (m_pRecords[iRec].pFeatureTypes != NULL) delete[] m_pRecords[iRec].pFeatureTypes;
		if (m_pRecords[iRec].pShapePoints != NULL) delete[] m_pRecords[iRec].pShapePoints;
		if (m_pRecords[iRec].pVertices != NULL) delete[] m_pRecords[iRec].pVertices;
	}


	if (m_pRecords != NULL) {
		delete[] m_pRecords;
		m_pRecords = NULL;
	}
	printf("m_pRecords cleared\r\n");
	m_nRecords = vAdditionalWays.size();
	m_pRecords = new MapRecord[vAdditionalWays.size()];
	for(int iRec = 0; iRec < m_nRecords; iRec++)
	{
		m_pRecords[iRec].nFeatureNames =vAdditionalWays[iRec].nFeatureNames;
		m_pRecords[iRec].pFeatureNames = new unsigned int[m_pRecords[iRec].nFeatureNames];
		m_pRecords[iRec].pFeatureTypes = new unsigned int[m_pRecords[iRec].nFeatureNames];
		for(int i = 0; i < m_pRecords[iRec].nFeatureNames; i++)
		{
			m_pRecords[iRec].pFeatureNames[i] = vAdditionalWays[iRec].pFeatureNames[i];
			m_pRecords[iRec].pFeatureTypes[i] = vAdditionalWays[iRec].pFeatureTypes[i];
		}
		m_pRecords[iRec].nAddressRanges = vAdditionalWays[iRec].nAddressRanges;
		m_pRecords[iRec].pAddressRanges = new AddressRange[m_pRecords[iRec].nAddressRanges];
		for(int i = 0; i < m_pRecords[iRec].nAddressRanges; i++)
		{
			m_pRecords[iRec].pAddressRanges[i].iFromAddr = vAdditionalWays[iRec].pAddressRanges[i].iFromAddr;	
			m_pRecords[iRec].pAddressRanges[i].iToAddr = vAdditionalWays[iRec].pAddressRanges[i].iToAddr;
			m_pRecords[iRec].pAddressRanges[i].iZip = vAdditionalWays[iRec].pAddressRanges[i].iZip;
			m_pRecords[iRec].pAddressRanges[i].bOnLeft = vAdditionalWays[iRec].pAddressRanges[i].bOnLeft;
		}
		m_pRecords[iRec].nShapePoints = vAdditionalWays[iRec].nShapePoints;
		m_pRecords[iRec].pShapePoints = new Coords[m_pRecords[iRec].nShapePoints];
		for(int i = 0; i < m_pRecords[iRec].nShapePoints; i++)
		{
			m_pRecords[iRec].pShapePoints[i].m_iLat = vAdditionalWays[iRec].pShapePoints[i].m_iLat;
			m_pRecords[iRec].pShapePoints[i].m_iLong = vAdditionalWays[iRec].pShapePoints[i].m_iLong;
		}
		m_pRecords[iRec].eRecordType = vAdditionalWays[iRec].eRecordType;
		m_pRecords[iRec].nVertices = 0;
		m_pRecords[iRec].pVertices = NULL;
	}
	
	
}


bool TIGERProcessor::LoadSet(const QString & strBaseName)
{
	int iCode;
	QFileInfo fi(strBaseName);	// strBaseName is some file...

	QString strBase = fi.baseName(true);
	QDir dir = fi.dir();
	countyCode = iCode = atoi(strBase.right(5));

	areaLeft = areaTop = areaRight = areaBottom = 0;
	/*LoadType1(dir.absFilePath(strBase + ".RT1"));
	LoadType2(dir.absFilePath(strBase + ".RT2"));
	LoadType4(dir.absFilePath(strBase + ".RT4"));
	LoadType5(dir.absFilePath(strBase + ".RT5"));
	LoadType6(dir.absFilePath(strBase + ".RT6"));
	LoadTypeI(dir.absFilePath(strBase + ".RTI"));
	LoadTypeP(dir.absFilePath(strBase + ".RTP"));*/
	LoadTypeOSM(dir.absFilePath(strBaseName));


	if (areaLeft && areaTop && areaRight && areaBottom)
		boundingRect = Rect(areaLeft->m_iLong, areaTop->m_iLat, areaRight->m_iLong, areaBottom->m_iLat);
	else
		boundingRect = Rect(0, 0, 0, 0);
	//printf("areaLeft : %ld %ld\r\n",areaLeft->m_iLat,areaLeft->m_iLong);
	if (areaLeft != 0) delete areaLeft;
	if (areaRight != 0) delete areaRight;
	if (areaTop != 0) delete areaTop;
	if (areaBottom != 0) delete areaBottom;
	unsigned int verCnt = 0;
	EnumerateVertices();
	for(int i = 0; i < m_nRecords; i++)
	{
		verCnt = verCnt + m_pRecords[i].nVertices;
	}
	printf("total Vertex: %d\r\n",verCnt);
	verCnt = 0;
	FixZipCodes();
	//printf("enumerate finished zip finished\r\n");
	QString codeString;
	codeString.sprintf("%05d", iCode);
	//printf("start writing\r\n");
	if (!WriteMap(dir.absFilePath(QString("%1.MAP").arg(codeString)))) {
		printf("in if !writeMap\r\n");
		QStringList files = dir.entryList(QString("TGR%1.*").arg(codeString), QDir::Files|QDir::Readable);
		QStringList::iterator filesIterator = files.begin();
		while (filesIterator != files.end()) {
			dir.remove(*filesIterator);
			++filesIterator;
		}
		printf("finish in if !writemap\r\n");
	}
	printf("start clean\r\n");
	Cleanup();
	printf("finished clean\r\n");
	printf("Loadset Finished\r\n");
	return false;
}

bool TIGERProcessor::WriteMap(const QString & fileName)
{
	int hFile = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (hFile == -1) return true;

	unsigned int i, j, numVertices, numStrings, numPolys, numPoints;
	unsigned char recordFlags;
	unsigned short numNeighbors;
	std::vector<Coords> coords;
	std::map<Coords, unsigned int>::iterator iterCoords;
	std::map<unsigned int, unsigned int>::iterator iterEdges;
	std::list<std::pair<Rect, std::list<Coords> > >::iterator poly;
	std::list<Coords>::iterator polyPoint;
	coords.resize(m_Vertices.size());
	for (iterCoords = m_mapCoordinateToVertex.begin(); iterCoords != m_mapCoordinateToVertex.end(); ++iterCoords)
		coords[iterCoords->second] = iterCoords->first;

	WriteMemory(&countyCode, hFile, sizeof(int));
	WriteRect(&boundingRect, hFile);
	numStrings = m_Strings.size();
	WriteMemory(&numStrings, hFile, sizeof(unsigned int));
	for (i = 0; i < numStrings; i++) {
		WriteString(m_Strings[i], hFile, m_Strings[i].length());
	}
	numVertices = m_Vertices.size();
	WriteMemory(&numVertices, hFile, sizeof(unsigned int));
	for (i = 0; i < numVertices; i++) {
		WriteCoords(&coords[i], hFile);
		numNeighbors = m_Vertices[i].mapEdges.size();
		WriteMemory(&numNeighbors, hFile, sizeof(unsigned short));
		for (iterEdges = m_Vertices[i].mapEdges.begin(); iterEdges != m_Vertices[i].mapEdges.end(); ++iterEdges)
		{
			WriteMemory(&iterEdges->second, hFile, sizeof(unsigned int));
			WriteMemory(&iterEdges->first, hFile, sizeof(unsigned int));
		}
		numNeighbors = m_Vertices[i].vecRoads.size();
		WriteMemory(&numNeighbors, hFile, sizeof(unsigned short));
		for (j = 0; j < numNeighbors; j++)
			WriteMemory(&m_Vertices[i].vecRoads[j], hFile, sizeof(unsigned int));
	}
//	printf("start writing m_pRecords\r\n");
	WriteMemory(&m_nRecords, hFile, sizeof(unsigned int));
	for (i = 0; i < m_nRecords; i++) {
		//printf("writing the %dth Rec\r\n",i);
		WriteMemory(&m_pRecords[i].nFeatureNames, hFile, sizeof(unsigned short));
		for (j = 0; j < m_pRecords[i].nFeatureNames; j++) {
			WriteMemory(m_pRecords[i].pFeatureNames + j, hFile, sizeof(unsigned int));
			WriteMemory(m_pRecords[i].pFeatureTypes + j, hFile, sizeof(unsigned int));
		}
		recordFlags = ((unsigned char)m_pRecords[i].eRecordType) & 0x3f;
		if (m_pRecords[i].bWaterL) recordFlags |= 0x80;
		if (m_pRecords[i].bWaterR) recordFlags |= 0x40;
		WriteMemory(&recordFlags, hFile, sizeof(unsigned char));
		WriteMemory(&m_pRecords[i].fCost, hFile, sizeof(float));
		WriteCoords(&m_pRecords[i].ptWaterL, hFile);
		WriteCoords(&m_pRecords[i].ptWaterR, hFile);
		WriteMemory(&m_pRecords[i].nAddressRanges, hFile, sizeof(unsigned short));
		for (j = 0; j < m_pRecords[i].nAddressRanges; j++)
			WriteAddressRange(m_pRecords[i].pAddressRanges + j, hFile);
		WriteRect(&m_pRecords[i].rBounds, hFile);
		WriteMemory(&m_pRecords[i].nShapePoints, hFile, sizeof(unsigned short));
		for (j = 0; j < m_pRecords[i].nShapePoints; j++)
			WriteCoords(m_pRecords[i].pShapePoints + j, hFile);
		WriteMemory(&m_pRecords[i].nVertices, hFile, sizeof(unsigned short));
		for (j = 0; j < m_pRecords[i].nVertices; j++)
			WriteMemory(m_pRecords[i].pVertices + j, hFile, sizeof(unsigned int));
	}
//	printf("finish writing m_pRecords\r\n");

	
//	printf("printf sth\r\n");
	numPolys = m_WaterPolygons.size();
	WriteMemory(&numPolys, hFile, sizeof(unsigned int));
	for (poly = m_WaterPolygons.begin(); poly != m_WaterPolygons.end(); ++poly) {
		WriteRect(&poly->first, hFile);
		numPoints = poly->second.size();
		WriteMemory(&numPoints, hFile, sizeof(unsigned int));
		for (polyPoint = poly->second.begin(); polyPoint != poly->second.end(); ++polyPoint)
			WriteCoords(&(*polyPoint), hFile);
	}
	//printf("writing finished\r\n");
	//printf("the Last Rec\r\n ");
//	for(int i = 0; i < m_pRecords[168784].nShapePoints;i++)
//	{
//		printf("Coords: %ld	%ld\r\n",m_pRecords[168784].pShapePoints[i].m_iLat,m_pRecords[168784].pShapePoints[i].m_iLong);
//	}

	close(hFile);
		printf("write map Finished\r\n");
	return false;
}

void TIGERProcessor::EnumerateVertices()
{
	// enumerate the vertices
	unsigned int iRec, iPreviousVertex, iVertex, iAdj;
	Coords * ptPrevious, * pt;
	MapRecord * psRec;
	std::map<Coords, unsigned int>::iterator vertexFromCoords;
	unsigned int * newAdjVertices;
	for (iRec = 0; iRec < m_nRecords; iRec++)
	{
		psRec = m_pRecords + iRec;
		psRec->rBounds = Rect::BoundingRect(psRec->pShapePoints, psRec->nShapePoints);

		// can't drive on rivers or railroad tracks
		if (IsRoad(psRec)/* || (psRec->eRecordType == RecordTypePedestrian && psRec->pFeatureNames[0] != AddString(QString("    ").stripWhiteSpace())   )*/)
		{
			iPreviousVertex = 0;
			pt = psRec->pShapePoints;
			vertexFromCoords = m_mapCoordinateToVertex.find(*pt);
			if (vertexFromCoords == m_mapCoordinateToVertex.end())
			{
				iVertex = m_Vertices.size();
				m_Vertices.push_back(Vertex());
				m_mapCoordinateToVertex.insert(std::pair<Coords, unsigned int>(*pt, iVertex));
			}
			else
				iVertex = vertexFromCoords->second;
			newAdjVertices = new unsigned int[m_pRecords[iRec].nVertices + 1];
			for (iAdj = 0; iAdj < m_pRecords[iRec].nVertices; iAdj++)
				newAdjVertices[iAdj] = m_pRecords[iRec].pVertices[iAdj];
			newAdjVertices[iAdj] = iVertex;
			if (m_pRecords[iRec].nVertices > 0) delete[] m_pRecords[iRec].pVertices;
			m_pRecords[iRec].pVertices = newAdjVertices;
			m_pRecords[iRec].nVertices++;
			iPreviousVertex = iVertex;
			ptPrevious = pt;
			if (psRec->nShapePoints > 0) {
				if (psRec->nShapePoints > 1) {
					pt = psRec->pShapePoints + psRec->nShapePoints - 1;
					vertexFromCoords = m_mapCoordinateToVertex.find(*pt);
					if (vertexFromCoords == m_mapCoordinateToVertex.end())
					{
						iVertex = m_Vertices.size();
						m_Vertices.push_back(Vertex());
						m_mapCoordinateToVertex.insert(std::pair<Coords, unsigned int>(*pt, iVertex));
					}
					else
						iVertex = vertexFromCoords->second;
					newAdjVertices = new unsigned int[m_pRecords[iRec].nVertices + 1];
					for (iAdj = 0; iAdj < m_pRecords[iRec].nVertices; iAdj++)
						newAdjVertices[iAdj] = m_pRecords[iRec].pVertices[iAdj];
					newAdjVertices[iAdj] = iVertex;
					if (m_pRecords[iRec].nVertices > 0) delete[] m_pRecords[iRec].pVertices;
					m_pRecords[iRec].pVertices = newAdjVertices;
					m_pRecords[iRec].nVertices++;
				}
				m_pRecords[iRec].fCost = RecordDistance(&m_pRecords[iRec]) * CostFactor(psRec);
				AddRecordToVertex(&m_Vertices[iPreviousVertex], m_pRecords, iRec, iVertex);
//				m_Vertices[iPreviousVertex].mapEdges.insert(std::pair<unsigned int, unsigned int>(iRec, iVertex));
				if (!IsOneWay(psRec))
				{
					AddRecordToVertex(&m_Vertices[iVertex], m_pRecords, iRec, iPreviousVertex);
//					m_Vertices[iVertex].mapEdges.insert(std::pair<unsigned int, unsigned>(iRec, iPreviousVertex));
				}
			/*	else
				{
					AddRecordToVertex(&m_Vertices[iVertex], m_pRecords, iRec, iVertex);
					
				}*/
			}
		}
	}
}

void TIGERProcessor::FixZipCodes()
{
	// fill in zip codes
	unsigned int iRec, iIterations, iVertex, iPeerVertex, iRange, i;
	unsigned int iRecEntry, iVertexNumber, iNewRecordNumber;
	MapRecord * psRec, * psRecEntry, * psNewRec;
	bool bDone;
	std::vector<unsigned int> vRecords, vVertices, vNewRecords;
	std::map<Coords, unsigned int>::iterator vertexIter;
	std::map<unsigned int, unsigned int>::iterator iPeer;
	Coords * pt;
	AddressRange * psOldAddressZipRanges;
	for (iRec = 0; iRec < m_nRecords; iRec++)
	{
		psRec = m_pRecords + iRec;
		if (!psRec->nAddressRanges)
		{
			vRecords.clear();
			bDone = false;
			iIterations = 3;
			vRecords.push_back(iRec);
			while (!bDone && (iIterations--) > 0)
			{
				vVertices.clear();
				vNewRecords.clear();
				for (iRecEntry = 0; iRecEntry < vRecords.size(); iRecEntry++)
				{
					psRecEntry = m_pRecords + vRecords[iRecEntry];
					if (psRecEntry->nShapePoints > 0) {
						pt = psRecEntry->pShapePoints;
						vertexIter = m_mapCoordinateToVertex.find(*pt);
						if (vertexIter != m_mapCoordinateToVertex.end())
							vVertices.push_back(vertexIter->second);
						pt = psRecEntry->pShapePoints + psRecEntry->nShapePoints - 1;
						vertexIter = m_mapCoordinateToVertex.find(*pt);
						if (vertexIter != m_mapCoordinateToVertex.end())
							vVertices.push_back(vertexIter->second);
					}
				}
				for (iVertexNumber = 0; iVertexNumber < vVertices.size(); iVertexNumber++)
				{
					iVertex = vVertices[iVertexNumber];
					for (iPeer = m_Vertices[vVertices[iVertexNumber]].mapEdges.begin(); iPeer != m_Vertices[vVertices[iVertexNumber]].mapEdges.end(); ++iPeer)
					{
						vNewRecords.push_back(iPeer->first);
						iPeerVertex = iPeer->second;
					}
				}
				for (iNewRecordNumber = 0; iNewRecordNumber < vNewRecords.size() && !bDone; iNewRecordNumber++)
				{
					psNewRec = m_pRecords + vNewRecords[iNewRecordNumber];
					for (iRange = 0; iRange < psNewRec->nAddressRanges && !bDone; iRange++)
					{
						if (psNewRec->pAddressRanges[iRange].iZip)
						{
							psOldAddressZipRanges = psRec->pAddressRanges;
							psRec->pAddressRanges = new AddressRange[psRec->nAddressRanges + 1];
							for (i = 0; i < psRec->nAddressRanges; i++)
								psRec->pAddressRanges[i] = psOldAddressZipRanges[i];
							psRec->pAddressRanges[i].iFromAddr = 0;
							psRec->pAddressRanges[i].iToAddr = 0;
							psRec->pAddressRanges[i].iZip = psNewRec->pAddressRanges[iRange].iZip;
							if (psRec->nAddressRanges)
								delete psOldAddressZipRanges;
							psRec->nAddressRanges++;
							bDone = true;
						}
					}
				}
				vRecords = vNewRecords;
				if (!vNewRecords.size())
					bDone = true;
			}
		}
	}
	//printf("FixZipCodes finished\r\n");
}

unsigned int TIGERProcessor::AddString(const QString & str)
{
	std::map<QString, unsigned int>::iterator iterName = m_mapStringsToIndex.find(str);
	if (iterName != m_mapStringsToIndex.end()) return iterName->second;

	unsigned int ret = m_Strings.size();
	m_mapStringsToIndex.insert(std::pair<QString, unsigned int>(str, ret));
	m_Strings.push_back(str);
	return ret;
}
long TIGERProcessor::getCoord(char* str, int strLen)
{
	int i = 0;
  int cnt = 0;
  if(str[0] == '-')
  {
    char newStr[strLen];
    memset(newStr,'0',strLen);
    newStr[strLen-1] = '\0';
    newStr[cnt] = '-';
    cnt++;
    i++;
    for (i; i < strLen;i++)
    {
      if(str[i] >= '0' && str[i] <= '9')   
      {
	newStr[cnt] = str[i];
	cnt++;
	
      }
    }
    //printf("str = %s\r\n",newStr);
   // printf("strNum = %ld\r\n",atol(newStr));
    return atol(newStr);  
  }
  else
  {
    char newStr[strLen-1];
    memset(newStr,'0',strLen-1);
    newStr[strLen-2] = '\0';
    for (i; i < strLen;i++)
    {
      if(str[i] >= '0' && str[i] <= '9')   
      {
	newStr[cnt] = str[i];
	cnt++;
      }    
    }
   // printf("str = %s\r\n",newStr);
    //printf("strNum = %ld\r\n",atol(newStr));
    return atol(newStr);
  }

}
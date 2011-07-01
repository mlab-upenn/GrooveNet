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

#include "MapDB.h"
#include "TIGERProcessor.h"
#include "Global.h"
#include "Settings.h"
#include "StringHelp.h"
#include "Logger.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qapplication.h>
#include <qstringlist.h>
#include <qcursor.h>

#define SQUARES_PER_COUNTY 100
#define SQUARES_PER_STATE 1600
#define SQUARES_PER_COUNTY_SIDE 10.0
#define SQUARES_PER_STATE_SIDE 40.0

#define TIGER_BASE_URL "http://www2.census.gov/geo/tiger/tiger2003/"
#define COUNTIES_URL "http://www.census.gov/geo/tigerline/app_a03.txt"

std::map<QString,QString> g_mapAbbreviationToFullName;
std::map<QString,QString> g_mapNameToAbbreviation;

std::map<QString, unsigned short> g_mapCountyNameToCode;
std::map<unsigned short, QString> g_mapCodeToCountyName;
std::vector<QString> g_vecCountyNames;

std::map<QString, unsigned short> g_mapStateNameToCode;
std::map<unsigned short, QString> g_mapStateCodeToName;
std::map<QString,QString> g_mapStateAbbreviationToName;
std::map<QString,QString> g_mapStateNameToAbbreviation;
std::vector<QString> g_vecStateAbbreviations;
std::vector<QString> g_vecStateNames;

std::map<QString, std::vector<unsigned int> > g_mapCityToZip;
std::map<unsigned int, QString> g_mapZipToCity;
std::map<unsigned int, QString> g_mapZipToStateName;
std::map<unsigned int, unsigned int> g_mapZipToCountyCode;

bool StringToAddress(const QString & strValue, Address * pAddress)
{
	QStringList listAddress;
	QStringList::iterator iterString;
	unsigned int i;

	// example address: 5000 Forbes Avenue, Pittsburgh, PA
	// example intersection: Forbes Avenue & Morewood Avenue, Pittsburgh, PA
	listAddress = QStringList::split(",", strValue);
	if (listAddress.size() < 3) // make sure we have enough members
		return false;

	iterString = listAddress.begin();
	if ((*iterString).find(" & ") > -1)
	{
		// intersection
		QString strStreetName, strStreetType, strStreetTypeAbbrev, strCity, strState, strTemp;
		QStringList listStreets;
		QStringList::iterator iterStreet;
		std::vector<unsigned int> vecZipCodes;
		std::set<unsigned int> setZipCodes, setRecords;
		std::set<unsigned int>::iterator iterRecord;
		std::set<unsigned short> setDownload;
		std::set<StreetNameAndType> setStreetNamesAndTypes;
		StreetNameAndType sStreetNameAndType;

		// split first field into list of streets
		*iterString = (*iterString).simplifyWhiteSpace();
		listStreets = QStringList::split(" & ", *iterString);

		// get all records for first street
		iterStreet = listStreets.begin();
		strStreetName = (*iterStreet).section(" ", 0, -2);
		strStreetType = (*iterStreet).section(" ", -1, -1);
		strStreetTypeAbbrev = GetAbbreviation(strStreetType);
		if (strStreetTypeAbbrev.isEmpty())
			strStreetTypeAbbrev = strStreetType;

		// get city name
		++iterString;
		*iterString = (*iterString).simplifyWhiteSpace();
		strCity = *iterString;

		// extract state name
		++iterString;
		*iterString = (*iterString).simplifyWhiteSpace();
		strState = *iterString;

		vecZipCodes = ZipCodesByCityState(strCity, strState);

		// take care of the list of zip codes
		for (i = 0; i < vecZipCodes.size(); i++)
		{
			unsigned short iCountyCode = CountyCodeByZip(vecZipCodes[i]);
			if (!g_pMapDB->IsCountyLoaded(iCountyCode))
				setDownload.insert(iCountyCode);
			setZipCodes.insert(vecZipCodes[i]);
		}

		if (!setDownload.empty())
			g_pMapDB->DownloadCounties(setDownload);

		g_pMapDB->GetStreetsByName(strStreetName, strStreetTypeAbbrev, setRecords);

		// get the street name and type indexes for the other streets
		for (; iterStreet != listStreets.end(); ++iterStreet)
		{
			strStreetName = (*iterStreet).section(" ", 0, -2);
			strStreetType = (*iterStreet).section(" ", -1, -1);
			strStreetTypeAbbrev = GetAbbreviation(strStreetType);
			if (strStreetTypeAbbrev.isEmpty())
				strStreetTypeAbbrev = strStreetType;
			sStreetNameAndType.iStreetName = g_pMapDB->GetStringIndex(strStreetName);
			sStreetNameAndType.iStreetType = g_pMapDB->GetStringIndex(strStreetTypeAbbrev);
			setStreetNamesAndTypes.insert(sStreetNameAndType);
		}

		// go through records to find the vertices we want
		for (iterRecord = setRecords.begin(); iterRecord != setRecords.end(); ++iterRecord)
		{
			MapRecord * pRecord = g_pMapDB->GetRecord(*iterRecord);
			bool bFoundZipCode = false;
			// confirm that we're in the right zip code
			for (i = 0; i < pRecord->nAddressRanges; i++)
			{
				if (setZipCodes.find(pRecord->pAddressRanges[i].iZip) != setZipCodes.end())
				{
					bFoundZipCode = true;
					break;
				}
			}

			if (!bFoundZipCode) // not in the right zip code
				continue;

			for (i = 0; i < pRecord->nVertices; i++)
			{
				// for each vertex, see if we have all the streets
				// we are looking for (and no others)
				if (g_pMapDB->IsVertex(pRecord->pVertices[i], setStreetNamesAndTypes))
				{
					// found it!
					if (g_pMapDB->GetVertex(pRecord->pVertices[i], pAddress))
						return true;
				}
			}
		}

		return false;
	}
	else
	{
		// regular address
		QString strStreetName, strStreetType, strCity, strState, strTemp;
		int iStreetNumber;
		std::vector<unsigned int> vecZipCodes;
		std::set<unsigned short> setDownload;

		// extract street number, name, type
		*iterString = (*iterString).simplifyWhiteSpace();
		strTemp = (*iterString).section(' ', 0, 0);
		iStreetNumber = (int)ValidateNumber(round(StringToNumber(strTemp)), 0, HUGE_VAL);
		strStreetName = (*iterString).section(' ', 1, -2);
		strStreetType = (*iterString).section(' ', -1, -1);

		// extract city name
		++iterString;
		*iterString = (*iterString).simplifyWhiteSpace();
		strCity = *iterString;

		// extract state name
		++iterString;
		*iterString = (*iterString).simplifyWhiteSpace();
		strState = *iterString;

		vecZipCodes = ZipCodesByCityState(strCity, strState);

		// perform search
		for (i = 0; i < vecZipCodes.size(); i++)
		{
			unsigned short iCountyCode = CountyCodeByZip(vecZipCodes[i]);
			if (!g_pMapDB->IsCountyLoaded(iCountyCode))
				setDownload.insert(iCountyCode);
		}

		if (!setDownload.empty())
			g_pMapDB->DownloadCounties(setDownload);

		return g_pMapDB->FindAddress(pAddress, iStreetNumber, strStreetName, strStreetType, strCity, strState);
	}
}

QString AddressToString(const Address * pAddress)
{
	QString strAddress;

	if (pAddress != NULL && pAddress->iRecord != (unsigned)-1)
	{
		if (pAddress->iVertex != (unsigned)-1)
		{
			// intersection
			strAddress = QString("%1, %2, %3").arg(pAddress->szStreetName).arg(pAddress->szCityName).arg(pAddress->szStateName);
		}
		else
		{
			// regular address
			strAddress = QString("%1 %2 %3, %4, %5").arg(pAddress->iStreetNumber).arg(pAddress->szStreetName).arg(pAddress->szStreetType).arg(pAddress->szCityName).arg(pAddress->szStateName);
		}
	}
	return strAddress;
}

bool IsRoad(const MapRecord * pRecord)
{
	switch (pRecord->eRecordType)
	{
	case RecordTypeOneWaySmallRoad:
	case RecordTypeOneWayLargeRoad:
	case RecordTypeOneWayPrimary:
	case RecordTypeOneWayHighway:
	case RecordTypeTwoWaySmallRoad:
	case RecordTypeTwoWayLargeRoad:
	case RecordTypeTwoWayPrimary:
	case RecordTypeTwoWayHighway:
		return true;
	default:
		return false;
	}
}

bool IsBigRoad(const MapRecord * pRecord)
{
	switch (pRecord->eRecordType)
	{
	case RecordTypeOneWayLargeRoad:
	case RecordTypeOneWayPrimary:
	case RecordTypeOneWayHighway:
	case RecordTypeTwoWayLargeRoad:
	case RecordTypeTwoWayPrimary:
	case RecordTypeTwoWayHighway:
		return true;
	default:
		return false;
	}
}

bool IsOneWay(const MapRecord * pRecord)
{
	switch (pRecord->eRecordType)
	{
	case RecordTypeOneWaySmallRoad:
	case RecordTypeOneWayLargeRoad:
	case RecordTypeOneWayPrimary:
	case RecordTypeOneWayHighway:
		return true;
	default:
		return false;
	}
}

bool IsSameRoad(const MapRecord * pRecord1, const MapRecord * pRecord2)
{
	unsigned int i, j;

	// assume same road if both records have same names - probably not a great assumption...
	for (i = 0; i < pRecord1->nFeatureNames; i++)
	{
		for (j = 0; j < pRecord2->nFeatureNames; j++)
		{
			if (pRecord1->pFeatureNames[i] == pRecord2->pFeatureNames[j] && pRecord1->pFeatureTypes[i] == pRecord2->pFeatureTypes[j])
				return true;
		}
	}
	return false;
}

#define SMALLROADLANES 1
#define LARGEROADLANES 2
#define PRIMARYROADLANES 2
#define HIGHWAYLANES 2

unsigned char NumberOfLanes(const MapRecord * pRecord)
{
	switch (pRecord->eRecordType)
	{
	case RecordTypeOneWaySmallRoad:
	case RecordTypeTwoWaySmallRoad:
		return SMALLROADLANES;
	case RecordTypeOneWayLargeRoad:
	case RecordTypeTwoWayLargeRoad:
		return LARGEROADLANES;
	case RecordTypeOneWayPrimary:
	case RecordTypeTwoWayPrimary:
		return PRIMARYROADLANES;
	case RecordTypeOneWayHighway:
	case RecordTypeTwoWayHighway:
		return HIGHWAYLANES;
	default:
		return 0;
	}
}

#define SMALLROADPENALTY 3.0
#define LARGEROADPENALTY 1.5
#define PRIMARYROADPENALTY 1.0
#define HIGHWAYROADPENALTY 0.5

float CostFactor(const MapRecord * pRecord)
{
	switch (pRecord->eRecordType)
	{
	case RecordTypeOneWaySmallRoad:
	case RecordTypeTwoWaySmallRoad:
		return SMALLROADPENALTY; // penalty for small roads
	case RecordTypeOneWayLargeRoad:
	case RecordTypeTwoWayLargeRoad:
		return LARGEROADPENALTY; // penalty for large roads
	case RecordTypeOneWayPrimary:
	case RecordTypeTwoWayPrimary:
		return PRIMARYROADPENALTY; // penalty for primary roads
	case RecordTypeOneWayHighway:
	case RecordTypeTwoWayHighway:
		return HIGHWAYROADPENALTY; // penalty for highway
	default:
		return INFINITY; // don't take this edge (not a road!!!)
	}
}

#define SMALLROADSPEED 25
#define LARGEROADSPEED 35
#define PRIMARYROADSPEED 45
#define HIGHWAYROADSPEED 65

float TimeFactor(const MapRecord * pRecord)
{
	switch (pRecord->eRecordType)
	{
	case RecordTypeOneWaySmallRoad:
	case RecordTypeTwoWaySmallRoad:
		return SECSPERHOUR / SMALLROADSPEED; // penalty for small roads
	case RecordTypeOneWayLargeRoad:
	case RecordTypeTwoWayLargeRoad:
		return SECSPERHOUR / LARGEROADSPEED; // penalty for large roads
	case RecordTypeOneWayPrimary:
	case RecordTypeTwoWayPrimary:
		return SECSPERHOUR / PRIMARYROADSPEED; // penalty for primary roads
	case RecordTypeOneWayHighway:
	case RecordTypeTwoWayHighway:
		return SECSPERHOUR / HIGHWAYROADSPEED; // penalty for highway
	default:
		return INFINITY; // don't take this edge (not a road!!!)
	}
}

float RecordDistance(const MapRecord * pRecord)
{
	int iSeg;
	float fDistance = 0;

	for (iSeg = 1; iSeg < pRecord->nShapePoints; iSeg++)
		fDistance += Distance(pRecord->pShapePoints[iSeg - 1], pRecord->pShapePoints[iSeg]); // add distance from shape point to shape point

	return fDistance;
}

float PointRecordDistance(const Coords & pt, const MapRecord * pRecord, unsigned short & iShapePointClosest, float & fProgressClosest)
{
	Coords * pt0, * pt1;
	float fDistanceClosest = 0., fDistance, fProgress;
	unsigned short i;
	bool bFound = false;

	iShapePointClosest = (unsigned)-1;
	pt1 = pRecord->pShapePoints;
	for (i = 0; i < pRecord->nShapePoints - 1; i++)
	{
		pt0 = pt1;
		pt1++;

		fDistance = PointSegmentDistance(pt, *pt0, *pt1, fProgress);

		if (fDistance < fDistanceClosest || !bFound)
		{
			fDistanceClosest = fDistance;
			fProgressClosest = fProgress;
			iShapePointClosest = i;
			bFound = true;
		}
	}

	return bFound ? fDistanceClosest : INFINITY;
}

bool IsVehicleGoingForwards(unsigned short iShapePoint, short iHeading, const MapRecord * pRecord)
{
	Coords s0, s1;
	float sx, sy, ax, ay;
	// take care of some non-trivial conditions

	// first, we need to make sure we're not out of the range
	if (iShapePoint > pRecord->nShapePoints - 1 || pRecord->nShapePoints < 2)
		return true;

	// next, if we're on the last shape point, look at the preceding segment
	if (iShapePoint == pRecord->nShapePoints - 1)
		iShapePoint--;

	s0 = pRecord->pShapePoints[iShapePoint].Flatten();
	s1 = pRecord->pShapePoints[iShapePoint+1].Flatten();
	sx = s1.m_iLong - s0.m_iLong;
	sy = s1.m_iLat - s0.m_iLat;

	ax = sinf(iHeading * RADIANSPERCENTIDEGREE);
	ay = cosf(iHeading * RADIANSPERCENTIDEGREE);

	return (sx*ax+sy*ay)/(sx*sx+sy*sy) >= 0.f;
}

float DistanceAlongRecord(const MapRecord * pRecord, unsigned short iStartShapePoint, float fStartProgress, unsigned short iEndShapePoint, float fEndProgress)
{
	unsigned short i;
	float fDistance = 0.f;

	// swap to ensure that start <= end
	if (iStartShapePoint > iEndShapePoint || iStartShapePoint == iEndShapePoint && fStartProgress > fEndProgress)
	{
		unsigned short iTemp = iStartShapePoint;
		float fTemp = fStartProgress;
		iStartShapePoint = iEndShapePoint;
		fStartProgress = fEndProgress;
		iEndShapePoint = iTemp;
		fEndProgress = fTemp;
	}

	for (i = iStartShapePoint; i < iEndShapePoint; i++)
		fDistance += Distance(pRecord->pShapePoints[i], pRecord->pShapePoints[i+1]);
	if (iEndShapePoint < pRecord->nShapePoints - 1)
		fDistance += Distance(pRecord->pShapePoints[iEndShapePoint], pRecord->pShapePoints[iEndShapePoint+1]) * fEndProgress;
	if (iStartShapePoint < pRecord->nShapePoints - 1)
		fDistance -= Distance(pRecord->pShapePoints[iStartShapePoint], pRecord->pShapePoints[iStartShapePoint+1]) * fStartProgress;
	return fDistance;
}

void InitMapDrawingSettings(MapDrawingSettings * pSettings)
{
	pSettings->rUpdate.setCoords(0, 0, -1, -1);
	pSettings->bL1Redraw = true;
	pSettings->bL2Redraw = true;
	pSettings->tLastChange = timeval0;
	pSettings->iCurrentObject = -1;
	pSettings->iDetailLevel = 4;
	pSettings->iControlWidth = 0;
	pSettings->iControlHeight = 0;
	pSettings->ptCenter.Set(0, 0);
	pSettings->ptTopLeft.Set(0, 0);
	pSettings->ptBottomRight.Set(0, 0);

	UpdateMapDrawingSettings(pSettings);
}

void UpdateMapDrawingSettings(MapDrawingSettings * pSettings)
{
	RecordTypes eRT;
	int iDetailLevel;

	pSettings->bFillInWater = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_FILLWATER_NUM].GetValue().bValue;
	pSettings->bShowCompass = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_SHOWCOMPASS_NUM].GetValue().bValue;
	pSettings->bShowScale = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_SHOWSCALE_NUM].GetValue().bValue;
	pSettings->bShowMarkers = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_SHOWMARKERS_NUM].GetValue().bValue;
	pSettings->bShowTracks = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_SHOWTRACKS_NUM].GetValue().bValue;
	pSettings->clrBackground = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_BG_NUM].GetValue().strValue;

//	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
//		pSettings->vecLevelScales[iDetailLevel] = 65536. * ::pow(0.5, iDetailLevel);

	// load appearance settings for each type of record (visibility info, etc.)
	eRT = RecordTypeDefault;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 5;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 5;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_FG_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_FG_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;
	}

	eRT = RecordTypeTwoWaySmallRoad;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 7;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 5;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_SMROAD_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_SMROAD_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 12;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;

		if (iDetailLevel >= 5)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
		}
		if (iDetailLevel >= 6)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 6;
		}
		if (iDetailLevel >= 7)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 1;
		}
	}
	eRT = RecordTypeOneWaySmallRoad;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 7;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 5;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_SMROAD_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_SMROAD_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 12;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 10;

		if (iDetailLevel >= 5)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
		}
		if (iDetailLevel >= 6)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 6;
		}
		if (iDetailLevel >= 7)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 1;
		}
	}
	eRT = RecordTypeTwoWayLargeRoad;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 4;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 9;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 7;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_LGROAD_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_LGROAD_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 16;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;

		if (iDetailLevel == 6)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 10;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 3;
		}
		if (iDetailLevel == 7)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		}
		if (iDetailLevel == 8)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 7;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		}
		if (iDetailLevel == 9)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 6;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 1;
		}
	}

	eRT = RecordTypeOneWayLargeRoad;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 4;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 9;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 7;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_LGROAD_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_LGROAD_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 16;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 10;

		if (iDetailLevel == 6)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 10;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 3;
		}
		if (iDetailLevel == 7)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		}
		if (iDetailLevel == 8)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 7;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		}
		if (iDetailLevel == 9)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 6;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 1;
		}
	}

	eRT = RecordTypeTwoWayPrimary;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 5;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 10;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 9;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_PRIMARY_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_PRIMARY_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 16;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;

		if (iDetailLevel == 7)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 12;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 5;
		}
		if (iDetailLevel == 8)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 9;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 4;
		}
		if (iDetailLevel == 9)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		}
		if (iDetailLevel == 10)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 7;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 1;
		}
	}

	eRT = RecordTypeOneWayPrimary;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 5;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 10;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 9;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_PRIMARY_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_PRIMARY_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 16;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 10;

		if (iDetailLevel == 7)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 12;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 5;
		}
		if (iDetailLevel == 8)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 9;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 4;
		}
		if (iDetailLevel == 9)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		}
		if (iDetailLevel == 10)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 7;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 1;
		}
	}

	eRT = RecordTypeTwoWayHighway;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 6;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 10;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 10;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_HIGHWAY_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_HIGHWAY_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 16;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;

		if (iDetailLevel == 7)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 12;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 5;
		}
		if (iDetailLevel == 8)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 10;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 4;
		}
		if (iDetailLevel == 9)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		}
		if (iDetailLevel == 10)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 1;
		}
	}

	eRT = RecordTypeOneWayHighway;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 6;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 10;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 10;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_HIGHWAY_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_HIGHWAY_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 16;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 10;

		if (iDetailLevel == 7)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 12;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 5;
		}
		if (iDetailLevel == 8)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 10;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 4;
		}
		if (iDetailLevel == 9)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		}
		if (iDetailLevel == 10)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 1;
		}
	}

	eRT = RecordTypeRailroad;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::DashLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 7;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 5;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_RAIL_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_RAIL_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 10;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;
	}

	eRT = RecordTypeWater;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 1;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 8;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 5;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_WATER_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_WATER_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 10;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;
	}

	eRT = RecordTypeInvisibleLandBoundary;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = iDetailLevel <= 7 ? 3 : 1;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = /*iDetailLevel <= 15 ? true : */false; // don't draw these
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = /*iDetailLevel <= 9 ? true : */false; // don't draw these
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_LANDBOUNDS_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_LANDBOUNDS_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 16;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;
	}

	eRT = RecordTypeInvisibleWaterBoundary;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = iDetailLevel <= 7 ? 3 : 1;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = /*iDetailLevel <= 15 ? true : */false; // don't draw these
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = /*iDetailLevel <= 9 ? true : */false; // don't draw these
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_WATERBOUNDS_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_WATERBOUNDS_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 16;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;
	}

	eRT = RecordTypeInvisibleMiscBoundary;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = iDetailLevel <= 7 ? 3 : 1;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = /*iDetailLevel <= 4 ? true : */false; // don't draw these
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = /*iDetailLevel <= 4 ? true : */false; // don't draw these
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_MISCBOUNDS_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_MISCBOUNDS_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 16;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;
	}

	eRT = RecordTypeLandmark;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 3;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 4;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 4;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_LANDMARK_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_LANDMARK_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;
	}

	eRT = RecordTypePhysicalFeature;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 6;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::SolidLine;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = /*iDetailLevel <= 10 ? true :*/ false; // don't bother to draw physical features (they're ugly!)
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = /*iDetailLevel <= 10 ? true :*/ false; // don't bother to draw physical features
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_PHYSICAL_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = g_pSettings->m_sSettings[SETTINGS_APPEARANCE_COLOR_PHYSICAL_NUM].GetValue().strValue;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 16;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;

		if (iDetailLevel == 7)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 12;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 5;
		}
		if (iDetailLevel == 8)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 4;
		}
		if (iDetailLevel == 9)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		}
		if (iDetailLevel == 10)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 1;
		}
	}

	eRT = RecordTypeTownOrCity;
	for (iDetailLevel = MIN_DETAIL_LEVEL; iDetailLevel <= MAX_DETAIL_LEVEL; iDetailLevel++)
	{
		QColor clr;

		pSettings->vecLevelDetails[iDetailLevel][eRT].iWidth = 2;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iStyle = Qt::NoPen;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bLineVisible = iDetailLevel <= 10 && iDetailLevel >= 7;
		pSettings->vecLevelDetails[iDetailLevel][eRT].bTextVisible = iDetailLevel <= 10 && iDetailLevel >= 7;

		if (iDetailLevel <= 8)
			clr.setRgb(192, 0, 0);
		else
			clr.setRgb(160, 160, 160);

		pSettings->vecLevelDetails[iDetailLevel][eRT].clrLine = clr;
		pSettings->vecLevelDetails[iDetailLevel][eRT].clrText = clr;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 14;
		pSettings->vecLevelDetails[iDetailLevel][eRT].iArrowSpacing = 0;

		if (iDetailLevel > 9)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 8;
		}
		else if (iDetailLevel > 8)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 10;
		}
		else if (iDetailLevel > 7)
		{
			pSettings->vecLevelDetails[iDetailLevel][eRT].iFontSize = 12;
		}
	}
}

void CopyMapDrawingSettings(MapDrawingSettings * pDest, const MapDrawingSettings * pSrc)
{
	memcpy(pDest->vecLevelDetails, pSrc->vecLevelDetails, sizeof(pSrc->vecLevelDetails));
	pDest->rUpdate = pSrc->rUpdate;
	pDest->ptTopLeft = pSrc->ptTopLeft;
	pDest->ptBottomRight = pSrc->ptBottomRight;
	pDest->ptCenter = pSrc->ptCenter;
	pDest->clrBackground = pSrc->clrBackground;
	pDest->iControlWidth = pSrc->iControlWidth;
	pDest->iControlHeight = pSrc->iControlHeight;
	pDest->iDetailLevel = pSrc->iDetailLevel;
	pDest->iCurrentObject = pSrc->iCurrentObject;
	pDest->tLastChange = pSrc->tLastChange;
	pDest->bL1Redraw = pSrc->bL1Redraw;
	pDest->bL2Redraw = pSrc->bL2Redraw;
	pDest->bShowMarkers = pSrc->bShowMarkers;
	pDest->bShowCompass = pSrc->bShowCompass;
	pDest->bShowScale = pSrc->bShowScale;
	pDest->bShowTracks = pSrc->bShowTracks;
	pDest->bFillInWater = pSrc->bFillInWater;
}

void ReplaceOldEverythingBmp(MapDrawingSettings * pDrawingSettings, QPixmap * pNewImage)
{
	if (pDrawingSettings->pOldEverythingBmp != NULL)
		delete pDrawingSettings->pOldEverythingBmp;
	pDrawingSettings->pOldEverythingBmp = pNewImage;
}

void ReplaceOldMapBmp(MapDrawingSettings * pDrawingSettings, QPixmap * pNewImage)
{
	if (pDrawingSettings->pOldMapBmp != NULL)
		delete pDrawingSettings->pOldMapBmp;
	pDrawingSettings->pOldMapBmp = pNewImage;
}

void InvalidateRect(MapDrawingSettings * pSettings, const QRect & rClip, bool bL1, bool bL2)
{
	if (bL1)
		pSettings->bL1Redraw = true;
	if (bL2)
		pSettings->bL2Redraw = true;

	if (rClip.isEmpty())
		pSettings->rUpdate.setRect(0, 0, pSettings->iControlWidth-1, pSettings->iControlHeight-1);
	else
	{
		if (pSettings->rUpdate.isEmpty())
			pSettings->rUpdate = rClip;
		else
			pSettings->rUpdate = pSettings->rUpdate.unite(rClip);

		if (pSettings->rUpdate.left() < 0)
			pSettings->rUpdate.setLeft(0);
		if (pSettings->rUpdate.right() > pSettings->iControlWidth-1)
			pSettings->rUpdate.setRight(pSettings->iControlWidth-1);
		if (pSettings->rUpdate.top() < 0)
			pSettings->rUpdate.setTop(0);
		if (pSettings->rUpdate.bottom() > pSettings->iControlHeight-1)
			pSettings->rUpdate.setBottom(pSettings->iControlHeight-1);
	}
}

bool IsOnScreen(const Coords & pt, const Coords & ptTopLeft, const Coords & ptBottomRight)
{
	return pt.m_iLong >= ptTopLeft.m_iLong && pt.m_iLong <= ptBottomRight.m_iLong && pt.m_iLat >= ptBottomRight.m_iLat && pt.m_iLat <= ptTopLeft.m_iLat;
}

bool IsRecordVisible(Coords * coords, int nCoords, const Coords & ptTopLeft, const Coords & ptBottomRight)
{
	if (nCoords == 0) return false;
	Coords * p1, * p2 = coords;
	int i;
	double tX1, tY1, tX2, tY2, swap;
	for (i = 0; i < nCoords - 1; i++) {
		p1 = p2;
		++p2;
		if (IsOnScreen(*p1, ptTopLeft, ptBottomRight)) return true;

		// test if line from p1 to p2 intersects bounding rectangle
		if (p2->m_iLat == p1->m_iLat) {
			if (p2->m_iLat < ptTopLeft.m_iLat && p2->m_iLat > ptBottomRight.m_iLat) {
				if ((p1->m_iLong < ptTopLeft.m_iLong && p2->m_iLong > ptBottomRight.m_iLong) || (p2->m_iLong < ptTopLeft.m_iLong && p1->m_iLong > ptBottomRight.m_iLong))
					return true;
			}
		} else if (p2->m_iLong == p1->m_iLong) {
			if (p2->m_iLong > ptTopLeft.m_iLong && p2->m_iLong < ptBottomRight.m_iLong) {
				if ((p1->m_iLat > ptTopLeft.m_iLat && p2->m_iLat < ptBottomRight.m_iLat) || (p2->m_iLat > ptTopLeft.m_iLat && p1->m_iLat < ptBottomRight.m_iLat))
					return true;
			}
		} else {
			tX1 = ((double)(ptTopLeft.m_iLong - p1->m_iLong)) / (p2->m_iLong - p1->m_iLong);
			tX2 = ((double)(ptBottomRight.m_iLong - p1->m_iLong)) / (p2->m_iLong - p1->m_iLong);
			tY1 = ((double)(ptBottomRight.m_iLat - p1->m_iLat)) / (p2->m_iLat - p1->m_iLat);
			tY2 = ((double)(ptTopLeft.m_iLat - p1->m_iLat)) / (p2->m_iLat - p1->m_iLat);
			if (tX2 < tX1) {
				swap = tX1;
				tX1 = tX2;
				tX2 = swap;
			}
			if (tY2 < tY1) {
				swap = tY1;
				tY1 = tY2;
				tY2 = swap;
			}
			if (tY1 < tX2 && tX1 < tY2) return true;
		}
	}
	return IsOnScreen(*p2, ptTopLeft, ptBottomRight);
}

bool IsRecordVisible(const std::vector<Coords> & coords, const Coords & ptTopLeft, const Coords & ptBottomRight)
{
	if (coords.size() == 0) return false;
	std::vector<Coords>::const_iterator p1, p2 = coords.begin();
	int i;
	double tX1, tY1, tX2, tY2, swap;
	for (i = 0; i < (int)coords.size() - 1; i++) {
		p1 = p2;
		++p2;
		if (IsOnScreen(*p1, ptTopLeft, ptBottomRight)) return true;

		// test if line from p1 to p2 intersects bounding rectangle
		if (p2->m_iLat == p1->m_iLat) {
			if (p2->m_iLat < ptTopLeft.m_iLat && p2->m_iLat > ptBottomRight.m_iLat) {
				if ((p1->m_iLong < ptTopLeft.m_iLong && p2->m_iLong > ptBottomRight.m_iLong) || (p2->m_iLong < ptTopLeft.m_iLong && p1->m_iLong > ptBottomRight.m_iLong))
					return true;
			}
		} else if (p2->m_iLong == p1->m_iLong) {
			if (p2->m_iLong > ptTopLeft.m_iLong && p2->m_iLong < ptBottomRight.m_iLong) {
				if ((p1->m_iLat > ptTopLeft.m_iLat && p2->m_iLat < ptBottomRight.m_iLat) || (p2->m_iLat > ptTopLeft.m_iLat && p1->m_iLat < ptBottomRight.m_iLat))
					return true;
			}
		} else {
			tX1 = ((double)(ptTopLeft.m_iLong - p1->m_iLong)) / (p2->m_iLong - p1->m_iLong);
			tX2 = ((double)(ptBottomRight.m_iLong - p1->m_iLong)) / (p2->m_iLong - p1->m_iLong);
			tY1 = ((double)(ptBottomRight.m_iLat - p1->m_iLat)) / (p2->m_iLat - p1->m_iLat);
			tY2 = ((double)(ptTopLeft.m_iLat - p1->m_iLat)) / (p2->m_iLat - p1->m_iLat);
			if (tX2 < tX1) {
				swap = tX1;
				tX1 = tX2;
				tX2 = swap;
			}
			if (tY2 < tY1) {
				swap = tY1;
				tY1 = tY2;
				tY2 = swap;
			}
			if (tY1 < tX2 && tX1 < tY2) return true;
		}
	}
	return IsOnScreen(*p2, ptTopLeft, ptBottomRight);
}

void DrawLine(QPainter * dc, int sx, int sy, int dx, int dy, int iWidth, const QColor & clrColor, int iStyle)
{
	QPen pen(clrColor, iWidth, (Qt::PenStyle)iStyle);
	QPen oldPen = dc->pen();
	dc->setPen(pen);
	dc->drawLine(sx, sy, dx, dy);
	dc->setPen(oldPen);
}

void DrawRotatedText(QPainter * dc, const QString & str, const QPoint & p, const QSize & sz, double angle, const QColor & clrText)
{
	QPen oldPen(dc->pen());
	dc->setPen(clrText);
	dc->rotate(-angle);
	QPoint ptRotated = dc->xFormDev(p);
	dc->drawText(ptRotated.x() - sz.width() / 2, ptRotated.y() - sz.height() / 2, str);
	dc->resetXForm();
	dc->setPen(oldPen);
}

void AddRecordToVertex(Vertex * pVertex, const MapRecord * pRecordSet, unsigned int iRecord, unsigned int iPreviousVertex)
{
	unsigned int i;
	pVertex->mapEdges.insert(std::pair<unsigned int, unsigned>(iRecord, iPreviousVertex));
	if (IsRoad(pRecordSet + iRecord))
	{
		for (i = 0; i < pVertex->vecRoads.size(); i++)
		{
			// same as existing road
			if (IsSameRoad(pRecordSet + pVertex->vecRoads[i], pRecordSet + iRecord))
				return;
		}
		// new road
		pVertex->vecRoads.push_back(iRecord);
	}
}

bool CanCarGoThrough(const Vertex & vertex, unsigned int iRecord)
{
	return IsSameRoad(g_pMapDB->GetRecord(iRecord), g_pMapDB->GetRecord(vertex.vecRoads[vertex.iRoadPermitted]));
}


MapDB::MapDB()
: m_pRecords(NULL), m_nRecords(0), m_bTrafficLights(false)
{
	m_tLastChange = GetCurrentTime();
}

MapDB::~MapDB()
{
	Clear();
}

void MapDB::Clear()
{
	unsigned int iRec;

	m_Mutex.lock();
	for (iRec = 0; iRec < m_nRecords; iRec++)
	{
		if (m_pRecords[iRec].pAddressRanges != NULL) delete[] m_pRecords[iRec].pAddressRanges;
		if (m_pRecords[iRec].pFeatureNames != NULL) delete[] m_pRecords[iRec].pFeatureNames;
		if (m_pRecords[iRec].pFeatureTypes != NULL) delete[] m_pRecords[iRec].pFeatureTypes;
		if (m_pRecords[iRec].pShapePoints != NULL) delete[] m_pRecords[iRec].pShapePoints;
	}

	m_nRecords = 0;
	if (m_pRecords != NULL) {
		delete[] m_pRecords;
		m_pRecords = NULL;
	}
	m_vecVertices.clear();
	m_vecStringRoads.clear();
	m_vecStrings.clear();

	m_mapCoordinateToVertex.clear();
	m_mapTLIDtoRecord.clear();
	m_mapPolyIDtoRecord.clear();
	m_mapPolyIDtoSide.clear();
	m_mapCountyCodeToWaterPolys.clear();
	m_mapAdditionalNameIDtoRecords.clear();
	m_mapStringsToIndex.clear();
	m_mapCountyCodeToRecords.clear();
	m_mapCountyCodeToBoundingRect.clear();
	m_mapCountyCodeToRegions.clear();
	m_vecVerticesHeapLookup.clear();

	m_Mutex.unlock();
}

void MapDB::ResetTrafficLights()
{
	unsigned int i;

	for (i = 0; i < m_vecVertices.size(); i++)
		m_vecVertices[i].iRoadPermitted = 0;
}

void MapDB::GetStreetsByName(const QString & strStreetName, const QString & strStreetType, std::set<unsigned int> & setMatching)
{
	unsigned int iStreetName, iStreetType, i, j;

	iStreetName = g_pMapDB->GetStringIndex(strStreetName);
	iStreetType = g_pMapDB->GetStringIndex(strStreetType);

	if (iStreetName == (unsigned)-1 || iStreetType == (unsigned)-1)
		return;

	for (i = 0; i < m_nRecords; i++)
	{
		if (!IsRoad(m_pRecords + i))
			continue;
		for (j = 0; j < m_pRecords[i].nFeatureNames; j++)
		{
			if (m_pRecords[i].pFeatureNames[j] == iStreetName && m_pRecords[i].pFeatureTypes[j] == iStreetType)
			{
				setMatching.insert(i);
				break;
			}
		}
	}
}

QString MapDB::GetNameAndType(unsigned int iRec, int iEntry, bool bLongTypeForm)
{
	if (iRec < m_nRecords && iEntry < m_pRecords[iRec].nFeatureNames)
	{
		QString strType = "";

		if (bLongTypeForm)
			strType = GetFullName(m_vecStrings[m_pRecords[iRec].pFeatureTypes[iEntry]]);

		if (strType == "")
			strType = m_vecStrings[m_pRecords[iRec].pFeatureTypes[iEntry]];

		if (strType != "")
			strType = " " + strType;

		return m_vecStrings[m_pRecords[iRec].pFeatureNames[iEntry]] + strType;
	} else
		return "";
}

bool MapDB::IsVertex(unsigned int iVertex, const std::set<StreetNameAndType> & setStreets)
{
	std::set<StreetNameAndType> setFound;
	std::set<StreetNameAndType>::iterator iterStreet;
	StreetNameAndType pairNameAndType;
	std::map<unsigned int, unsigned int>::iterator iterSeg;
	unsigned int i;

	for (iterSeg = m_vecVertices[iVertex].mapEdges.begin(); iterSeg != m_vecVertices[iVertex].mapEdges.end(); ++iterSeg)
	{
		iterStreet = setStreets.end();
		MapRecord * pRecord = m_pRecords + iterSeg->first;
		if (!IsRoad(pRecord) || pRecord->nFeatureNames == 0)
			continue;

		for (i = 0; i < pRecord->nFeatureNames; i++)
		{
			pairNameAndType.iStreetName = pRecord->pFeatureNames[i];
			pairNameAndType.iStreetType = pRecord->pFeatureTypes[i];
			if ((iterStreet = setStreets.find(pairNameAndType)) != setStreets.end())
			{
				setFound.insert(pairNameAndType);
				break;
			}
		}

		if (iterStreet == setStreets.end()) // not the intersection we're looking for
			return false;
	}

	return setFound.size() == setStreets.size();
}

bool MapDB::GetVertex(unsigned int iVertex, Address * pAddress)
{
	std::map<unsigned int, unsigned int>::iterator iterSeg;
	QString strName, strRoadName;
	Coords ptCenter;
	std::set<QString> setRoadNames;

	pAddress->iVertex = iVertex;
	pAddress->iRecord = (unsigned)-1;
	pAddress->iStreetNumber = 0;
	pAddress->szStreetType = "";

	for (iterSeg = m_vecVertices[iVertex].mapEdges.begin(); iterSeg != m_vecVertices[iVertex].mapEdges.end(); ++iterSeg)
	{
		if (IsRoad(m_pRecords + iterSeg->first))
		{
			if (pAddress->iRecord == (unsigned)-1)
				pAddress->iRecord = iterSeg->first;

			strRoadName = GetNameAndType(iterSeg->first);
			if (!strRoadName.isEmpty() && setRoadNames.insert(strRoadName).second)
			{
				if (strName.isEmpty())
					strName = strRoadName;
				else
					strName += " & " + strRoadName;
			}
		}
	}

	if (pAddress->iRecord == (unsigned)-1)
		return false;

	pAddress->szStreetName = strName;

	if (m_pRecords[pAddress->iRecord].nAddressRanges > 0)
		pAddress->iZipCode = m_pRecords[pAddress->iRecord].pAddressRanges[0].iZip;
	else
		pAddress->iZipCode = 0;
	pAddress->szCityName = CityByZip(pAddress->iZipCode);
	pAddress->szStateName = StateByZip(pAddress->iZipCode);

	if (m_pRecords[pAddress->iRecord].pVertices[0] == iVertex)
		ptCenter = m_pRecords[pAddress->iRecord].pShapePoints[0];
	else
		ptCenter = m_pRecords[pAddress->iRecord].pShapePoints[m_pRecords[pAddress->iRecord].nShapePoints - 1];
	pAddress->ptCoordinates = ptCenter;
	return true;
}

bool MapDB::IsCountyLoaded(unsigned short iFIPSCode)
{
	return m_mapCountyCodeToRecords.find(iFIPSCode) != m_mapCountyCodeToRecords.end();
}

bool MapDB::DownloadCounties(const std::set<unsigned short> & setFIPSCodes)
{
	std::set<unsigned short>::iterator iterFIPSCode;
	std::set<QString> filesLoad;
	bool bProcessable, bRetVal = false;
	unsigned short iCombinedCode, iStateCode;
	QString strCode, strState, strProcessedFile, strUnzippedFile, strZippedFile;

	qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	for (iterFIPSCode = setFIPSCodes.begin(); iterFIPSCode != setFIPSCodes.end(); ++iterFIPSCode)
	{
		iCombinedCode = *iterFIPSCode;
		iStateCode = iCombinedCode / 1000;

		if (IsCountyLoaded(iCombinedCode))
			continue;

		strState = StateAbbreviationByCode(iStateCode);
		strCode.sprintf("%05d", iCombinedCode);
		strProcessedFile = GetDataPath(QString("%1.MAP").arg(strCode));
		if (strProcessedFile.isEmpty() || !QFileInfo(strProcessedFile).exists() || !QFileInfo(strProcessedFile).isFile() || !QFileInfo(strProcessedFile).isReadable()) {
			// processed file not found
			strUnzippedFile = GetDataPath(QString("TGR%1.RT1").arg(strCode));
			bProcessable = false;
			if (strUnzippedFile.isEmpty() || !QFileInfo(strUnzippedFile).exists() || !QFileInfo(strUnzippedFile).isFile() || !QFileInfo(strUnzippedFile).isReadable()) {
				// unzipped file not found
				strZippedFile = GetDataPath(QString("tgr%1.zip").arg(strCode), QString("%1%2/tgr%3.zip").arg(TIGER_BASE_URL).arg(strState).arg(strCode), false);

				QDir dataDir(GetDataPath());
				if (!strZippedFile.isEmpty() && QFileInfo(strZippedFile).exists() && QFileInfo(strZippedFile).isFile() && QFileInfo(strZippedFile).isReadable()) {
					// zipped file downloaded - unzip now
					system(QString("unzip -uoqq %1 -d %2").arg(strZippedFile).arg(dataDir.absPath()));
					strUnzippedFile = GetDataPath(QString("TGR%1.RT1").arg(strCode));
					if (!strUnzippedFile.isEmpty() && QFileInfo(strUnzippedFile).exists() && QFileInfo(strUnzippedFile).isFile() && QFileInfo(strUnzippedFile).isReadable())
						bProcessable = true;

					dataDir.remove(strZippedFile);
				}
			} else
				bProcessable = true;
			if (bProcessable) {
				TIGERProcessor processor;
				if (!processor.LoadSet(strUnzippedFile)) { // process files
					strProcessedFile = GetDataPath(QString("%1.MAP").arg(strCode));
					if (!strProcessedFile.isEmpty() && QFileInfo(strProcessedFile).exists() && QFileInfo(strProcessedFile).isFile() && QFileInfo(strProcessedFile).isReadable())
						filesLoad.insert(strProcessedFile);
				}
			}
		} else
			filesLoad.insert(strProcessedFile);
	}

	if (!filesLoad.empty())
		bRetVal = LoadAll(GetDataPath(), filesLoad); // load processed files
	qApp->restoreOverrideCursor();
	return bRetVal;
}

bool MapDB::DownloadCounty(unsigned short iFIPSCode)
{
	std::set<unsigned short> setFIPSCodes;
	setFIPSCodes.insert(iFIPSCode);
	return DownloadCounties(setFIPSCodes);
}

bool MapDB::LoadAll(const QString & strDirectory)
{
	QDir dir(strDirectory);
	QStringList listFiles;
	QStringList::iterator iterFiles;
	QFileInfo fileInfo;
	QString filePath;
	unsigned int nRecordsOld = m_nRecords;

	if (!dir.exists())
		return false;

	m_Mutex.lock();

	listFiles = dir.entryList("*.MAP", QDir::Files|QDir::Readable);
	iterFiles = listFiles.begin();
	while (iterFiles != listFiles.end()) {
		filePath = dir.absFilePath(*iterFiles);
		g_pLogger->LogInfo(QString("Loading map \"%1\"...").arg(filePath));
		if (LoadMap(filePath))
			g_pLogger->LogInfo("Successful\n");
		else
			g_pLogger->LogInfo("Failed\n");
		++iterFiles;
		qApp->processEvents();
	}

	if (nRecordsOld < m_nRecords) {
		m_tLastChange = GetCurrentTime();
		m_vecVerticesHeapLookup.resize(m_vecVertices.size());
	}
	m_Mutex.unlock();
	return nRecordsOld < m_nRecords;
}

bool MapDB::LoadAll(const QString & strDirectory, const std::set<QString> & setFilenames)
{
	QDir dir(strDirectory);
	std::set<QString>::const_iterator iterFile;
	QFileInfo fileInfo;
	QString filePath;
	unsigned int nRecordsOld = m_nRecords;

	if (!dir.exists() || setFilenames.empty())
		return false;

	m_Mutex.lock();

	for (iterFile = setFilenames.begin(); iterFile != setFilenames.end(); ++iterFile)
	{
		filePath = dir.absFilePath(*iterFile);
		fileInfo = filePath;
		if (filePath != "" && fileInfo.exists() && fileInfo.isFile() && fileInfo.isReadable() && strcasecmp(fileInfo.extension(false), "MAP") == 0)
		{
			g_pLogger->LogInfo(QString("Loading map \"%1\"...").arg(filePath));
			if (LoadMap(filePath))
				g_pLogger->LogInfo("Successful\n");
			else
				g_pLogger->LogInfo("Failed\n");
		}
		qApp->processEvents();
	}

	if (nRecordsOld < m_nRecords) {
		m_tLastChange = GetCurrentTime();
		m_vecVerticesHeapLookup.resize(m_vecVertices.size());
	}
	m_Mutex.unlock();
	return nRecordsOld < m_nRecords;
}

void MapDB::DrawMap(MapDrawingSettings * pSettings, const QRect & rMap)
{
	pSettings->pMemoryDC->fillRect(rMap, pSettings->clrBackground);

	DrawMapFeatures(pSettings);
	DrawMapCompass(pSettings);
	DrawMapKey(pSettings);
}

void MapDB::DrawBorder(MapDrawingSettings * pSettings, bool bFocus)
{
	int iWidth = 2;
	QColor clrSegment = bFocus ? QColor(0, 0, 0) : QColor(211, 211, 211);

	DrawLine(pSettings->pMemoryDC, 0, 0, pSettings->iControlWidth, 0, iWidth, clrSegment, Qt::SolidLine);
	DrawLine(pSettings->pMemoryDC, pSettings->iControlWidth - 1, 0, pSettings->iControlWidth - 1, pSettings->iControlHeight, iWidth, clrSegment, Qt::SolidLine);
	DrawLine(pSettings->pMemoryDC, 0, pSettings->iControlHeight - 1, pSettings->iControlWidth, pSettings->iControlHeight - 1, iWidth, clrSegment, Qt::SolidLine);
	DrawLine(pSettings->pMemoryDC, 0, 0, 0, pSettings->iControlHeight, iWidth, clrSegment, Qt::SolidLine);

	return;
}

void MapDB::DrawRecordHighlights(MapDrawingSettings * pSettings, const std::set<unsigned int> & setRecords, const QColor & clrHighlight)
{
	std::set<unsigned int>::const_iterator iterRecord;
	Rect bounds(pSettings->ptTopLeftClip, pSettings->ptBottomRightClip);
	bool bVisible;
	std::vector<QPoint> parPoints;
	MapRecord * psRec;
	int iSeg;
	QPen penThis, oldPen;
	QBrush brThis, oldBrush;

	for (iterRecord = setRecords.begin(); iterRecord != setRecords.end(); ++iterRecord)
	{
		psRec = m_pRecords + *iterRecord;
		bVisible = psRec->rBounds.intersectRect(bounds);
		if (bVisible && pSettings->vecLevelDetails[pSettings->iDetailLevel][psRec->eRecordType].bLineVisible)
		{
			parPoints.resize(psRec->nShapePoints);
			for (iSeg = 0; iSeg < psRec->nShapePoints; iSeg++)
				parPoints[iSeg] = MapLongLatToScreen(pSettings, psRec->pShapePoints[iSeg]);

			if (psRec->nShapePoints == 1)
			{
				// a place
				penThis = QPen(clrHighlight, 1, Qt::SolidLine);
				brThis = QBrush(clrHighlight);

				oldPen = pSettings->pMemoryDC->pen();
				oldBrush = pSettings->pMemoryDC->brush();
				pSettings->pMemoryDC->setPen(penThis);
				pSettings->pMemoryDC->setBrush(brThis);

				pSettings->pMemoryDC->drawPie(parPoints[0].x(), parPoints[0].y(), 3, 3, 0, 5760);

				pSettings->pMemoryDC->setPen(oldPen);
				pSettings->pMemoryDC->setBrush(oldBrush);
			}
			for (iSeg = 0; iSeg < psRec->nShapePoints - 1; iSeg++)
				DrawLine(pSettings->pMemoryDC, parPoints[iSeg].x(), parPoints[iSeg].y(), parPoints[iSeg+1].x(), parPoints[iSeg+1].y(), 3, clrHighlight, Qt::DashLine);
		}
	}
}

bool MapDB::LoadMap(const QString & strBaseName)
{
	std::vector<unsigned int> vecStrings; // maps string codes to record set string codes
	std::vector<unsigned int> vecVertices; // maps file vertex codes to record set vertex codes
	std::vector<Vertex> vertexInfo; // for each vertex index, stores a list of edges (records)...
	std::map<unsigned short, CountySquares>::iterator iterSquares;
	std::map<unsigned short, Rect>::iterator iterBoundary;
	std::map<unsigned short, WaterPolygons>::iterator polys;
	MapRecord * newRecordBuffer = NULL;
	unsigned char * buffer, * startBuffer;
	unsigned short countyCode;
	Rect boundingRect;
	Coords vertexCoords;
	unsigned int i, j, numStrings, numVertices, neighborVertex, neighborRecord, numRecords, vertexMapIndex, numPolys, numPoints;
	unsigned char recordType;
	unsigned short numNeighbors;
	std::map<Coords, unsigned int>::iterator vertexCoordsIter;
	std::map<unsigned int, unsigned int>::iterator vertexNeighborIter;
	char szString[256]; // no string longer than this...
	bool bSuccess = false;
	struct stat fileInfo;

	int handle = open(strBaseName.ascii(), O_RDONLY);
	if (handle == -1) return false;

	// get buffer
	fstat(handle, &fileInfo);
	buffer = startBuffer = (unsigned char *)mmap(0, fileInfo.st_size, PROT_READ, MAP_PRIVATE, handle, 0);

	// read header
	memcpy(&countyCode, buffer, sizeof(unsigned short));
	if (IsCountyLoaded(countyCode)) goto TIGERPROCESSOR_LOAD_ERROR;

	buffer += sizeof(int);
	memcpy(&boundingRect.m_iLeft, buffer, sizeof(long));
	buffer += sizeof(long);
	memcpy(&boundingRect.m_iTop, buffer, sizeof(long));
	buffer += sizeof(long);
	memcpy(&boundingRect.m_iRight, buffer, sizeof(long));
	buffer += sizeof(long);
	memcpy(&boundingRect.m_iBottom, buffer, sizeof(long));
	buffer += sizeof(long);
	iterBoundary = m_mapCountyCodeToBoundingRect.insert(std::pair<unsigned short, Rect>(countyCode, boundingRect)).first;

	// read strings
	memcpy(&numStrings, buffer, sizeof(unsigned int));
	buffer += sizeof(unsigned int);
	m_vecStrings.reserve(m_vecStrings.size() + numStrings);
	vecStrings.resize(numStrings);
	for (i = 0; i < numStrings; i++) {
		strncpy(szString, (const char *)buffer, 256);
		buffer += (strlen(szString) + 1);
		vecStrings[i] = AddString(QString(szString).stripWhiteSpace());
	}
	m_vecStringRoads.resize(m_vecStrings.size());

	// read vertices
	memcpy(&numVertices, buffer, sizeof(unsigned int));
	buffer += sizeof(unsigned int);
	vecVertices.resize(numVertices);
	vertexInfo.resize(numVertices);
	m_vecVertices.reserve(m_vecVertices.size() + vecVertices.size());
	for (i = 0; i < numVertices; i++) {
		memcpy(&vertexCoords.m_iLong, buffer, sizeof(long));
		memcpy(&vertexCoords.m_iLat, (buffer += sizeof(long)), sizeof(long));
		vertexCoordsIter = m_mapCoordinateToVertex.find(vertexCoords);
		if (vertexCoordsIter == m_mapCoordinateToVertex.end()) {
			vertexCoordsIter = m_mapCoordinateToVertex.insert(std::pair<Coords, unsigned int>(vertexCoords, m_vecVertices.size())).first;
			m_vecVertices.push_back(Vertex());
			m_vecVertices.back().iRoadPermitted = 0;
		}
		vecVertices[i] = vertexCoordsIter->second;
		memcpy(&numNeighbors, buffer += sizeof(long), sizeof(unsigned short));
		buffer += sizeof(unsigned short);
		for (j = 0; j < numNeighbors; j++)
		{
			memcpy(&neighborVertex, buffer, sizeof(unsigned int));
			memcpy(&neighborRecord, (buffer += sizeof(unsigned int)), sizeof(unsigned int));
			buffer += sizeof(unsigned int);
			neighborRecord += m_nRecords;
			vertexInfo[i].mapEdges.insert(std::pair<unsigned int, unsigned int>(neighborRecord, neighborVertex));
		}
		memcpy(&numNeighbors, buffer, sizeof(unsigned short));
		buffer += sizeof(unsigned short);
		vertexInfo[i].vecRoads.resize(numNeighbors);
		for (j = 0; j < numNeighbors; j++)
		{
			memcpy(&vertexInfo[i].vecRoads[j], buffer, sizeof(unsigned int));
			vertexInfo[i].vecRoads[j] += m_nRecords;
			buffer += sizeof(unsigned int);
		}
	}
	// postprocessing step... (realign vertex indexes within record set)
	for (i = 0; i < numVertices; i++) {
		for (vertexNeighborIter = vertexInfo[i].mapEdges.begin(); vertexNeighborIter != vertexInfo[i].mapEdges.end(); ++vertexNeighborIter) {
			vertexMapIndex = vecVertices[vertexNeighborIter->second];
			m_vecVertices[vecVertices[i]].mapEdges.insert(std::pair<unsigned int, unsigned int>(vertexNeighborIter->first, vertexMapIndex));
		}
		m_vecVertices[vecVertices[i]].vecRoads.insert(m_vecVertices[vecVertices[i]].vecRoads.end(), vertexInfo[i].vecRoads.begin(), vertexInfo[i].vecRoads.end());
	}

	// read records
	memcpy(&numRecords, buffer, sizeof(unsigned int));
	buffer += sizeof(unsigned int);
	newRecordBuffer = new MapRecord[m_nRecords + numRecords];
	memcpy(newRecordBuffer, m_pRecords, m_nRecords * sizeof(MapRecord));
	for (i = 0; i < numRecords; i++) {
		memcpy(&newRecordBuffer[i+m_nRecords].nFeatureNames, buffer, sizeof(unsigned short));
		buffer += sizeof(unsigned short);
		newRecordBuffer[i + m_nRecords].pFeatureNames = new unsigned int[newRecordBuffer[i + m_nRecords].nFeatureNames];
		newRecordBuffer[i + m_nRecords].pFeatureTypes = new unsigned int[newRecordBuffer[i + m_nRecords].nFeatureNames];
		for (j = 0; j < newRecordBuffer[i + m_nRecords].nFeatureNames; j++) {
			memcpy(newRecordBuffer[i + m_nRecords].pFeatureNames + j, buffer, sizeof(unsigned int));
			newRecordBuffer[i + m_nRecords].pFeatureNames[j] = vecStrings[newRecordBuffer[i + m_nRecords].pFeatureNames[j]];
			m_vecStringRoads[newRecordBuffer[i + m_nRecords].pFeatureNames[j]].push_back(i + m_nRecords);
			memcpy(newRecordBuffer[i + m_nRecords].pFeatureTypes + j, (buffer += sizeof(unsigned int)), sizeof(unsigned int));
			buffer += sizeof(unsigned int);
			newRecordBuffer[i + m_nRecords].pFeatureTypes[j] = vecStrings[newRecordBuffer[i + m_nRecords].pFeatureTypes[j]];
		}
		memcpy(&recordType, buffer, sizeof(unsigned char));
		newRecordBuffer[i + m_nRecords].bWaterL = ((recordType & 0x80) == 0x80);
		newRecordBuffer[i + m_nRecords].bWaterR = ((recordType & 0x40) == 0x40);
		newRecordBuffer[i + m_nRecords].eRecordType = (RecordTypes)(recordType & 0x3f);
		memcpy(&newRecordBuffer[i + m_nRecords].fCost, (buffer += sizeof(unsigned char)), sizeof(float));
		memcpy(&newRecordBuffer[i + m_nRecords].ptWaterL.m_iLong, (buffer += sizeof(float)), sizeof(long));
		memcpy(&newRecordBuffer[i + m_nRecords].ptWaterL.m_iLat, (buffer += sizeof(long)), sizeof(long));
		memcpy(&newRecordBuffer[i + m_nRecords].ptWaterR.m_iLong, (buffer += sizeof(long)), sizeof(long));
		memcpy(&newRecordBuffer[i + m_nRecords].ptWaterR.m_iLat, (buffer += sizeof(long)), sizeof(long));
		memcpy(&newRecordBuffer[i + m_nRecords].nAddressRanges, (buffer += sizeof(long)), sizeof(unsigned short));
		buffer += sizeof(unsigned short);
		newRecordBuffer[i + m_nRecords].pAddressRanges = new AddressRange[newRecordBuffer[i + m_nRecords].nAddressRanges];
		for (j = 0; j < newRecordBuffer[i + m_nRecords].nAddressRanges; j++) {
			memcpy(&newRecordBuffer[i + m_nRecords].pAddressRanges[j].iFromAddr, buffer, sizeof(unsigned short));
			memcpy(&newRecordBuffer[i + m_nRecords].pAddressRanges[j].iToAddr, (buffer += sizeof(unsigned short)), sizeof(unsigned short));
			memcpy(&newRecordBuffer[i + m_nRecords].pAddressRanges[j].iZip, (buffer += sizeof(unsigned short)), sizeof(int));
			buffer += sizeof(int);
			newRecordBuffer[i + m_nRecords].pAddressRanges[j].bOnLeft = ((newRecordBuffer[i + m_nRecords].pAddressRanges[j].iZip & 0x80000000) == 0x80000000);
			newRecordBuffer[i + m_nRecords].pAddressRanges[j].iZip &= 0x7fffffff;
		}
		memcpy(&newRecordBuffer[i + m_nRecords].rBounds.m_iLeft, buffer, sizeof(long));
		memcpy(&newRecordBuffer[i + m_nRecords].rBounds.m_iTop, (buffer += sizeof(long)), sizeof(long));
		memcpy(&newRecordBuffer[i + m_nRecords].rBounds.m_iRight, (buffer += sizeof(long)), sizeof(long));
		memcpy(&newRecordBuffer[i + m_nRecords].rBounds.m_iBottom, (buffer += sizeof(long)), sizeof(long));
		memcpy(&newRecordBuffer[i + m_nRecords].nShapePoints, (buffer += sizeof(long)), sizeof(unsigned short));
		buffer += sizeof(unsigned short);
		newRecordBuffer[i + m_nRecords].pShapePoints = new Coords[newRecordBuffer[i + m_nRecords].nShapePoints];
		for (j = 0; j < newRecordBuffer[i + m_nRecords].nShapePoints; j++) {
			memcpy(&newRecordBuffer[i + m_nRecords].pShapePoints[j].m_iLong, buffer, sizeof(long));
			memcpy(&newRecordBuffer[i + m_nRecords].pShapePoints[j].m_iLat, (buffer += sizeof(long)), sizeof(long));
			buffer += sizeof(long);
		}
		memcpy(&newRecordBuffer[i + m_nRecords].nVertices, buffer, sizeof(unsigned short));
		buffer += sizeof(unsigned short);
		newRecordBuffer[i + m_nRecords].pVertices = new unsigned int[newRecordBuffer[i + m_nRecords].nVertices];
		for (j = 0; j < newRecordBuffer[i + m_nRecords].nVertices; j++) {
			memcpy(newRecordBuffer[i + m_nRecords].pVertices + j, buffer, sizeof(unsigned int));
			buffer += sizeof(unsigned int);
			newRecordBuffer[i + m_nRecords].pVertices[j] = vecVertices[newRecordBuffer[i + m_nRecords].pVertices[j]];
		}
	}
	delete[] m_pRecords;
	m_pRecords = newRecordBuffer;
	newRecordBuffer = NULL;

	// read water polygons
	memcpy(&numPolys, buffer, sizeof(unsigned int));
	buffer += sizeof(unsigned int);
	polys = m_mapCountyCodeToWaterPolys.insert(std::pair<unsigned short, WaterPolygons>(countyCode, WaterPolygons())).first;
	polys->second.resize(numPolys);
	for (i = 0; i < numPolys; i++) {
		memcpy(&boundingRect.m_iLeft, buffer, sizeof(long));
		memcpy(&boundingRect.m_iTop, (buffer += sizeof(long)), sizeof(long));
		memcpy(&boundingRect.m_iRight, (buffer += sizeof(long)), sizeof(long));
		memcpy(&boundingRect.m_iBottom, (buffer += sizeof(long)), sizeof(long));
		polys->second[i].first = boundingRect;
		memcpy(&numPoints, (buffer += sizeof(long)), sizeof(unsigned int));
		buffer += sizeof(unsigned int);
		polys->second[i].second.resize(numPoints);
		for (j = 0; j < numPoints; j++) {
			memcpy(&polys->second[i].second[j].m_iLong, buffer, sizeof(long));
			memcpy(&polys->second[i].second[j].m_iLat, (buffer += sizeof(long)), sizeof(long));
			buffer += sizeof(long);
		}
	}
	m_mapCountyCodeToRecords.insert(std::pair<unsigned short, std::pair<unsigned int, unsigned int> >(countyCode, std::pair<unsigned int, unsigned int>(m_nRecords, m_nRecords + numRecords)));
	iterSquares = m_mapCountyCodeToRegions.insert(std::pair<unsigned short, std::vector<std::vector<unsigned int> > >(countyCode, std::vector<std::vector<unsigned int> >(SQUARES_PER_COUNTY))).first;
	AddRecordsToRegionSquares(m_nRecords, m_nRecords + numRecords, &iterSquares->second, iterBoundary->second);
	m_nRecords += numRecords;
	qApp->processEvents();
	bSuccess = true;

TIGERPROCESSOR_LOAD_ERROR:
	munmap(startBuffer, fileInfo.st_size);
	close(handle);
	qApp->processEvents();
	if (newRecordBuffer != NULL) delete[] newRecordBuffer; // TODO: fix memory leak! (note: only occurs on error)
	return bSuccess;
}

bool MapDB::FindAddress(Address * pAddress, int iSearchNumber, const QString & strSearchStreet, const QString & strSearchType, const QString & strCity, const QString & strState)
{
	unsigned int iStreetName = (unsigned)-1, iStreetType = (unsigned)-1, i, iRec, j;
	std::map<QString, unsigned int>::iterator iterStringID;
	std::vector<unsigned int> vecZips;
	std::set<unsigned int> setZips;
	QString strSearchTypeAbbrev;
	MapRecord * pRec;
	bool bMatch, bOnLeft, bOnRight;
	double fPercentageAlongLeg, fTotalDistance, fDesiredDistance, fSearchDistance, fChange;
	int iAddressZipEntry;

	if (!strSearchStreet.isEmpty() && (iterStringID = m_mapStringsToIndex.find(strSearchStreet)) != m_mapStringsToIndex.end())
		iStreetName = iterStringID->second;

	strSearchTypeAbbrev = GetAbbreviation(strSearchType);
	if (!strSearchTypeAbbrev.isEmpty() && (iterStringID = m_mapStringsToIndex.find(strSearchTypeAbbrev)) != m_mapStringsToIndex.end())
		iStreetType = iterStringID->second;
	else
	{
		strSearchTypeAbbrev = strSearchType;
		if (!strSearchType.isEmpty() && (iterStringID = m_mapStringsToIndex.find(strSearchType)) != m_mapStringsToIndex.end())
			iStreetType = iterStringID->second;
	}

	if (!strCity.isEmpty() && !strState.isEmpty())
		vecZips = ZipCodesByCityState(strCity, strState);
	for (i = 0; i < vecZips.size(); i++)
		setZips.insert(vecZips[i]);

	if (iStreetName == (unsigned)-1 || m_vecStringRoads[iStreetName].empty())
		return false;

	for (i = 0; i < m_vecStringRoads[iStreetName].size(); i++)
	{
		iRec = m_vecStringRoads[iStreetName][i];
		pRec = m_pRecords + iRec;
		if (pRec != NULL)
		{
			// verify street name and type matches
			bMatch = false;
			for (j = 0; j < pRec->nFeatureNames; j++)
			{
				if (iStreetType == (unsigned)-1 || (pRec->pFeatureNames[j] == iStreetName && pRec->pFeatureTypes[j] == iStreetType))
				{
					bMatch = true;
					break;
				}
			}
			if (!bMatch)
				continue;

			// verify address and zip code matches
			bOnLeft = bOnRight = false;
			fPercentageAlongLeg = 0.;
			iAddressZipEntry = -1;
			if (iSearchNumber || (!strCity.isEmpty() && !strState.isEmpty()))
			{
				for (j = 0; j < pRec->nAddressRanges; j++)
				{
					int iLow = pRec->pAddressRanges[j].iFromAddr;
					int iHigh = pRec->pAddressRanges[j].iToAddr;
					bool bLeft = pRec->pAddressRanges[j].bOnLeft;
					bool bSwapped = false;

					if (iLow > iHigh)
					{
						int iTmp;
						iTmp = iLow;
						iLow = iHigh;
						iHigh = iTmp;
						bSwapped = true;
					}

					if ((iLow <= iSearchNumber && iSearchNumber <= iHigh && ((iHigh & 0x1) != (iLow & 0x1) || (iLow & 0x1) == (iSearchNumber & 0x1))) || !iSearchNumber)
					{
						if (bLeft)
							bOnLeft = true;
						else
							bOnRight = true;
	
						fPercentageAlongLeg = (double) (iSearchNumber - iLow) / (iHigh - iLow + 1);
	
						if (bSwapped)
							fPercentageAlongLeg = 1 - fPercentageAlongLeg;
	
						if (!iSearchNumber)
							fPercentageAlongLeg = 0;
					}
					if ((bOnLeft || bOnRight) && (!strCity.isEmpty() || !strState.isEmpty()))
					{
						// verify zip code matches
						if (setZips.find(pRec->pAddressRanges[j].iZip) == setZips.end())
						{
							bOnLeft = false;
							bOnRight = false;
						}
					}
					if (bOnLeft || bOnRight) {
						iAddressZipEntry = j;
						break;
					}
				}
			}
			else // we're not searching by street number, city, or zip
				bOnLeft = true;
			bMatch = bOnLeft || bOnRight;
			if (!bMatch)
				continue;

			fTotalDistance = RecordDistance(pRec);
			fDesiredDistance = fTotalDistance * fPercentageAlongLeg;
			fSearchDistance = 0.;
			for (j = 0; (signed)j < pRec->nShapePoints - 1; j++)
			{
				fChange = Distance(pRec->pShapePoints[j], pRec->pShapePoints[j + 1]);
				if (fSearchDistance <= fDesiredDistance && fDesiredDistance < fSearchDistance + fChange)
				{
					int iZip = iAddressZipEntry < 0 ? 0 : pRec->pAddressRanges[iAddressZipEntry].iZip;
					pAddress->iVertex = (unsigned)-1;
					pAddress->ptCoordinates = pRec->pShapePoints[j+1] * (fDesiredDistance - fSearchDistance) / fChange + pRec->pShapePoints[j] * (1 - (fDesiredDistance - fSearchDistance) / fChange);

					pAddress->iStreetNumber = iSearchNumber;
					pAddress->szStreetName = m_vecStrings[pRec->pFeatureNames[0]];
					pAddress->szStreetType = m_vecStrings[pRec->pFeatureTypes[0]];

					pAddress->szCityName = CityByZip(iZip);
					pAddress->szStateName = StateByZip(iZip);
					pAddress->iZipCode = iZip;

					pAddress->iRecord = iRec;

					return true;
				}

				fSearchDistance += fChange;
			}
		}
	}
	return false;
/*
	unsigned int iRec, i;
	std::vector<unsigned int> vecZips;
	QString strSearchTypeAbbreviated;
	unsigned int iStreetName = (unsigned)-1, iStreetType = (unsigned)-1;
	std::map<QString, unsigned int>::iterator iterStringID;
	std::list<RecordRange> listRecordRanges;
	std::list<RecordRange>::iterator iterRecordRange;
	std::map<unsigned short, RecordRange>::iterator iterCountyRecordRange;
	std::set<unsigned short> setCounties;
	std::set<unsigned short>::iterator iterCountyCode;

	if (!strSearchStreet.isEmpty() && (iterStringID = m_mapStringsToIndex.find(strSearchStreet)) != m_mapStringsToIndex.end())
		iStreetName = iterStringID->second;

	strSearchTypeAbbreviated = GetAbbreviation(strSearchType);
	if (strSearchTypeAbbreviated.isEmpty())
		strSearchTypeAbbreviated = strSearchType;

	if (!strSearchTypeAbbreviated.isEmpty() && (iterStringID = m_mapStringsToIndex.find(strSearchTypeAbbreviated)) != m_mapStringsToIndex.end())
		iStreetType = iterStringID->second;

	if (!strCity.isEmpty() && !strState.isEmpty())
		vecZips = ZipCodesByCityState(strCity, strState);

	// get list of record ranges to search
	for (i = 0; i < vecZips.size(); i++)
		setCounties.insert(CountyCodeByZip(vecZips[i]));
	for (iterCountyCode = setCounties.begin(); iterCountyCode != setCounties.end(); ++iterCountyCode)
	{
		iterCountyRecordRange = m_mapCountyCodeToRecords.find(*iterCountyCode);
		if (iterCountyRecordRange != m_mapCountyCodeToRecords.end())
			listRecordRanges.push_back(iterCountyRecordRange->second);
	}
	if (listRecordRanges.empty())
		listRecordRanges.push_back(RecordRange(0, m_nRecords));

	for (iterRecordRange = listRecordRanges.begin(); iterRecordRange != listRecordRanges.end(); ++iterRecordRange)
	{
		for (iRec = iterRecordRange->first; iRec < iterRecordRange->second; iRec++)
		{
			MapRecord * pRec = m_pRecords + iRec;
	
			bool bMatch = true;
			bool bOnLeft = false;
			bool bOnRight = false;
			double fPercentageAlongLeg = 0;
			int iAddressZipEntry = -1;
	
			if (iSearchNumber || (!strCity.isEmpty() && !strState.isEmpty()))
			{
				int iRange;
				for (iRange = 0; iRange < pRec->nAddressRanges && !bOnLeft && !bOnRight; iRange++)
				{
					int iLow = pRec->pAddressRanges[iRange].iFromAddr;
					int iHigh = pRec->pAddressRanges[iRange].iToAddr;
					bool bLeft = pRec->pAddressRanges[iRange].bOnLeft;
					int iSwapped = 0;
	
					if (iLow > iHigh)
					{
						int iTmp;
						iTmp = iLow;
						iLow = iHigh;
						iHigh = iTmp;
						iSwapped = 1;
					}
	
					if ((iLow <= iSearchNumber && iSearchNumber <= iHigh && ((iHigh & 0x1) != (iLow & 0x1) || (iLow & 0x1) == (iSearchNumber & 0x1))) || !iSearchNumber)
					{
						if (bLeft)
							bOnLeft = true;
						else
							bOnRight = true;
	
						fPercentageAlongLeg = (double) (iSearchNumber - iLow) / (iHigh - iLow + 1);
	
						if (iSwapped)
							fPercentageAlongLeg = 1 - fPercentageAlongLeg;
	
						if (!iSearchNumber)
							fPercentageAlongLeg = 0;
					}
	
	
					if ((bOnLeft || bOnRight) && (!strCity.isEmpty() || !strState.isEmpty()))
					{
						bool bZipOkay = false;
						unsigned int i;
	
						for(i = 0; i < vecZips.size(); i++)
						{
							if (pRec->pAddressRanges[iRange].iZip == vecZips[i])
								bZipOkay = true;
						}
	
						if (!bZipOkay)
						{
							bOnLeft = false;
							bOnRight = false;
						}
					}
					if (bOnLeft || bOnRight)
						iAddressZipEntry = iRange;
				}
			}
			else // we're not searching by street number, city, or zip
			{
				bOnLeft = true;
			}
	
			if (!bOnLeft && !bOnRight)
				bMatch = false;
	
			if (bMatch)
			{
				bool bMatchesName = false;
				int i;

				for (i = 0; i < pRec->nFeatureNames; i++)
				{
					if ((strSearchStreet.isEmpty() || pRec->pFeatureNames[i] == iStreetName) && (strSearchTypeAbbreviated.isEmpty() || pRec->pFeatureTypes[i] == iStreetType))
						bMatchesName = true;
				}
	
				if (!bMatchesName)
					bMatch = false;
			}

			if (bMatch)
			{
				double fTotalDistance = 0;
				{
					int iSeg;
					for (iSeg = 0; iSeg < pRec->nShapePoints - 1; iSeg ++)
					{
						fTotalDistance += Distance(pRec->pShapePoints[iSeg], pRec->pShapePoints[iSeg + 1]);
					}
				}
	
				double fDesiredDistance = fTotalDistance * fPercentageAlongLeg;
				double fSearchDistance = 0;
				int iSeg;
				for (iSeg = 0; iSeg < pRec->nShapePoints - 1; iSeg ++)
				{
					double fChange = Distance(pRec->pShapePoints[iSeg], pRec->pShapePoints[iSeg + 1]);
	
					if (fSearchDistance <= fDesiredDistance && fDesiredDistance < fSearchDistance + fChange)
					{
						int iZip = 0;
	
						if (iAddressZipEntry >= 0)
							iZip = pRec->pAddressRanges[iAddressZipEntry].iZip;
	
						pAddress->iVertex = (unsigned)-1;
						pAddress->ptCoordinates.m_iLong = (long)((pRec->pShapePoints[iSeg + 1].m_iLong - pRec->pShapePoints[iSeg].m_iLong) * (fDesiredDistance - fSearchDistance) / fChange + pRec->pShapePoints[iSeg].m_iLong);
						pAddress->ptCoordinates.m_iLat = (long)((pRec->pShapePoints[iSeg + 1].m_iLat - pRec->pShapePoints[iSeg].m_iLat) * (fDesiredDistance - fSearchDistance) / fChange + pRec->pShapePoints[iSeg].m_iLat);
	
						pAddress->iStreetNumber = iSearchNumber;
						pAddress->szStreetName = m_vecStrings[pRec->pFeatureNames[0]];
						pAddress->szStreetType = m_vecStrings[pRec->pFeatureTypes[0]];
	
						pAddress->szCityName = CityByZip(iZip);
						pAddress->szStateName = StateByZip(iZip);
						pAddress->iZipCode = iZip;
	
						pAddress->iRecord = iRec;
	
						return true;
					}
	
					fSearchDistance += fChange;
				}
			}
		}
	}
	return false;
*/
}

bool MapDB::FindCoordinates(Address * pAddress, const Coords & ptSearch)
{
	unsigned int iRecord = (unsigned)-1;
	unsigned short iShapePoint = (unsigned)-1;
	float fProgress = 0.f;
	if (CoordsToRecord(ptSearch, iRecord, iShapePoint, fProgress, HUGE_VAL))
	{
		return AddressFromRecord(pAddress, iRecord, iShapePoint, fProgress);
	}
	else
		return false;
}

bool MapDB::GetRandomAddress(Address * pAddress, const Rect & rRect)
{
	std::vector<unsigned int> vecRecords;
	unsigned int iRecord = (unsigned)-1, iShapePoint = (unsigned)-1;
	float fProgress = 0.f;
	MapRecord * pRecord;

	if (GetRecordsInRegion(vecRecords, rRect))
	{
		iRecord = vecRecords[RandUInt(0, vecRecords.size())];
		pRecord = GetRecord(iRecord);
		if (pRecord->nShapePoints > 1)
		{
			iShapePoint = RandUInt(0, pRecord->nShapePoints-1);
			fProgress = RandDouble(0., 1.);
			return AddressFromRecord(pAddress, iRecord, iShapePoint, fProgress);
		}
	}
	return false;
}

bool MapDB::GetRecordsInRegion(std::vector<unsigned int> & vecRecords, const Rect & rRect)
{
	std::map<unsigned short, CountySquares>::iterator iterCountySquares;
	std::map<unsigned short, Rect>::iterator iterCountyRect;
	unsigned int x1, x2, y1, y2, i, j, k;
	MapRecord * pRecord;

	// construct a list of records in the rectangle
	for (iterCountySquares = m_mapCountyCodeToRegions.begin(); iterCountySquares != m_mapCountyCodeToRegions.end(); ++iterCountySquares)
	{
		// first, see if we are really in this county
		iterCountyRect = m_mapCountyCodeToBoundingRect.find(iterCountySquares->first);
		if (iterCountyRect == m_mapCountyCodeToBoundingRect.end() || !iterCountyRect->second.intersectRect(rRect))
			continue;

		x1 = (unsigned int)ValidateNumber(floor((SQUARES_PER_COUNTY_SIDE * (rRect.m_iLeft - iterCountyRect->second.m_iLeft)) / (iterCountyRect->second.m_iRight - iterCountyRect->second.m_iLeft)), 0, SQUARES_PER_COUNTY_SIDE - 1);
		y1 = (unsigned int)ValidateNumber(floor((SQUARES_PER_COUNTY_SIDE * (iterCountyRect->second.m_iTop - rRect.m_iTop)) / (iterCountyRect->second.m_iTop - iterCountyRect->second.m_iBottom)), 0, SQUARES_PER_COUNTY_SIDE - 1);
		x2 = (unsigned int)ValidateNumber(ceil((SQUARES_PER_COUNTY_SIDE * (rRect.m_iRight - iterCountyRect->second.m_iLeft)) / (iterCountyRect->second.m_iRight - iterCountyRect->second.m_iLeft)), 0, SQUARES_PER_COUNTY_SIDE - 1);
		y2 = (unsigned int)ValidateNumber(ceil((SQUARES_PER_COUNTY_SIDE * (iterCountyRect->second.m_iTop - rRect.m_iBottom)) / (iterCountyRect->second.m_iTop - iterCountyRect->second.m_iBottom)), 0, SQUARES_PER_COUNTY_SIDE - 1);

		for (i = y1; i <= y2; i++)
		{
			for (j = x1; j <= x2; j++)
			{
				CountySquare & regionRecords = iterCountySquares->second[(int)(j+i*SQUARES_PER_COUNTY_SIDE)];
				for (k = 0; k < regionRecords.size(); k++)
				{
					pRecord = GetRecord(regionRecords[k]);
					if (IsRoad(pRecord) && pRecord->nShapePoints >= 2 && pRecord->nFeatureNames > 0 && !m_vecStrings[pRecord->pFeatureNames[0]].isEmpty() && !m_vecStrings[pRecord->pFeatureTypes[0]].isEmpty() && pRecord->nAddressRanges > 0 && rRect.intersectRect(pRecord->rBounds) && IsRecordVisible(pRecord->pShapePoints, pRecord->nShapePoints, Coords(rRect.m_iLeft, rRect.m_iTop), Coords(rRect.m_iRight, rRect.m_iBottom)))
						vecRecords.push_back(regionRecords[k]);
				}
			}
		}
	}
	return !vecRecords.empty();
}

bool MapDB::AddressFromRecord(Address * pAddress, unsigned int iRecord, unsigned int iShapePoint, float fProgress)
{
	MapRecord * pRecord = GetRecord(iRecord);
	float fRecordDistance = RecordDistance(pRecord);
	float fFractionDistance = 0.;
	int i, iPoint = (signed)iShapePoint, iStart, iEnd, iStep;

	pAddress->iRecord = iRecord;
	pAddress->iVertex = (unsigned)-1;

	if (pRecord->nFeatureNames > 0)
	{
		pAddress->szStreetName = m_vecStrings[pRecord->pFeatureNames[0]];
		pAddress->szStreetType = m_vecStrings[pRecord->pFeatureTypes[0]];
	}
	else
		pAddress->szStreetName = pAddress->szStreetType = QString::null;

	if (pRecord->nAddressRanges > 0)
	{
		pAddress->iZipCode = pRecord->pAddressRanges[0].iZip;
		pAddress->szCityName = CityByZip(pAddress->iZipCode);
		pAddress->szStateName = StateByZip(pAddress->iZipCode);

		// find street number
		if (iPoint >= pRecord->nShapePoints - 1)
		{
			iPoint = pRecord->nShapePoints - 2;
			fProgress = 1.f;
		}
		for (i = 0; i < iPoint; i++)
			fFractionDistance += Distance(pRecord->pShapePoints[i], pRecord->pShapePoints[i+1]);
		if (iPoint >= 0) {
			fFractionDistance += fProgress * Distance(pRecord->pShapePoints[iPoint], pRecord->pShapePoints[iPoint+1]);
			pAddress->ptCoordinates = pRecord->pShapePoints[iPoint] * (1 - fProgress) + pRecord->pShapePoints[iPoint+1] * fProgress;
			fProgress = fFractionDistance / fRecordDistance;
			iStart = pRecord->pAddressRanges[0].iFromAddr;
			iEnd = pRecord->pAddressRanges[0].iToAddr;
			iStep = (iStart % 2 == iEnd % 2) ? 2 : 1;
			pAddress->iStreetNumber = iStart + (int)round(((iEnd - iStart) / iStep) * fProgress) * iStep;
		}
		else
		{
			pAddress->iStreetNumber = 0;
			pAddress->ptCoordinates = pRecord->pShapePoints[0];
		}
	}
	else
	{
		pAddress->iZipCode = (unsigned)-1;
		pAddress->szCityName = pAddress->szStateName = QString::null;
		pAddress->iStreetNumber = 0;
		pAddress->ptCoordinates = pRecord->pShapePoints[0];
	}
	return true;
}

std::list<unsigned int> MapDB::ShortestPath(Address * pStart, Address * pEnd, bool & bBackwardsStart, bool & bBackwardsEnd)
{
	unsigned int nVertices = m_vecVertices.size(), startVertex1, startVertex2, endVertex1, endVertex2;
	double startDistance = 0.0, endDistance = 0.0, newDistance, totalDistance1 = INFINITY, totalDistance2 = INFINITY;
	bool bNeedEnd2 = false, bFoundEnd1 = false, bFoundEnd2, bBackwards1, bBackwards2, bBackwardsEnd1, bBackwardsEnd2;
	MapRecord * pStartRec = NULL, * pEndRec = NULL;
	AddressRange * pAddrZip, * pEndAddrZip;
	std::list<unsigned int> rPath1, rPath2, rPath;
	FibonacciHeap<double, DijkstraVertex> heap;
	FibonacciHeapNode<double, DijkstraVertex> * minNode;
	std::map<unsigned int, unsigned int>::iterator iterAdj;
	std::list<FibonacciHeapNode<double, DijkstraVertex> *> visitedNodes;
	std::list<FibonacciHeapNode<double, DijkstraVertex> *>::iterator iterVisitedNode;
	DijkstraVertex v;

	// initialize heap
	v.predecessor = (unsigned)-1;
	v.record = (unsigned)-1;
	for (v.vertex = 0; v.vertex < nVertices; v.vertex++)
		m_vecVerticesHeapLookup[v.vertex] = heap.InsertNode(INFINITY, v);

	// use starting address to start algorithm
	startVertex1 = startVertex2 = pStart->iVertex;
	if (pStart->iVertex < m_vecVerticesHeapLookup.size()) { // address is a vertex
		heap.DecreaseKey(m_vecVerticesHeapLookup[startVertex1], 0.0);
	} else {
		pStartRec = m_pRecords + pStart->iRecord;
		startVertex1 = pStartRec->pVertices[0];
		startVertex2 = pStartRec->pVertices[pStartRec->nVertices-1];
		// find distance along address range (of course, we first have to find the correct address range)
		pEndAddrZip = pStartRec->pAddressRanges + pStartRec->nAddressRanges;
		for (pAddrZip = pStartRec->pAddressRanges; pAddrZip < pEndAddrZip; pAddrZip++) {
			if (pAddrZip->iZip != pStart->iZipCode) continue;
			if (pAddrZip->iFromAddr <= pAddrZip->iToAddr && pAddrZip->iFromAddr <= pStart->iStreetNumber && pAddrZip->iToAddr >= pStart->iStreetNumber && (pAddrZip->iFromAddr % 2 != pAddrZip->iToAddr % 2 || pAddrZip->iFromAddr % 2 == pStart->iStreetNumber % 2)) {
				startDistance = pStartRec->fCost * ((double)pStart->iStreetNumber - pAddrZip->iFromAddr) / (pAddrZip->iToAddr - pAddrZip->iFromAddr + 1);
				break;
			} else if (pAddrZip->iFromAddr >= pAddrZip->iToAddr && pAddrZip->iFromAddr >= pStart->iStreetNumber && pAddrZip->iToAddr <= pStart->iStreetNumber && (pAddrZip->iFromAddr % 2 != pAddrZip->iToAddr % 2 || pAddrZip->iFromAddr % 2 == pStart->iStreetNumber % 2)) {
				startDistance = pStartRec->fCost * ((double)pAddrZip->iFromAddr - pStart->iStreetNumber) / (pAddrZip->iFromAddr - pAddrZip->iToAddr + 1);
				break;
			}
		}
		if (startDistance == 0.0 || !IsOneWay(pStartRec)) heap.DecreaseKey(m_vecVerticesHeapLookup[startVertex1], startDistance);
		heap.DecreaseKey(m_vecVerticesHeapLookup[startVertex2], pStartRec->fCost - startDistance);
	}

	// use ending address to find termination point(s) for algorithm
	endVertex1 = endVertex2 = pEnd->iVertex;
	if (pEnd->iVertex < m_vecVerticesHeapLookup.size()) { // address is a vertex
		if (pEnd->iVertex == pStart->iVertex) { // same vertex - don't go anywhere!
			return rPath;
		}
	} else {
		pEndRec = m_pRecords + pEnd->iRecord;
		endVertex1 = pEndRec->pVertices[0];
		endVertex2 = pEndRec->pVertices[pEndRec->nVertices-1];
		// find distance along address range
		pEndAddrZip = pEndRec->pAddressRanges + pEndRec->nAddressRanges;
		for (pAddrZip = pEndRec->pAddressRanges; pAddrZip < pEndAddrZip; pAddrZip++) {
			if (pAddrZip->iZip != pEnd->iZipCode) continue;
			if (pAddrZip->iFromAddr <= pAddrZip->iToAddr && pAddrZip->iFromAddr <= pEnd->iStreetNumber && pAddrZip->iToAddr >= pEnd->iStreetNumber && (pAddrZip->iFromAddr % 2 != pAddrZip->iToAddr % 2 || pAddrZip->iFromAddr % 2 == pEnd->iStreetNumber % 2)) {
				endDistance = pEndRec->fCost * ((double)pEnd->iStreetNumber - pAddrZip->iFromAddr) / (pAddrZip->iToAddr - pAddrZip->iFromAddr + 1);
				break;
			} else if (pAddrZip->iFromAddr >= pAddrZip->iToAddr && pAddrZip->iFromAddr >= pEnd->iStreetNumber && pAddrZip->iToAddr <= pEnd->iStreetNumber && (pAddrZip->iFromAddr % 2 != pAddrZip->iToAddr % 2 || pAddrZip->iFromAddr % 2 == pEnd->iStreetNumber % 2)) {
				endDistance = pEndRec->fCost * ((double)pAddrZip->iFromAddr - pEnd->iStreetNumber) / (pAddrZip->iFromAddr - pAddrZip->iToAddr + 1);
				break;
			}
		}
		bNeedEnd2 = !IsOneWay(pEndRec);
	}

	bFoundEnd2 = !bNeedEnd2; // don't bother looking for the second end point if unnecessary

	while (!heap.IsEmpty()) {
		minNode = heap.ExtractMin();
		visitedNodes.push_back(minNode);
		if (minNode->getKey() == INFINITY) break; // the heap might as well be empty
		if (minNode->m_Data.vertex == endVertex1) bFoundEnd1 = true;
		if (minNode->m_Data.vertex == endVertex2) bFoundEnd2 = true;
		if (bFoundEnd1 && bFoundEnd2) break; // we've found our route(s) - stop searching
		// update adjacent vertices
		for (iterAdj = m_vecVertices[minNode->m_Data.vertex].mapEdges.begin(); iterAdj != m_vecVertices[minNode->m_Data.vertex].mapEdges.end(); ++iterAdj) {
			newDistance = m_pRecords[iterAdj->first].fCost + minNode->getKey();
			if (newDistance < m_vecVerticesHeapLookup[iterAdj->second]->getKey()) { // key will decrease - update
				heap.DecreaseKey(m_vecVerticesHeapLookup[iterAdj->second], newDistance);
				m_vecVerticesHeapLookup[iterAdj->second]->m_Data.predecessor = minNode->m_Data.vertex; // the vertex previous to this one
				m_vecVerticesHeapLookup[iterAdj->second]->m_Data.record = iterAdj->first; // record to take from previous vertex to this one
			}
		}
	}

	if (bFoundEnd1 || (bFoundEnd2 && bNeedEnd2)) { // we found a route
		// iterate through paths to both endpoints (each endpoint as necessary)
		bBackwards1 = bBackwards2 = bBackwardsEnd1 = bBackwardsEnd2 = false;
		if (bFoundEnd1) {
			minNode = m_vecVerticesHeapLookup[endVertex1];
			if (pEndRec != NULL) {
				rPath1.push_back(pEndRec - m_pRecords);
				totalDistance1 = endDistance;
			} else
				totalDistance1 = 0.0;
			while (true) {
				if (minNode->m_Data.record == (unsigned)-1) {
					totalDistance1 += minNode->getKey();
					break; // got to start vertex - don't go any farther
				} else {
					rPath1.push_back(minNode->m_Data.record);
					totalDistance1 += m_pRecords[minNode->m_Data.record].fCost;
				}
				minNode = m_vecVerticesHeapLookup[minNode->m_Data.predecessor];
			}
			if (pStartRec != NULL) // this is for a starting address
				rPath1.push_back(pStartRec - m_pRecords);
			if (rPath1.empty()) { // short circuited route - this should NEVER happen...
				totalDistance1 = INFINITY;
			} else {
				if (pStartRec != NULL) // if we start at an address, the first vertex is where we go to
					bBackwards1 = m_pRecords[rPath1.back()].pVertices[0] == minNode->m_Data.vertex;
				else // otherwise, we start on a vertex, and the first vertex is where we leave from
					bBackwards1 = m_pRecords[rPath1.back()].pVertices[m_pRecords[rPath1.back()].nVertices-1] == minNode->m_Data.vertex;
				if (pEndRec != NULL) // if we end at an address, the last vertex (endVertex1) is at the beginning of the last record
					bBackwardsEnd1 = false;
				else // otherwise, the last vertex (endVertex1) is where we stop, and we need to compare this to the last record
					bBackwardsEnd1 = m_pRecords[rPath1.front()].pVertices[0] == endVertex1;
			}
		}
		if (bFoundEnd2 && bNeedEnd2) {
			minNode = m_vecVerticesHeapLookup[endVertex2];
			rPath2.push_back(pEndRec - m_pRecords);
			totalDistance2 = pEndRec->fCost - endDistance;
			while (true) {
				if (minNode->m_Data.record == (unsigned)-1) {
					totalDistance2 += minNode->getKey();
					break;
				} else {
					rPath2.push_back(minNode->m_Data.record);
					totalDistance2 += m_pRecords[minNode->m_Data.record].fCost;
				}
				minNode = m_vecVerticesHeapLookup[minNode->m_Data.predecessor];
			}
			if (pStartRec != NULL)
				rPath2.push_back(pStartRec - m_pRecords);
			if (rPath2.empty()) { // short circuited route - this should NEVER happen...
				totalDistance2 = INFINITY;
			} else {
				if (pStartRec != NULL) // if we start at an address, the first vertex is where we go to
					bBackwards2 = m_pRecords[rPath2.back()].pVertices[0] == minNode->m_Data.vertex;
				else // otherwise, we start on a vertex, and the first vertex is where we leave from
					bBackwards2 = m_pRecords[rPath2.back()].pVertices[m_pRecords[rPath2.back()].nVertices-1] == minNode->m_Data.vertex;
				bBackwardsEnd2 = true;
			}
		}
		// see which is better
		if (totalDistance1 <= totalDistance2) { // take path to end point through end vertex 1
			// make sure we aren't on the same record and should "short circuit" the route
			if (pStartRec != NULL && pStartRec == pEndRec) {
				if (startDistance <= endDistance && endDistance - startDistance <= totalDistance1) { // we should just go forwards to the end address
					rPath.push_back(pStartRec - m_pRecords);
					bBackwardsStart = bBackwardsEnd = false;
				} else if (startDistance > endDistance && !IsOneWay(pStartRec) && startDistance - endDistance <= totalDistance1) { // we should just go backwards to the end address
					rPath.push_back(pStartRec - m_pRecords);
					bBackwardsStart = bBackwardsEnd = true;
				} else { // stick with the generated route
					rPath = rPath1;
					bBackwardsStart = bBackwards1;
					bBackwardsEnd = bBackwardsEnd1;
				}
			} else {
				rPath = rPath1;
				bBackwardsStart = bBackwards1;
				bBackwardsEnd = bBackwardsEnd1;
			}
		} else {
			// make sure we aren't on the same record and should "short circuit" the route
			if (pStartRec != NULL && pStartRec == pEndRec) {
				if (startDistance <= endDistance && endDistance - startDistance <= totalDistance2) { // we should just go forwards to the end address
					rPath.push_back(pStartRec - m_pRecords);
					bBackwardsStart = bBackwardsEnd = false;
				} else if (startDistance > endDistance && !IsOneWay(pStartRec) && startDistance - endDistance <= totalDistance2) { // we should just go backwards to the end address
					rPath.push_back(pStartRec - m_pRecords);
					bBackwardsStart = bBackwardsEnd = true;
				} else { // stick with the generated route
					rPath = rPath2;
					bBackwardsStart = bBackwards2;
					bBackwardsEnd = bBackwardsEnd2;
				}
			} else {
				rPath = rPath2;
				bBackwardsStart = bBackwards2;
				bBackwardsEnd = bBackwardsEnd2;
			}
		}
	}

	for (iterVisitedNode = visitedNodes.begin(); iterVisitedNode != visitedNodes.end(); ++iterVisitedNode)
		delete *iterVisitedNode;

	return rPath;
}

std::list<unsigned int> MapDB::ShortestPath(unsigned int iStartVertex, unsigned int iStartRecord, unsigned short iStartShapePoint, float fStartProgress, unsigned int iEndVertex, unsigned int iEndRecord, unsigned short iEndShapePoint, float fEndProgress, bool & bBackwardsStart, bool & bBackwardsEnd)
{
	unsigned int nVertices = m_vecVertices.size(), startVertex1, startVertex2, endVertex1, endVertex2;
	double startDistance = 0.0, endDistance = 0.0, newDistance, totalDistance1 = INFINITY, totalDistance2 = INFINITY;
	bool bNeedEnd2 = false, bFoundEnd1 = false, bFoundEnd2, bBackwards1, bBackwards2, bBackwardsEnd1, bBackwardsEnd2;
	MapRecord * pStartRec = NULL, * pEndRec = NULL;
	std::list<unsigned int> rPath1, rPath2, rPath;
	FibonacciHeap<double, DijkstraVertex> heap;
	FibonacciHeapNode<double, DijkstraVertex> * minNode;
	std::map<unsigned int, unsigned int>::iterator iterAdj;
	std::list<FibonacciHeapNode<double, DijkstraVertex> *> visitedNodes;
	std::list<FibonacciHeapNode<double, DijkstraVertex> *>::iterator iterVisitedNode;
	DijkstraVertex v;
	unsigned short i;

	// initialize heap
	v.predecessor = (unsigned)-1;
	v.record = (unsigned)-1;
	for (v.vertex = 0; v.vertex < nVertices; v.vertex++)
		m_vecVerticesHeapLookup[v.vertex] = heap.InsertNode(INFINITY, v);

	// use starting address to start algorithm
	startVertex1 = startVertex2 = iStartVertex;
	if (iStartVertex < m_vecVerticesHeapLookup.size()) { // address is a vertex
		heap.DecreaseKey(m_vecVerticesHeapLookup[startVertex1], 0.0);
	} else {
		pStartRec = m_pRecords + iStartRecord;
		startVertex1 = pStartRec->pVertices[0];
		startVertex2 = pStartRec->pVertices[pStartRec->nVertices-1];

		double fTotalDistance = RecordDistance(pStartRec), fFracDistance = 0.;
		if (iStartShapePoint == pStartRec->nShapePoints - 1)
		{
			iStartShapePoint--;
			fStartProgress = 1.f;
		}
		for (i = 0; i < iStartShapePoint; i++)
			fFracDistance += Distance(pStartRec->pShapePoints[i], pStartRec->pShapePoints[i+1]);
		fFracDistance += Distance(pStartRec->pShapePoints[iStartShapePoint], pStartRec->pShapePoints[iStartShapePoint+1]) * fStartProgress;
		startDistance = pStartRec->fCost * (fFracDistance / fTotalDistance);

		if (startDistance == 0.0 || !IsOneWay(pStartRec))
			heap.DecreaseKey(m_vecVerticesHeapLookup[startVertex1], startDistance);
		heap.DecreaseKey(m_vecVerticesHeapLookup[startVertex2], pStartRec->fCost - startDistance);
	}

	// use ending address to find termination point(s) for algorithm
	endVertex1 = endVertex2 = iEndVertex;
	if (iEndVertex < m_vecVerticesHeapLookup.size()) { // address is a vertex
		if (iEndVertex == iStartVertex) // same vertex - don't go anywhere!
			return rPath;
	} else {
		pEndRec = m_pRecords + iEndRecord;
		endVertex1 = pEndRec->pVertices[0];
		endVertex2 = pEndRec->pVertices[pEndRec->nVertices-1];

		double fTotalDistance = RecordDistance(pEndRec), fFracDistance = 0.;
		if (iEndShapePoint == pEndRec->nShapePoints - 1)
		{
			iEndShapePoint--;
			fEndProgress = 1.f;
		}
		for (i = 0; i < iEndShapePoint; i++)
			fFracDistance += Distance(pEndRec->pShapePoints[i], pEndRec->pShapePoints[i+1]);
		fFracDistance += Distance(pEndRec->pShapePoints[iEndShapePoint], pEndRec->pShapePoints[iEndShapePoint+1]) * fEndProgress;
		endDistance = pEndRec->fCost * (fFracDistance / fTotalDistance);
		bNeedEnd2 = !IsOneWay(pEndRec);
	}

	bFoundEnd2 = !bNeedEnd2; // don't bother looking for the second end point if unnecessary

	while (!heap.IsEmpty()) {
		minNode = heap.ExtractMin();
		visitedNodes.push_back(minNode);
		if (minNode->getKey() == INFINITY) break; // the heap might as well be empty
		if (minNode->m_Data.vertex == endVertex1) bFoundEnd1 = true;
		if (minNode->m_Data.vertex == endVertex2) bFoundEnd2 = true;
		if (bFoundEnd1 && bFoundEnd2) break; // we've found our route(s) - stop searching
		// update adjacent vertices
		for (iterAdj = m_vecVertices[minNode->m_Data.vertex].mapEdges.begin(); iterAdj != m_vecVertices[minNode->m_Data.vertex].mapEdges.end(); ++iterAdj) {
			newDistance = m_pRecords[iterAdj->first].fCost + minNode->getKey();
			if (newDistance < m_vecVerticesHeapLookup[iterAdj->second]->getKey()) { // key will decrease - update
				heap.DecreaseKey(m_vecVerticesHeapLookup[iterAdj->second], newDistance);
				m_vecVerticesHeapLookup[iterAdj->second]->m_Data.predecessor = minNode->m_Data.vertex; // the vertex previous to this one
				m_vecVerticesHeapLookup[iterAdj->second]->m_Data.record = iterAdj->first; // record to take from previous vertex to this one
			}
		}
	}

	if (bFoundEnd1 || (bFoundEnd2 && bNeedEnd2)) { // we found a route
		// iterate through paths to both endpoints (each endpoint as necessary)
		bBackwards1 = bBackwards2 = bBackwardsEnd1 = bBackwardsEnd2 = false;
		if (bFoundEnd1) {
			minNode = m_vecVerticesHeapLookup[endVertex1];
			if (pEndRec != NULL) {
				rPath1.push_back(pEndRec - m_pRecords);
				totalDistance1 = endDistance;
			} else
				totalDistance1 = 0.0;
			while (true) {
				if (minNode->m_Data.record == (unsigned)-1) {
					totalDistance1 += minNode->getKey();
					break; // got to start vertex - don't go any farther
				} else {
					rPath1.push_back(minNode->m_Data.record);
					totalDistance1 += m_pRecords[minNode->m_Data.record].fCost;
				}
				minNode = m_vecVerticesHeapLookup[minNode->m_Data.predecessor];
			}
			if (pStartRec != NULL) // this is for a starting address
				rPath1.push_back(pStartRec - m_pRecords);
			if (rPath1.empty()) { // short circuited route - this should NEVER happen...
				totalDistance1 = INFINITY;
			} else {
				if (pStartRec != NULL) // if we start at an address, the first vertex is where we go to
					bBackwards1 = m_pRecords[rPath1.back()].pVertices[0] == minNode->m_Data.vertex;
				else // otherwise, we start on a vertex, and the first vertex is where we leave from
					bBackwards1 = m_pRecords[rPath1.back()].pVertices[m_pRecords[rPath1.back()].nVertices-1] == minNode->m_Data.vertex;
				if (pEndRec != NULL) // if we end at an address, the last vertex (endVertex1) is at the beginning of the last record
					bBackwardsEnd1 = false;
				else // otherwise, the last vertex (endVertex1) is where we stop, and we need to compare this to the last record
					bBackwardsEnd1 = m_pRecords[rPath1.front()].pVertices[0] == endVertex1;
			}
		}
		if (bFoundEnd2 && bNeedEnd2) {
			minNode = m_vecVerticesHeapLookup[endVertex2];
			rPath2.push_back(pEndRec - m_pRecords);
			totalDistance2 = pEndRec->fCost - endDistance;
			while (true) {
				if (minNode->m_Data.record == (unsigned)-1) {
					totalDistance2 += minNode->getKey();
					break;
				} else {
					rPath2.push_back(minNode->m_Data.record);
					totalDistance2 += m_pRecords[minNode->m_Data.record].fCost;
				}
				minNode = m_vecVerticesHeapLookup[minNode->m_Data.predecessor];
			}
			if (pStartRec != NULL)
				rPath2.push_back(pStartRec - m_pRecords);
			if (rPath2.empty()) { // short circuited route - this should NEVER happen...
				totalDistance2 = INFINITY;
			} else {
				if (pStartRec != NULL) // if we start at an address, the first vertex is where we go to
					bBackwards2 = m_pRecords[rPath2.back()].pVertices[0] == minNode->m_Data.vertex;
				else // otherwise, we start on a vertex, and the first vertex is where we leave from
					bBackwards2 = m_pRecords[rPath2.back()].pVertices[m_pRecords[rPath2.back()].nVertices-1] == minNode->m_Data.vertex;
				bBackwardsEnd2 = true;
			}
		}
		// see which is better
		if (totalDistance1 <= totalDistance2) { // take path to end point through end vertex 1
			// make sure we aren't on the same record and should "short circuit" the route
			if (pStartRec != NULL && pStartRec == pEndRec) {
				if (startDistance <= endDistance && endDistance - startDistance <= totalDistance1) { // we should just go forwards to the end address
					rPath.push_back(pStartRec - m_pRecords);
					bBackwardsStart = bBackwardsEnd = false;
				} else if (startDistance > endDistance && !IsOneWay(pStartRec) && startDistance - endDistance <= totalDistance1) { // we should just go backwards to the end address
					rPath.push_back(pStartRec - m_pRecords);
					bBackwardsStart = bBackwardsEnd = true;
				} else { // stick with the generated route
					rPath = rPath1;
					bBackwardsStart = bBackwards1;
					bBackwardsEnd = bBackwardsEnd1;
				}
			} else {
				rPath = rPath1;
				bBackwardsStart = bBackwards1;
				bBackwardsEnd = bBackwardsEnd1;
			}
		} else {
			// make sure we aren't on the same record and should "short circuit" the route
			if (pStartRec != NULL && pStartRec == pEndRec) {
				if (startDistance <= endDistance && endDistance - startDistance <= totalDistance2) { // we should just go forwards to the end address
					rPath.push_back(pStartRec - m_pRecords);
					bBackwardsStart = bBackwardsEnd = false;
				} else if (startDistance > endDistance && !IsOneWay(pStartRec) && startDistance - endDistance <= totalDistance2) { // we should just go backwards to the end address
					rPath.push_back(pStartRec - m_pRecords);
					bBackwardsStart = bBackwardsEnd = true;
				} else { // stick with the generated route
					rPath = rPath2;
					bBackwardsStart = bBackwards2;
					bBackwardsEnd = bBackwardsEnd2;
				}
			} else {
				rPath = rPath2;
				bBackwardsStart = bBackwards2;
				bBackwardsEnd = bBackwardsEnd2;
			}
		}
	}

	for (iterVisitedNode = visitedNodes.begin(); iterVisitedNode != visitedNodes.end(); ++iterVisitedNode)
		delete *iterVisitedNode;

	return rPath;
}

bool MapDB::AddressToPosition(Address * pAddress, unsigned int & iRecord, unsigned int & iShapePoint, float & fProgress)
{
	MapRecord * pRecord = m_pRecords + pAddress->iRecord;

	if (pAddress->iRecord == (unsigned)-1)
		return false;

	if (pAddress->iVertex == (unsigned)-1)
	{
		// regular address
		unsigned short i;
		unsigned short iEvenOddThis = pAddress->iStreetNumber & 0x1;
		float fFraction = -1.f;
		for (i = 0; pRecord->nAddressRanges; i++)
		{
			unsigned short iEvenOddFrom = pRecord->pAddressRanges[i].iFromAddr & 0x1;
			unsigned short iEvenOddTo = pRecord->pAddressRanges[i].iToAddr & 0x1;
			unsigned short step = (iEvenOddFrom == iEvenOddTo) ? 2 : 1;

			if (iEvenOddFrom != iEvenOddTo || iEvenOddFrom == iEvenOddThis)
			{
				if (pRecord->pAddressRanges[i].iFromAddr > pRecord->pAddressRanges[i].iToAddr)
				{
					if (pAddress->iStreetNumber <= pRecord->pAddressRanges[i].iFromAddr && pAddress->iStreetNumber >= pRecord->pAddressRanges[i].iToAddr)
					{
						fFraction = ((float)pRecord->pAddressRanges[i].iFromAddr - pAddress->iStreetNumber) / (pRecord->pAddressRanges[i].iFromAddr - pRecord->pAddressRanges[i].iToAddr + step);
						break;
					}
				}
				else
				{
					if (pAddress->iStreetNumber >= pRecord->pAddressRanges[i].iFromAddr && pAddress->iStreetNumber <= pRecord->pAddressRanges[i].iToAddr)
					{
						fFraction = ((float)pAddress->iStreetNumber - pRecord->pAddressRanges[i].iFromAddr) / (pRecord->pAddressRanges[i].iToAddr - pRecord->pAddressRanges[i].iFromAddr + step);
						break;
					}
				}
			}
		}

		if (fFraction < 0.f || fFraction > 1.f)
			return false;

		iRecord = pAddress->iRecord;
		iShapePoint = pRecord->nShapePoints - 1;
		fProgress = 0.f;

		float fRecordDistance = RecordDistance(pRecord) * fFraction;

		for (i = 0; i < pRecord->nShapePoints - 1; i++)
		{
			float fDistance = Distance(pRecord->pShapePoints[i], pRecord->pShapePoints[i+1]);

			if (fDistance > fRecordDistance)
			{
				iShapePoint = i;
				fProgress = fRecordDistance / fDistance;
				break;
			}
			else
				fRecordDistance -= fDistance;
		}
	}
	else
	{
		// an intersection (at either end of the record)
		if (pRecord->nVertices < 1)
			return false;

		iRecord = pAddress->iRecord;
		if (pRecord->pVertices[0] == pAddress->iVertex)
			iShapePoint = 0;
		else
			iShapePoint = pRecord->nShapePoints - 1;
		fProgress = 0.f;
	}

	return true;
}

bool MapDB::GetNextPossibleRecords(std::vector<unsigned int> & vecRecords, unsigned int iVertex, unsigned int iPrevRecord)
{
	std::map<unsigned int, unsigned int>::iterator iterVertices;
	bool bAdded = false, bPrevFound = false;
	for (iterVertices = m_vecVertices[iVertex].mapEdges.begin(); iterVertices != m_vecVertices[iVertex].mapEdges.end(); ++iterVertices)
	{
		if (!IsRoad(GetRecord(iterVertices->first)))
			continue; // can't travel on this record

		if (iterVertices->first == iPrevRecord) // should be done at most once!
			bPrevFound = true;
		else
			vecRecords.push_back(iterVertices->first);
		bAdded = true;
	}
	if (bPrevFound)
		vecRecords.push_back(iPrevRecord);
	return bAdded;
}

bool MapDB::CoordsToRecord(const Coords & ptPosition, unsigned int & iRecordClosest, unsigned short & iShapePointClosest, float & fProgressClosest, float fDistanceClosest)
{
	float fDistance, fProgress;
	unsigned int iRecordGuess = iRecordClosest;
	unsigned short iShapePoint;
	MapRecord * pRecord;
	std::map<unsigned short, CountySquares>::iterator iterCountySquares;
	std::map<unsigned short, Rect>::iterator iterCountyRect;
	int x, y;
	unsigned int i;

	iRecordClosest = (unsigned)-1;
	if (iRecordGuess != (unsigned)-1)
	{
		std::set<unsigned int> setRecords;
		std::set<unsigned int>::iterator iterRecord;
		std::map<unsigned int, unsigned int>::iterator iterVertex;

		// we made a guess
		pRecord = GetRecord(iRecordGuess);
		// see about this and connecting roads
		for (i = 0; i < pRecord->nVertices; i++)
		{
			for (iterVertex = m_vecVertices[pRecord->pVertices[i]].mapEdges.begin(); iterVertex != m_vecVertices[pRecord->pVertices[i]].mapEdges.end(); ++iterVertex)
				setRecords.insert(iterVertex->first);
		}

		for (iterRecord = setRecords.begin(); iterRecord != setRecords.end(); ++iterRecord)
		{
			pRecord = GetRecord(*iterRecord);

			fDistance = PointRecordDistance(ptPosition, pRecord, iShapePoint, fProgress);
			if (fDistance < fDistanceClosest)
			{
				iRecordClosest = *iterRecord;
				fDistanceClosest = fDistance;
				iShapePointClosest = iShapePoint;
				fProgressClosest = fProgress;
			}
		}

		if (iRecordClosest != (unsigned)-1)
			return true;
	}

	// for each county, if we're in the county, search it
	for (iterCountySquares = m_mapCountyCodeToRegions.begin(); iterCountySquares != m_mapCountyCodeToRegions.end(); ++iterCountySquares)
	{
		// first, see if we are really in this county
		iterCountyRect = m_mapCountyCodeToBoundingRect.find(iterCountySquares->first);
		if (iterCountyRect == m_mapCountyCodeToBoundingRect.end() || !iterCountyRect->second.contains(ptPosition))
			continue;

		x = (int)((SQUARES_PER_COUNTY_SIDE * (ptPosition.m_iLong - iterCountyRect->second.m_iLeft)) / (iterCountyRect->second.m_iRight - iterCountyRect->second.m_iLeft));
		y = (int)((SQUARES_PER_COUNTY_SIDE * (iterCountyRect->second.m_iTop - ptPosition.m_iLat)) / (iterCountyRect->second.m_iTop - iterCountyRect->second.m_iBottom));

		CountySquare & regionRecords = iterCountySquares->second[(int)(x+y*SQUARES_PER_COUNTY_SIDE)];
		for (i = 0; i < regionRecords.size(); i++)
		{
			pRecord = GetRecord(regionRecords[i]);

			fDistance = PointRecordDistance(ptPosition, pRecord, iShapePoint, fProgress);
			if (fDistance < fDistanceClosest)
			{
				iRecordClosest = regionRecords[i];
				fDistanceClosest = fDistance;
				iShapePointClosest = iShapePoint;
				fProgressClosest = fProgress;
			}
		}
	}

	return iRecordClosest != (unsigned)-1;
}

bool MapDB::GetRelativeRecord(unsigned int & iRecord, unsigned short & iCountyCode)
{
	std::map<unsigned short, RecordRange>::const_iterator iterCounty;

	iCountyCode = (unsigned)-1;

	for (iterCounty = m_mapCountyCodeToRecords.begin(); iterCounty != m_mapCountyCodeToRecords.end(); ++iterCounty)
	{
		if (iRecord >= iterCounty->second.first && iRecord < iterCounty->second.second)
		{
			iCountyCode = iterCounty->first;
			iRecord -= iterCounty->second.first;
			return true;
		}
	}

	iRecord = (unsigned)-1;
	return false;
}

bool MapDB::GetAbsoluteRecord(unsigned int & iRecord, unsigned short iCountyCode)
{
	std::map<unsigned short, RecordRange>::const_iterator iterCounty;

	iterCounty = m_mapCountyCodeToRecords.find(iCountyCode);
	if (iterCounty == m_mapCountyCodeToRecords.end())
		iRecord = (unsigned)-1;
	else
	{
		iRecord += iterCounty->second.first;
		if (iRecord >= iterCounty->second.second)
			iRecord = (unsigned)-1;
	}
	return iRecord != (unsigned)-1;
}

void MapDB::AddRecordsToRegionSquares(unsigned int begin, unsigned int end, CountySquares * squares, const Rect & totalBounds)
{
	unsigned int i;
	int j, k, left, top, right, bottom, sqWidth, sqHeight, squares_sides = (int)sqrt((double)squares->size());
	sqWidth = (int)ceil(((double)(totalBounds.m_iRight - totalBounds.m_iLeft)) / squares_sides);
	sqHeight = (int)ceil(((double)(totalBounds.m_iTop - totalBounds.m_iBottom)) / squares_sides);
	MapRecord * psRec = m_pRecords + begin;
	for (i = begin; i < end; i++) {
		qApp->processEvents();
		psRec->rBounds = Rect::BoundingRect(psRec->pShapePoints, psRec->nShapePoints);
		left = (psRec->rBounds.m_iLeft - totalBounds.m_iLeft) / sqWidth;
		if (left < 0) left = 0;
		if (left > squares_sides - 1) left = squares_sides - 1;
		top = (totalBounds.m_iTop - psRec->rBounds.m_iTop) / sqHeight;
		if (top < 0) top = 0;
		if (top > squares_sides - 1) top = squares_sides - 1;
		right = (psRec->rBounds.m_iRight - totalBounds.m_iLeft) / sqWidth;
		if (right < 0) right = 0;
		if (right > squares_sides - 1) right = squares_sides - 1;
		bottom = (totalBounds.m_iTop - psRec->rBounds.m_iBottom) / sqHeight;
		if (bottom < 0) bottom = 0;
		if (bottom > squares_sides - 1) bottom = squares_sides - 1;
		for (j = left; j <= right; j++) {
			for (k = top; k <= bottom; k++)
				(*squares)[j+k*squares_sides].push_back(i);
		}
		psRec++;
	}
}

unsigned int MapDB::AddString(const QString & str)
{
	std::map<QString, unsigned int>::iterator iterName = m_mapStringsToIndex.find(str);
	if (iterName != m_mapStringsToIndex.end()) return iterName->second;

	unsigned int ret = m_vecStrings.size();
	m_mapStringsToIndex.insert(std::pair<QString, unsigned int>(str, ret));
	m_vecStrings.push_back(str);
	return ret;
}

typedef struct LabelInfoStruct
{
	Coords ptStart;
	Coords ptEnd;
	float fDistance;
	DetailSettings * pLevelDetails;
} LabelInfo;

void MapDB::DrawMapFeatures(MapDrawingSettings * pSettings)
{
	std::list<CountySquare *> recordSquares;
	std::map<QString, LabelInfo> mapLabels;
	std::map<QString, LabelInfo>::iterator itLabels, modLabel;
	std::map<unsigned short, RecordRange >::iterator iterCounties;
	std::map<unsigned short, CountySquares >::iterator iterCountySquares;
	std::map<unsigned short, WaterPolygons >::iterator iterCountyWaterPolys;
	std::map<unsigned short, Rect>::iterator countyRect;
	std::list<CountySquare *>::iterator iterRecordSquares;
	QString strText, strRouteNumber, strFullName;
	QColor clrText, clrLine;
	int i, j, left, right, top, bottom, sqWidth, sqHeight, iFontSize, iEllipseWidth, iEllipseHeight, iSymbolWidth, iSymbolHeight, iWidth, iStyle, iArrowSpacing, iSeg, iOffset;
	unsigned int iRec;
	QSize szText;
	QPoint p1, p2, p;
	std::vector<QPoint> parPoints;
	Coords ptStart, ptEnd;
	double fAngle, fEllipseScaling, fSymbolScaling;
	QFont fnt, oldFont;
	QPen penOutline, penThis, oldPen;
	QBrush brCircleFill, brThis, oldBrush;
	MapRecord * psRec;
	bool bVisible;
	DetailSettings * pLevelDetails;

	// get visible squares of counties
	Rect bounds(pSettings->ptTopLeftClip, pSettings->ptBottomRightClip);
	for (iterCounties = m_mapCountyCodeToRecords.begin(); iterCounties != m_mapCountyCodeToRecords.end(); ++iterCounties) {
		countyRect = m_mapCountyCodeToBoundingRect.find(iterCounties->first);
		if (countyRect != m_mapCountyCodeToBoundingRect.end() && bounds.intersectRect(countyRect->second)) {
			iterCountySquares = m_mapCountyCodeToRegions.find(iterCounties->first);
			sqWidth = (int)ceil((countyRect->second.m_iRight - countyRect->second.m_iLeft) / SQUARES_PER_COUNTY_SIDE);
			sqHeight = (int)ceil((countyRect->second.m_iTop - countyRect->second.m_iBottom) / SQUARES_PER_COUNTY_SIDE);
			left = (bounds.m_iLeft - countyRect->second.m_iLeft) / sqWidth;
			if (left < 0) left = 0; if (left > (int)SQUARES_PER_COUNTY_SIDE - 1) left = (int)SQUARES_PER_COUNTY_SIDE-1;
			right = (bounds.m_iRight - countyRect->second.m_iLeft) / sqWidth;
			if (right < 0) right = 0; if (right > (int)SQUARES_PER_COUNTY_SIDE - 1) right = (int)SQUARES_PER_COUNTY_SIDE-1;
			top = (countyRect->second.m_iTop - bounds.m_iTop) / sqHeight;
			if (top < 0) top = 0; if (top > (int)SQUARES_PER_COUNTY_SIDE - 1) top = (int)SQUARES_PER_COUNTY_SIDE-1;
			bottom = (countyRect->second.m_iTop - bounds.m_iBottom) / sqHeight;
			if (bottom < 0) bottom = 0; if (bottom > (int)SQUARES_PER_COUNTY_SIDE - 1) bottom = (int)SQUARES_PER_COUNTY_SIDE-1;
			for (i = left; i <= right; i++) {
				for (j = top; j <= bottom; j++)
					recordSquares.push_back(&iterCountySquares->second[i+j*(int)SQUARES_PER_COUNTY_SIDE]);
			}
		}
	}

	// fill in water
	if (pSettings->bFillInWater) {
		pLevelDetails = &pSettings->vecLevelDetails[pSettings->iDetailLevel][RecordTypeWater];
		pSettings->pMemoryDC->setRasterOp(Qt::XorROP);
		clrLine = pSettings->clrBackground.rgb() ^ pLevelDetails->clrLine.rgb();
		iWidth = pLevelDetails->iWidth;
		iStyle = pLevelDetails->iStyle;
		QPointArray waterPoly;
		QPoint ptWater;
		int iSeg2;
		oldPen = pSettings->pMemoryDC->pen();
		pSettings->pMemoryDC->setPen(clrLine);
		oldBrush = pSettings->pMemoryDC->brush();
		pSettings->pMemoryDC->setBrush(clrLine);

		for (iterCounties = m_mapCountyCodeToRecords.begin(); iterCounties != m_mapCountyCodeToRecords.end(); ++iterCounties) {
			countyRect = m_mapCountyCodeToBoundingRect.find(iterCounties->first);
			if (countyRect == m_mapCountyCodeToBoundingRect.end() || bounds.intersectRect(countyRect->second)) {
				iterCountyWaterPolys = m_mapCountyCodeToWaterPolys.find(iterCounties->first);
				if (iterCountyWaterPolys != m_mapCountyCodeToWaterPolys.end()) {
					for (iRec = 0; iRec < iterCountyWaterPolys->second.size(); iRec++) {
						bVisible = iterCountyWaterPolys->second[iRec].first.intersectRect(bounds);
//						bVisible = (iterCountyWaterPolys->second[iRec].first.m_iLeft > bounds.m_iRight || iterCountyWaterPolys->second[iRec].first.m_iRight < bounds.m_iLeft || iterCountyWaterPolys->second[iRec].first.m_iTop < bounds.m_iBottom || iterCountyWaterPolys->second[iRec].first.m_iBottom > bounds.m_iTop) ? false : IsRecordVisible(iterCountyWaterPolys->second[iRec].second, pSettings->ptTopLeftClip, pSettings->ptBottomRightClip);
						if (bVisible) {
							waterPoly.resize(iterCountyWaterPolys->second[iRec].second.size());
							for (iSeg2 = iSeg = 0; iSeg < (int)iterCountyWaterPolys->second[iRec].second.size(); iSeg++) {
								ptWater = MapLongLatToScreen(pSettings, iterCountyWaterPolys->second[iRec].second[iSeg]);
								if (iSeg2 == 0 || ptWater != waterPoly[iSeg2-1])
									waterPoly[iSeg2++] = ptWater;
							}
							waterPoly.resize(iSeg2);
							pSettings->pMemoryDC->drawPolygon(waterPoly, true);
						}
					}
				}
			}
		}

		// draw boundaries of water polygons - the polygons overlap, causing the color to change - change it back!
		for (iterCounties = m_mapCountyCodeToRecords.begin(); iterCounties != m_mapCountyCodeToRecords.end(); ++iterCounties) {
			countyRect = m_mapCountyCodeToBoundingRect.find(iterCounties->first);
			if (countyRect == m_mapCountyCodeToBoundingRect.end() || bounds.intersectRect(countyRect->second)) {
				iterCountyWaterPolys = m_mapCountyCodeToWaterPolys.find(iterCounties->first);
				if (iterCountyWaterPolys != m_mapCountyCodeToWaterPolys.end()) {
					for (iRec = 0; iRec < iterCountyWaterPolys->second.size(); iRec++) {
						bVisible = iterCountyWaterPolys->second[iRec].first.intersectRect(bounds);
//						bVisible = (iterCountyWaterPolys->second[iRec].first.m_iLeft > bounds.m_iRight || iterCountyWaterPolys->second[iRec].first.m_iRight < bounds.m_iLeft || iterCountyWaterPolys->second[iRec].first.m_iTop < bounds.m_iBottom || iterCountyWaterPolys->second[iRec].first.m_iBottom > bounds.m_iTop) ? false : IsRecordVisible(iterCountyWaterPolys->second[iRec].second, pSettings->ptTopLeftClip, pSettings->ptBottomRightClip);
						if (bVisible) {
							waterPoly.resize(iterCountyWaterPolys->second[iRec].second.size());
							for (iSeg2 = iSeg = 0; iSeg < (int)iterCountyWaterPolys->second[iRec].second.size(); iSeg++) {
								ptWater = MapLongLatToScreen(pSettings, iterCountyWaterPolys->second[iRec].second[iSeg]);
								if (iSeg2 == 0 || ptWater != waterPoly[iSeg2-1])
									waterPoly[iSeg2++] = ptWater;
							}
							waterPoly.resize(iSeg2);
							for (iSeg2 = 0; iSeg2 < (int)waterPoly.size() - 1; iSeg2++)
								DrawLine(pSettings->pMemoryDC, waterPoly[iSeg2].x(), waterPoly[iSeg2].y(), waterPoly[iSeg2+1].x(), waterPoly[iSeg2+1].y(), iWidth, clrLine, (Qt::PenStyle)iStyle);
							if (waterPoly.size() > 1)
								DrawLine(pSettings->pMemoryDC, waterPoly[waterPoly.size()-1].x(), waterPoly[waterPoly.size()-1].y(), waterPoly[0].x(), waterPoly[0].y(), iWidth, clrLine, (Qt::PenStyle)iStyle);
						}
					}
				}
			}
		}

		pSettings->pMemoryDC->setRasterOp(Qt::CopyROP);
		pSettings->pMemoryDC->setPen(oldPen);
		pSettings->pMemoryDC->setBrush(oldBrush);
	}

	// draw water boundaries
	for (iterRecordSquares = recordSquares.begin(); iterRecordSquares != recordSquares.end(); ++iterRecordSquares) {
		for (iRec = 0; iRec < (*iterRecordSquares)->size(); iRec++)
		{
			psRec = m_pRecords + (**iterRecordSquares)[iRec];
			bVisible = psRec->rBounds.intersectRect(bounds);
//			bVisible = (psRec->rBounds.m_iLeft > bounds.m_iRight || psRec->rBounds.m_iRight < bounds.m_iLeft || psRec->rBounds.m_iTop < bounds.m_iBottom || psRec->rBounds.m_iBottom > bounds.m_iTop) ? false : IsRecordVisible(psRec->pShapePoints, psRec->nShapePoints, pSettings->ptTopLeftClip, pSettings->ptBottomRightClip);
			if (bVisible)
			{
				if (psRec->eRecordType == RecordTypeWater)
				{
					pLevelDetails = &pSettings->vecLevelDetails[pSettings->iDetailLevel][psRec->eRecordType];
					if (pLevelDetails->bLineVisible) {
						clrLine = pLevelDetails->clrLine;
						iWidth = pLevelDetails->iWidth;
						iStyle = pLevelDetails->iStyle;
						parPoints.resize(psRec->nShapePoints);
						for (iSeg = 0; iSeg < psRec->nShapePoints; iSeg++)
							parPoints[iSeg] = MapLongLatToScreen(pSettings, psRec->pShapePoints[iSeg]);

						for (iSeg = 0; iSeg < psRec->nShapePoints - 1; iSeg++)
							DrawLine(pSettings->pMemoryDC, parPoints[iSeg].x(), parPoints[iSeg].y(), parPoints[iSeg+1].x(), parPoints[iSeg+1].y(), iWidth, clrLine, (Qt::PenStyle)iStyle);
					}
				}
			}
		}
	}

	// draw general features from records for each visible square (counties and states)
	for (iterRecordSquares = recordSquares.begin(); iterRecordSquares != recordSquares.end(); ++iterRecordSquares) {
		for (iRec = 0; iRec < (*iterRecordSquares)->size(); iRec++)
		{
			psRec = m_pRecords + (**iterRecordSquares)[iRec];
			if (psRec->eRecordType == RecordTypeWater) continue;
			bVisible = psRec->rBounds.intersectRect(bounds);
//			bVisible = (psRec->rBounds.m_iLeft > bounds.m_iRight || psRec->rBounds.m_iRight < bounds.m_iLeft || psRec->rBounds.m_iTop < bounds.m_iBottom || psRec->rBounds.m_iBottom > bounds.m_iTop) ? false : IsRecordVisible(psRec->pShapePoints, psRec->nShapePoints, pSettings->ptTopLeftClip, pSettings->ptBottomRightClip);
			if (bVisible)
			{
				pLevelDetails = &pSettings->vecLevelDetails[pSettings->iDetailLevel][psRec->eRecordType];

				if (pLevelDetails->bLineVisible)
				{
					clrLine = pLevelDetails->clrLine;
					iWidth = pLevelDetails->iWidth;
					iStyle = pLevelDetails->iStyle;
					iArrowSpacing = pLevelDetails->iArrowSpacing;

					parPoints.resize(psRec->nShapePoints);
					for (iSeg = 0; iSeg < psRec->nShapePoints; iSeg++)
						parPoints[iSeg] = MapLongLatToScreen(pSettings, psRec->pShapePoints[iSeg]);

					if (psRec->nShapePoints == 1)
					{
						// a place
						penThis = QPen(clrLine, 1, (Qt::PenStyle)iStyle);
						brThis = QBrush(clrLine, (Qt::BrushStyle)iStyle);

						oldPen = pSettings->pMemoryDC->pen();
						oldBrush = pSettings->pMemoryDC->brush();
						pSettings->pMemoryDC->setPen(penThis);
						pSettings->pMemoryDC->setBrush(brThis);

						pSettings->pMemoryDC->drawPie(parPoints[0].x(), parPoints[0].y(), iWidth, iWidth, 0, 5760);

						pSettings->pMemoryDC->setPen(oldPen);
						pSettings->pMemoryDC->setBrush(oldBrush);
					}
					for (iSeg = 0; iSeg < psRec->nShapePoints - 1; iSeg++)
						DrawLine(pSettings->pMemoryDC, parPoints[iSeg].x(), parPoints[iSeg].y(), parPoints[iSeg+1].x(), parPoints[iSeg+1].y(), iWidth, clrLine, (Qt::PenStyle)iStyle);
				}

				if (pLevelDetails->bTextVisible)
				{
					strFullName = GetNameAndType(psRec - m_pRecords);
					modLabel = mapLabels.find(strFullName);
					if (modLabel == mapLabels.end()) {
						modLabel = mapLabels.insert(std::pair<QString, LabelInfo>(strFullName, LabelInfo())).first;
						modLabel->second.fDistance = 0;
					}

					if (psRec->nShapePoints > 1)
					{
						for (iOffset = 0; iOffset < psRec->nShapePoints - 1; iOffset++)
						{
							if (Distance(psRec->pShapePoints[iOffset], psRec->pShapePoints[iOffset + 1]) >= modLabel->second.fDistance)
							{
								modLabel->second.fDistance = Distance(psRec->pShapePoints[0], psRec->pShapePoints[psRec->nShapePoints - 1]);
								modLabel->second.ptStart = psRec->pShapePoints[iOffset];
								modLabel->second.ptEnd = psRec->pShapePoints[iOffset + 1];
								modLabel->second.pLevelDetails = pLevelDetails;
							}
						}
					}
					else
					{
						modLabel->second.fDistance = 0;
						modLabel->second.ptStart = psRec->pShapePoints[0];
						modLabel->second.ptEnd = psRec->pShapePoints[0];
						modLabel->second.pLevelDetails = pLevelDetails;
					}
				}
			}
		}
	}

	// draw the text
	for (itLabels = mapLabels.begin(); itLabels != mapLabels.end(); itLabels++)
	{
		strText = itLabels->first;
		clrText = itLabels->second.pLevelDetails->clrText;
		iFontSize = itLabels->second.pLevelDetails->iFontSize;

		fnt = QFont("Helvetica", iFontSize, QFont::Normal);
		oldFont = pSettings->pMemoryDC->font();
		pSettings->pMemoryDC->setFont(fnt);
		szText = pSettings->pMemoryDC->fontMetrics().size(Qt::SingleLine, strText);

		ptStart = itLabels->second.ptStart;
		ptEnd = itLabels->second.ptEnd;

		p1 = MapLongLatToScreen(pSettings, ptStart);
		p2 = MapLongLatToScreen(pSettings, ptEnd);

		p.setX((p1.x() + p2.x()) / 2);
		p.setY((p1.y() + p2.y()) / 2);

		if (strText.left(2) == "I-")
		{
			// Interstate
			// route number inside a blue shield with a red hat

			penOutline = QPen(QColor(0, 0, 0), 1, Qt::SolidLine);
			fEllipseScaling = 1.3;
			strRouteNumber = strText.mid(2);
			szText = pSettings->pMemoryDC->fontMetrics().size(Qt::SingleLine, strRouteNumber);

			iEllipseWidth = (int) (szText.width() * fEllipseScaling);
			iEllipseHeight = (int) (szText.height() * fEllipseScaling);
			if (iEllipseWidth < iEllipseHeight * 1.1)
				iEllipseWidth = (int) (iEllipseHeight * 1.1);

			oldPen = pSettings->pMemoryDC->pen();
			pSettings->pMemoryDC->setPen(QColor(0, 0, 255));

			oldBrush = pSettings->pMemoryDC->brush();
			pSettings->pMemoryDC->setBrush(QColor(0, 0, 255));

			pSettings->pMemoryDC->drawPie((int) (p.x() - iEllipseWidth * 1.5), (int) (p.y() - iEllipseHeight * 1.5 - iEllipseHeight * 0.1), (int) (iEllipseWidth * 2), (int) (iEllipseHeight * 2.4), 4800, 960);
			pSettings->pMemoryDC->drawPie((int) (p.x() - iEllipseWidth * 0.5), (int) (p.y() - iEllipseHeight * 1.5 - iEllipseHeight * 0.1), (int) (iEllipseWidth * 2), (int) (iEllipseHeight * 2.4), 2880, 960);

			pSettings->pMemoryDC->setPen(penOutline);

			pSettings->pMemoryDC->drawArc((int) (p.x() - iEllipseWidth * 1.5), (int) (p.y() - iEllipseHeight * 1.5 - iEllipseHeight * 0.1), (int) (iEllipseWidth * 2), (int) (iEllipseHeight * 2.4), 4800, 960);
			pSettings->pMemoryDC->drawArc((int) (p.x() - iEllipseWidth * 0.5), (int) (p.y() - iEllipseHeight * 1.5 - iEllipseHeight * 0.1), (int) (iEllipseWidth * 2), (int) (iEllipseHeight * 2.4), 2880, 960);

			pSettings->pMemoryDC->setBrush(QColor(255, 0, 0));
			pSettings->pMemoryDC->drawRect(p.x() - iEllipseWidth / 2, (int) (p.y() - iEllipseHeight / 2 - iEllipseHeight * 0.1), iEllipseWidth, iEllipseHeight / 4);
			pSettings->pMemoryDC->fillRect(p.x() - iEllipseWidth / 2, (int) (p.y() - iEllipseHeight / 2 - iEllipseHeight * 0.1), iEllipseWidth, iEllipseHeight / 4, QColor(255, 0, 0));
			pSettings->pMemoryDC->setPen(QColor(255, 255, 255));

			pSettings->pMemoryDC->drawText(p.x() - szText.width() / 2, p.y() - szText.height() / 2 + pSettings->pMemoryDC->fontMetrics().ascent(), strRouteNumber);

			pSettings->pMemoryDC->setPen(oldPen);
			pSettings->pMemoryDC->setBrush(oldBrush);
		}
		else if (strText.left(3) == "US-")
		{
			// US highway
			// route number inside a white shield

			penOutline = QPen(QColor(0, 0, 0), 1, Qt::SolidLine);
			fSymbolScaling = 1.2;

			strRouteNumber = strText.mid(3);
			szText = pSettings->pMemoryDC->fontMetrics().size(Qt::SingleLine, strRouteNumber);

			iSymbolWidth = (int) (szText.width() * fSymbolScaling);
			iSymbolHeight = (int) (szText.height() * fSymbolScaling);
			if (iSymbolWidth < iSymbolHeight * 1.1)
				iSymbolWidth = (int) (iSymbolHeight * 1.1);

			oldPen = pSettings->pMemoryDC->pen();
			pSettings->pMemoryDC->setPen(penOutline);

			oldBrush = pSettings->pMemoryDC->brush();
			pSettings->pMemoryDC->setBrush(QColor(255, 255, 255));

			pSettings->pMemoryDC->drawRoundRect(p.x() - iSymbolWidth / 2, p.y() - iSymbolHeight / 2, iSymbolWidth, iSymbolHeight, 25, 25);

			pSettings->pMemoryDC->setPen(QColor(0, 0, 0));

			pSettings->pMemoryDC->drawText(p.x() - szText.width() / 2, p.y() - szText.height() / 2 + pSettings->pMemoryDC->fontMetrics().ascent(), strRouteNumber);

			pSettings->pMemoryDC->setPen(oldPen);
			pSettings->pMemoryDC->setBrush(oldBrush);
		}
		else if (strText.length() >= 3 && strText.at(2) == '-' && StateNameByAbbreviation(strText.left(2)) != "")
		{
			// state route
			// route number in a white circle

			penOutline = QPen(QColor(0, 0, 0), 1, Qt::SolidLine);
			fEllipseScaling = 1.2;

			strRouteNumber = strText.mid(3);
			szText = pSettings->pMemoryDC->fontMetrics().size(Qt::SingleLine, strRouteNumber);

			iEllipseWidth = (int) (szText.width() * fEllipseScaling);
			iEllipseHeight = (int) (szText.height() * fEllipseScaling);
			if (iEllipseWidth < iEllipseHeight * 1.1)
				iEllipseWidth = (int) (iEllipseHeight * 1.1);

			oldPen = pSettings->pMemoryDC->pen();
			pSettings->pMemoryDC->setPen(QColor(255, 255, 255));

			oldBrush = pSettings->pMemoryDC->brush();
			pSettings->pMemoryDC->setBrush(QColor(255, 255, 255));

			pSettings->pMemoryDC->drawPie(p.x() - iEllipseWidth / 2, p.y() - iEllipseHeight / 2, iEllipseWidth, iEllipseHeight, 0, 5760);

			pSettings->pMemoryDC->setPen(penOutline);
			pSettings->pMemoryDC->drawEllipse(p.x() - iEllipseWidth / 2, p.y() - iEllipseHeight / 2, iEllipseWidth, iEllipseHeight);

			pSettings->pMemoryDC->setPen(QColor(0, 0, 0));

			pSettings->pMemoryDC->drawText(p.x() - szText.width() / 2, p.y() - szText.height() / 2 + pSettings->pMemoryDC->fontMetrics().ascent(), strRouteNumber);

			pSettings->pMemoryDC->setPen(oldPen);
			pSettings->pMemoryDC->setBrush(oldBrush);
		}
		else
		{
			if (p1 != p2)
				fAngle = ::atan2(p2.x() - p1.x(), p2.y() - p1.y()) - M_PI / 2;
			else
				fAngle = 0;

			if (fAngle > M_PI / 2)
				fAngle -= M_PI;

			if (fAngle < -M_PI / 2)
				fAngle += M_PI;
			DrawRotatedText(pSettings->pMemoryDC, strText, p, szText, fAngle * 180 / M_PI, clrText);
		}

		pSettings->pMemoryDC->setFont(oldFont);
	}
}

void MapDB::DrawMapCompass(MapDrawingSettings * pSettings)
{
	if (!pSettings->bShowCompass)
		return;

	int iFontSize = 8;
	QFont fnt("Helvetica", iFontSize, QFont::Normal);
	QFont oldFont(pSettings->pMemoryDC->font());

	QString strNKeyLabel = "N";
	QString strEKeyLabel = "E";
	QString strSKeyLabel = "S";
	QString strWKeyLabel = "W";

	QSize szN, szW, szS, szE;

	pSettings->pMemoryDC->setFont(fnt);

	szN = pSettings->pMemoryDC->fontMetrics().size(Qt::SingleLine, strNKeyLabel);
	szW = pSettings->pMemoryDC->fontMetrics().size(Qt::SingleLine, strWKeyLabel);
	szS = pSettings->pMemoryDC->fontMetrics().size(Qt::SingleLine, strSKeyLabel);
	szE = pSettings->pMemoryDC->fontMetrics().size(Qt::SingleLine, strEKeyLabel);

	int iCompassRadius = 6;
	int iCompassArrowLength = 15;
	int iArrowRadius = 5;

	int iCompassX = 3 + iCompassArrowLength + szW.width();
	int iCompassY = 3 + iCompassArrowLength + szN.height();
	int iNTextX = iCompassX - szN.width() / 2;
	int iNTextY = iCompassY - iCompassArrowLength - szN.height() + pSettings->pMemoryDC->fontMetrics().ascent();
	int iETextX = (int) (iCompassX + iCompassArrowLength + 1);
	int iETextY = iCompassY - szE.height() / 2 + pSettings->pMemoryDC->fontMetrics().ascent();
	int iSTextX = iCompassX - szS.width() / 2;
	int iSTextY = iCompassY + iCompassArrowLength + pSettings->pMemoryDC->fontMetrics().ascent();
	int iWTextX = iCompassX - iCompassArrowLength - szW.width();
	int iWTextY = iCompassY - szW.height() / 2 + pSettings->pMemoryDC->fontMetrics().ascent();

	QPointArray pts(8);
	// top left
	pts.setPoint(0, (int) (iCompassX - iArrowRadius * ::sqrt(2) / 2), (int) (iCompassY - iArrowRadius * ::sqrt(2) / 2));
	// top
	pts.setPoint(1, (int) (iCompassX), (int) (iCompassY - iCompassArrowLength));
	// top right
	pts.setPoint(2, (int) (iCompassX + iArrowRadius * ::sqrt(2) / 2), (int) (iCompassY - iArrowRadius * ::sqrt(2) / 2));
	// right
	pts.setPoint(3, (int) (iCompassX + iCompassArrowLength), (int) (iCompassY));
	// bottom right
	pts.setPoint(4, (int) (iCompassX + iArrowRadius * ::sqrt(2) / 2), (int) (iCompassY + iArrowRadius * ::sqrt(2) / 2));
	// bottom
	pts.setPoint(5, (int) (iCompassX), (int) (iCompassY + iCompassArrowLength));
	// bottom left
	pts.setPoint(6, (int)(iCompassX - iArrowRadius * ::sqrt(2) / 2), (int) (iCompassY + iArrowRadius * ::sqrt(2) / 2));
	// left
	pts.setPoint(7, (int) (iCompassX - iCompassArrowLength), (int) (iCompassY));

	QPen penBlack(QColor(0, 0, 0), 1, Qt::SolidLine);
	QPen oldPen(pSettings->pMemoryDC->pen());
	pSettings->pMemoryDC->setPen(penBlack);

	QBrush brBackground(pSettings->clrBackground, Qt::SolidPattern);
	QBrush oldBrush(pSettings->pMemoryDC->brush());
	pSettings->pMemoryDC->setBrush(brBackground);

	pSettings->pMemoryDC->drawText(iNTextX, iNTextY, strNKeyLabel);
	pSettings->pMemoryDC->drawText(iETextX, iETextY, strEKeyLabel);
	pSettings->pMemoryDC->drawText(iSTextX, iSTextY, strSKeyLabel);
	pSettings->pMemoryDC->drawText(iWTextX, iWTextY, strWKeyLabel);

	pSettings->pMemoryDC->drawPolygon(pts);
	pSettings->pMemoryDC->drawEllipse(iCompassX - iCompassRadius, iCompassY - iCompassRadius, 2*iCompassRadius, 2*iCompassRadius);

	pSettings->pMemoryDC->setPen(oldPen);
	pSettings->pMemoryDC->setBrush(oldBrush);
	pSettings->pMemoryDC->setFont(oldFont);
}

void MapDB::DrawMapKey(MapDrawingSettings * pSettings)
{
	if (!pSettings->bShowScale)
		return;

	// draw key
	double fKeyLengthMiles = 64;
	int iKeyLengthPixels = 1000;

	while (iKeyLengthPixels > 50)
	{
		fKeyLengthMiles /= 2;
//		iKeyLengthPixels = (int) (fKeyLengthMiles / ((2 * M_PI * EARTHRADIUS) * (pSettings->vecLevelScales[pSettings->iDetailLevel] / 360000000)));
		iKeyLengthPixels = (int) (fKeyLengthMiles / ((2 * M_PI * EARTHRADIUS * (1 << pSettings->iDetailLevel) / 360000000)));
	}

	QString strLabel;
	if (fKeyLengthMiles >= 0.1)
		strLabel = QString("%1 miles").arg(fKeyLengthMiles, 0, 'f', 2);
	else
		strLabel = QString("%1 feet").arg(fKeyLengthMiles * 5280, 0, 'f', 1);

	int iFontSize = 8;
	int iKeyW = iKeyLengthPixels;
	int iKeyH = 5;
	int iKeyX = pSettings->iControlWidth - 5 - iKeyW;
	int iKeyY = pSettings->iControlHeight - 5 - iKeyH;

	QPen penBlack(QColor(0, 0, 0), 1, Qt::SolidLine);
	QPen oldPen(pSettings->pMemoryDC->pen());
	pSettings->pMemoryDC->setPen(penBlack);
	QBrush oldBrush(pSettings->pMemoryDC->brush());
	pSettings->pMemoryDC->setBrush(QColor(255, 255, 255));
	pSettings->pMemoryDC->fillRect(iKeyX, iKeyY, iKeyW, iKeyH, QColor(255, 255, 255));
	pSettings->pMemoryDC->drawRect(iKeyX, iKeyY, iKeyW, iKeyH);

	pSettings->pMemoryDC->setPen(oldPen);
	pSettings->pMemoryDC->setBrush(oldBrush);

	QFont fnt("Helvetica", iFontSize, QFont::Normal);
	QFont oldFont(pSettings->pMemoryDC->font());
	pSettings->pMemoryDC->setFont(fnt);
	oldPen = pSettings->pMemoryDC->pen();
	pSettings->pMemoryDC->setPen(QColor(0, 0, 0));

	QSize szText = pSettings->pMemoryDC->fontMetrics().size(Qt::SingleLine, strLabel);

	pSettings->pMemoryDC->drawText(iKeyX + iKeyW - szText.width(), iKeyY - szText.height() + pSettings->pMemoryDC->fontMetrics().ascent(), strLabel);

	pSettings->pMemoryDC->setFont(oldFont);
	pSettings->pMemoryDC->setPen(oldPen);
}













bool InitMapDB()
{
	bool bSuccess = true;
	g_pLogger->LogInfo("Loading state information...");
	if (LoadStateInformation(GetDataPath("states.tdf")))
		g_pLogger->LogInfo("Successful\n");
	else {
		g_pLogger->LogInfo("Failed\n");
		bSuccess = false;
	}
	g_pLogger->LogInfo("Loading county information...");
	if (LoadCountyInformation(GetDataPath("counties.txt", COUNTIES_URL, false)))
		g_pLogger->LogInfo("Successful\n");
	else {
		g_pLogger->LogInfo("Failed\n");
		bSuccess = false;
	}
	g_pLogger->LogInfo("Loading zip code information...");
	if (LoadZipInformation(GetDataPath("zips.tdf")))
		g_pLogger->LogInfo("Successful\n");
	else {
		g_pLogger->LogInfo("Failed\n");
		bSuccess = false;
	}
	g_pLogger->LogInfo("Loading abbreviations...");
	if (LoadAbbreviationInformation(GetDataPath("abbreviations.txt")))
		g_pLogger->LogInfo("Successful\n");
	else {
		g_pLogger->LogInfo("Failed\n");
		bSuccess = false;
	}
	return bSuccess;
}

bool LoadAbbreviationInformation(const QString & strFilename)
{
	FILE * hFile;
	char szLine[2048];

	hFile = fopen(strFilename, "rb");
	if (!hFile)
		return false;

	fgets(szLine, sizeof(szLine), hFile);
	while (!feof(hFile))
	{
		QString strName = "";
		QString strLongAbbrev = "";
		QString strShortAbbrev = "";
		QString strTranslation = "";
		bool bValid = true;

		char * pszToken;

		qApp->processEvents();

		// strip off CR/LF
		while (strlen(szLine) && (szLine[strlen(szLine) - 1] == '\r' || szLine[strlen(szLine) - 1] == '\n'))
			szLine[strlen(szLine) - 1] = 0;

		pszToken = strtok(szLine, "\t");

		if (pszToken)
		{
			strName = pszToken;
			strName = strName.stripWhiteSpace();
			pszToken = strtok(NULL, "\t");
		}

		if (pszToken)
		{
			strLongAbbrev = pszToken;
			strLongAbbrev = strLongAbbrev.stripWhiteSpace();
			pszToken = strtok(NULL, "\t");
		}

		if (pszToken)
		{
			strShortAbbrev = pszToken;
			strShortAbbrev = strShortAbbrev.stripWhiteSpace();
			pszToken = strtok(NULL, "\t");
		}
		else
		{
			bValid = false;
		}

		if (pszToken)
		{
			strTranslation = pszToken;
			strTranslation = strTranslation.stripWhiteSpace();
			pszToken = strtok(NULL, "\t");
		}


		if (bValid)
		{
			if (strShortAbbrev == "-")
				strShortAbbrev = strLongAbbrev;

			g_mapAbbreviationToFullName[strShortAbbrev.lower()] = strName;
			g_mapAbbreviationToFullName[strLongAbbrev.lower()] = strName;

			g_mapNameToAbbreviation[strLongAbbrev.lower()] = strShortAbbrev;
			g_mapNameToAbbreviation[strName.lower()] = strShortAbbrev;
		}

		fgets(szLine, sizeof(szLine), hFile);
	}

	fclose(hFile);
	qApp->processEvents();
	return true;
}

QString GetAbbreviation(const QString & strName)
{
	if (g_mapNameToAbbreviation.find(strName.lower()) != g_mapNameToAbbreviation.end())
		return g_mapNameToAbbreviation[strName.lower()];

	return "";
}

QString GetFullName(const QString & strName)
{
	if (g_mapAbbreviationToFullName.find(strName.lower()) != g_mapAbbreviationToFullName.end())
		return g_mapAbbreviationToFullName[strName.lower()];

	return "";
}

bool IsFeatureType(const QString & strName)
{
	if (!GetAbbreviation(strName).isEmpty() || !GetFullName(strName).isEmpty())
		return true;

	return false;
}

bool LoadCountyInformation(const QString & strFilename)
{
	FILE * hFile;
	char szLine[2048];

	hFile = fopen(strFilename, "rb");
	if (!hFile)
		return false;

	fgets(szLine, sizeof(szLine), hFile);
	while (!feof(hFile))
	{
		qApp->processEvents();
		// County name
		QString strCounty;
		// State abbreviation
		QString strState;
		// State + county
		QString strCombined;
		// FIPS code
		int iCode;

		// only process lines starting with a state code
		if (isdigit(szLine[0]))
		{
			// strip off CR/LF
			while (strlen(szLine) && (szLine[strlen(szLine) - 1] == '\r' || szLine[strlen(szLine) - 1] == '\n'))
				szLine[strlen(szLine) - 1] = 0;

			// combine state and county code
			iCode = atoi(ExtractField(szLine, 1, 2)) * 1000 + atoi(ExtractField(szLine, 8, 10));

			strCounty = szLine + 15;
			strState = strCounty.right(2);
			strCounty = strCounty.left(strCounty.length() - 2);
			strCounty = strCounty.stripWhiteSpace();

			strCombined = strCounty;

			g_mapCountyNameToCode[strCombined] = iCode;
			g_mapCodeToCountyName[iCode] = strCombined;
			g_vecCountyNames.push_back(strCombined);
		}

		fgets(szLine, sizeof(szLine), hFile);
	}

	fclose(hFile);
	qApp->processEvents();

	return true;
}

unsigned short CountyCodeByName(const QString & strName)
{
	if (g_mapCountyNameToCode.find(strName) != g_mapCountyNameToCode.end())
		return g_mapCountyNameToCode[strName];

	return (unsigned)-1;
}

QString CountyNameByCode(unsigned short iCode)
{
	if (g_mapCodeToCountyName.find(iCode) != g_mapCodeToCountyName.end())
		return g_mapCodeToCountyName[iCode];

	return "";
}

bool LoadStateInformation(const QString & strFilename)
{
	FILE * hFile;
	char szLine[2048];

	hFile = fopen(strFilename, "rb");
	if (!hFile)
		return false;

	fgets(szLine, sizeof(szLine), hFile);
	QString strLine;
	long lCode;
	QString strAbbr;
	QString strName;
	while (!feof(hFile))
	{
		qApp->processEvents();
		strLine = szLine;
		strLine = strLine.stripWhiteSpace();

		lCode = strLine.section('\t', 0, 0).toLong();
		strAbbr = strLine.section('\t', 1, 1);
		strName = strLine.section('\t', 2, 2);

		g_mapStateNameToCode[strName] = lCode;
		g_mapStateCodeToName[lCode] = strName;
		g_mapStateAbbreviationToName[strAbbr] = strName;
		g_mapStateNameToAbbreviation[strName] = strAbbr;
		g_vecStateAbbreviations.push_back(strAbbr);
		g_vecStateNames.push_back(strName);

		fgets(szLine, sizeof(szLine), hFile);
	}

	fclose(hFile);
	qApp->processEvents();
	return true;
}

QString StateAbbreviationByName(const QString & strName)
{
	QString strFormattedName = FormatName(strName);

	if (g_mapStateNameToAbbreviation.find(strFormattedName) != g_mapStateNameToAbbreviation.end())
		return g_mapStateNameToAbbreviation[strFormattedName];

	return "";
}

QString StateAbbreviationByCode(unsigned short iCode)
{
	if (g_mapStateCodeToName.find(iCode) != g_mapStateCodeToName.end())
	{
		QString strStateName = g_mapStateCodeToName[iCode];

		return g_mapStateNameToAbbreviation[strStateName];
	}
	return "";
}

QString StateNameByAbbreviation(const QString & strName)
{
	QString strFormattedAbbreviation = FormatAbbreviation(strName);
	if (g_mapStateAbbreviationToName.find(strFormattedAbbreviation) != g_mapStateAbbreviationToName.end())
		return g_mapStateAbbreviationToName[strFormattedAbbreviation];

	return "";
}

QString StateNameByCode(unsigned short iStateCode)
{
	if (g_mapStateCodeToName.find(iStateCode) != g_mapStateCodeToName.end())
		return g_mapStateCodeToName[iStateCode];

	return "";
}

unsigned short StateCodeByName(const QString & strStateName)
{
	QString strName;

	strName = FormatName(strStateName);

	if (g_mapStateAbbreviationToName.find(FormatAbbreviation(strStateName)) != g_mapStateAbbreviationToName.end())
		strName = g_mapStateAbbreviationToName[FormatAbbreviation(strStateName)];

	if (g_mapStateNameToCode.find(strName) != g_mapStateNameToCode.end())
		return g_mapStateNameToCode[strName];

	return (unsigned)-1;
}

QString FormatAddress(Address * psAddr)
{
	QString rtn;
	QString strType;

	if (psAddr)
	{
		if (psAddr->iVertex == (unsigned)-1) {
			QString strNumber;
			QString strType;

			strNumber = psAddr->iStreetNumber ? QString("%1 ").arg(psAddr->iStreetNumber) : "";

			strType = " " + GetFullName(psAddr->szStreetType);
			if (strType == " ")
				strType = QString(" ") + psAddr->szStreetType;

			QString strLong = QString("%1 %2").arg(abs(psAddr->ptCoordinates.m_iLong)/1e6, 0, 'f', 6).arg(psAddr->ptCoordinates.m_iLong >= 0 ? "E" : "W");
			QString strLat = QString("%1 %2").arg(abs(psAddr->ptCoordinates.m_iLat)/1e6, 0, 'f', 6).arg(psAddr->ptCoordinates.m_iLat >= 0 ? "N" : "S");
			rtn = QString("%1%2%3\n%4, %5 %6\n%7, %8").arg(strNumber).arg(psAddr->szStreetName).arg(strType).arg(psAddr->szCityName).arg(StateAbbreviationByName(psAddr->szStateName)).arg(psAddr->iZipCode,5).arg(strLong).arg(strLat);
		} else {
			QString strLong = QString("%1 %2").arg(abs(psAddr->ptCoordinates.m_iLong)/1e6, 0, 'f', 6).arg(psAddr->ptCoordinates.m_iLong >= 0 ? "E" : "W");
			QString strLat = QString("%1 %2").arg(abs(psAddr->ptCoordinates.m_iLat)/1e6, 0, 'f', 6).arg(psAddr->ptCoordinates.m_iLat >= 0 ? "N" : "S");
			rtn = QString("%1\n%4, %5 %6\n%7, %8").arg(psAddr->szStreetName).arg(psAddr->szCityName).arg(StateAbbreviationByName(psAddr->szStateName)).arg(psAddr->iZipCode,5).arg(strLong).arg(strLat);
		}
	}
	else
	{
		rtn = "";
	}
	return rtn;
}

char * ExtractField(char * pszText, int iStart, int iEnd)
{
	static char szField[1024];

	while (isspace(pszText[iStart - 1]) && iStart <= iEnd)
		iStart++;

	strncpy(szField, pszText + iStart - 1, iEnd - iStart + 1);
	szField[iEnd - iStart + 1] = 0;

	int iLength = strlen(szField);
	while (iLength && isspace(szField[iLength - 1]))
		iLength--;

	szField[iLength] = 0;

	return szField;
}

QString FormatName(const QString & strIn)
{
	QString strOut;
	unsigned int i;

	strOut = strIn;

	if (strIn.length() > 0)
		strOut[0] = strIn[0].upper();

	for (i = 1; i < strOut.length(); i++)
		strOut[i] = strIn[i - 1].isSpace() ? strIn[i].upper() : strIn[i].lower();

	strOut = strOut.stripWhiteSpace();

	return strOut;
}

QString FormatAbbreviation(const QString & strIn)
{
	return strIn.upper();
}

QString GrooveNetCommonDirectory()
{
	QString strRtn;

	strRtn = SHARE_DIRECTORY;
	return strRtn;
}

QString GetDataPath(const QString & strFilename, const QString & strURL, bool downloadAnyway)
{
	QString strRtn;

	QDir commonDir(GrooveNetCommonDirectory());

	strRtn = commonDir.absPath();

	if (strFilename != "")
	{
		strRtn = "";
		if (downloadAnyway || !commonDir.exists(strFilename)) {
			// create file from URL
			if (strURL != "") {
/*				Downloader cURL(strURL, QString("file://%1").arg(commonDir.absFilePath(strFilename)));
				if (cURL.Download(splash))
					strRtn = commonDir.absFilePath(strFilename);
*/				if (commonDir.exists(strFilename))
					strRtn = commonDir.absFilePath(strFilename);
			}
		} else
			strRtn = commonDir.absFilePath(strFilename);
	}

	if (strRtn.right(1) == '\\' || strRtn.right(1) == '/')
		strRtn.left(strRtn.length()-1);

	return strRtn;
}

bool LoadZipInformation(const QString & strFilename)
{
	FILE * hFile;
	char szLine[2048];

	hFile = fopen(strFilename, "rb");
	if (!hFile)
		return false;

	QString strLine;
	unsigned int lZip;
	QString strCity;
	long lStateCode;
	unsigned int lCountyCode;
	QString strState;
	fgets(szLine, sizeof(szLine), hFile);
	while (!feof(hFile))
	{
		qApp->processEvents();

		strLine = szLine;
		strLine = strLine.stripWhiteSpace();

		lZip = strLine.section('\t', 0, 0).toUInt();

		strCity = strLine.section('\t', 1, 1);

		lStateCode = strLine.section('\t', 2, 2).toLong();

		lCountyCode = strLine.section('\t', 3, 3).toUInt();

		strState = FormatName(StateNameByCode(lStateCode));

		g_mapCityToZip[strCity + "|" + strState].push_back(lZip);
		g_mapZipToCity.insert(std::pair<unsigned int, QString>(lZip, strCity));
		g_mapZipToStateName.insert(std::pair<unsigned int, QString>(lZip, strState));
		g_mapZipToCountyCode.insert(std::pair<unsigned int, unsigned int>(lZip, lCountyCode));

		fgets(szLine, sizeof(szLine), hFile);
	}

	fclose(hFile);
	qApp->processEvents();
	return true;
}

std::vector<unsigned int> ZipCodesByCityState(const QString & strCity, const QString & strState)
{
	QString strSearchString;
	QString strStateName;
	std::map<QString, std::vector<unsigned int> >::iterator iterSearch;

	if (StateNameByAbbreviation(strState) != "")
		strStateName = StateNameByAbbreviation(strState);
	else
		strStateName = strState;

	strSearchString = FormatName(strCity) + "|" + FormatName(strStateName);

	if ((iterSearch = g_mapCityToZip.find(strSearchString)) != g_mapCityToZip.end())
		return iterSearch->second;

	return std::vector<unsigned int>();
}

QString StateByZip(unsigned int iZip)
{
	if (g_mapZipToStateName.find(iZip) != g_mapZipToStateName.end())
		return g_mapZipToStateName[iZip];

	return "";
}

QString CityByZip(unsigned int iZip)
{
	if (g_mapZipToCity.find(iZip) != g_mapZipToCity.end())
		return g_mapZipToCity[iZip];

	return "";
}

int CountyCodeByZip(unsigned int iZip)
{
	int iCounty = 0;
	int iState = 0;

	if (g_mapZipToCountyCode.find(iZip) != g_mapZipToCountyCode.end())
		iCounty = g_mapZipToCountyCode[iZip];

	iState = StateCodeByName(StateByZip(iZip));

	return iState * 1000 + iCounty;
}

float PointSegmentDistance(const Coords & pt, const Coords & s0, const Coords & s1, float & fProgress)
{
	float vx, vy, w0x, w0y, c0, c1;
	Coords ptFlat(pt.Flatten()), s0Flat(s0.Flatten()), s1Flat(s1.Flatten());

	vx = s1Flat.m_iLong - s0Flat.m_iLong;
	vy = s1Flat.m_iLat - s0Flat.m_iLat;
	w0x = ptFlat.m_iLong - s0Flat.m_iLong;
	w0y = ptFlat.m_iLat - s0Flat.m_iLat;

	// compute perpendicular distance to segment
	if ((c0 = w0x*vx+w0y*vy) <= 0)
	{ // closest to endpoint s0
		fProgress = 0.f;
		return w0x*w0x+w0y*w0y;
	}
	else if ((c1 = vx*vx+vy*vy) <= c0)
	{ // closest to endpoint s1
		double w1x, w1y;
		fProgress = 1.f;
		w1x = ptFlat.m_iLong - s1Flat.m_iLong;
		w1y = ptFlat.m_iLat - s1Flat.m_iLat;
		return w1x*w1x+w1y*w1y;
	}
	else
	{ // closest to somewhere between s0 and s1
		fProgress = (float)(c0 / c1);
		return (w0x*w0x+w0y*w0y)-(c0*c0)/(c1);
	}
}


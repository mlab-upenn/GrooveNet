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

#include "RandomWalkModel.h"

#define RANDOMWALKMODEL_START_PARAM "START"
#define RANDOMWALKMODEL_START_PARAM_DEFAULT ""
#define RANDOMWALKMODEL_START_PARAM_DESC "START (address) -- The street address or intersection to start the vehicle's trip from."

RandomWalkModel::RandomWalkModel(const QString & strModelName)
: SimTripModel(strModelName), m_iCurrentRecord((unsigned)-1), m_iCRShapePoint((unsigned)-1), m_fCRProgress(0.f), m_bForwards(true)
{
}

RandomWalkModel::RandomWalkModel(const RandomWalkModel & copy)
: SimTripModel(copy), m_iCurrentRecord(copy.m_iCurrentRecord), m_iCRShapePoint(copy.m_iCRShapePoint), m_fCRProgress(copy.m_fCRProgress), m_bForwards(copy.m_bForwards), m_sStartAddress(copy.m_sStartAddress), m_strStartAddress(copy.m_strStartAddress)
{
}

RandomWalkModel::~RandomWalkModel()
{
}

RandomWalkModel & RandomWalkModel::operator = (const RandomWalkModel & copy)
{
	SimTripModel::operator = (copy);

	m_iCurrentRecord = copy.m_iCurrentRecord;
	m_iCRShapePoint = copy.m_iCRShapePoint;
	m_fCRProgress = copy.m_fCRProgress;
	m_bForwards = copy.m_bForwards;
	m_sStartAddress = copy.m_sStartAddress;
	m_strStartAddress = copy.m_strStartAddress;

	return *this;
}

unsigned int RandomWalkModel::GetCurrentRecord() const
{
	return m_iCurrentRecord;
}

bool RandomWalkModel::IsGoingForwards() const
{
	return m_bForwards;
}

unsigned short RandomWalkModel::GetCRShapePoint() const
{
	return m_iCRShapePoint;
}

float RandomWalkModel::GetCRProgress() const
{
	return m_fCRProgress;
}

Coords RandomWalkModel::GetCurrentPosition() const
{
	if (m_iCurrentRecord == (unsigned)-1 || m_iCRShapePoint == (unsigned)-1)
		return Coords();

	MapRecord * pRecord = g_pMapDB->GetRecord(m_iCurrentRecord);

	if ((signed)m_iCRShapePoint < pRecord->nShapePoints - 1)
	{
		Coords c0 = pRecord->pShapePoints[m_iCRShapePoint], c1 = pRecord->pShapePoints[m_iCRShapePoint + 1];

		return c0 + (c1 - c0) * m_fCRProgress;
	}
	else
		return pRecord->pShapePoints[pRecord->nShapePoints - 1];
}

short RandomWalkModel::GetCurrentDirection() const
{
	if (m_iCurrentRecord == (unsigned)-1 || m_iCRShapePoint == (unsigned)-1)
		return 0;

	MapRecord * pRecord = g_pMapDB->GetRecord(m_iCurrentRecord);

	if ((signed)m_iCRShapePoint < pRecord->nShapePoints - 1)
	{
		Coords c0 = pRecord->pShapePoints[m_iCRShapePoint], c1 = pRecord->pShapePoints[m_iCRShapePoint + 1];

		if (m_bForwards)
			return (short)round(atan2(c1.m_iLong - c0.m_iLong, c1.m_iLat - c0.m_iLat) * CENTIDEGREESPERRADIAN);
		else
			return (short)round(atan2(c0.m_iLong - c1.m_iLong, c0.m_iLat - c1.m_iLat) * CENTIDEGREESPERRADIAN);
	}
	else
		return 0;
}

bool RandomWalkModel::SetProgress(float & fTime, short iSpeed, unsigned int & iNextRecord)
{
	if (m_iCurrentRecord == (unsigned)-1 || m_iCRShapePoint == (unsigned)-1)
		return false;

	// find distance to next shape point
	MapRecord * pRecord = g_pMapDB->GetRecord(m_iCurrentRecord);
	float fRemaining = 0.f, fPart = 0.f, fDistance = (fTime * iSpeed) / SECSPERHOUR;

	if ((signed)m_iCRShapePoint < pRecord->nShapePoints - 1)
	{
		Coords c0 = pRecord->pShapePoints[m_iCRShapePoint], c1 = pRecord->pShapePoints[m_iCRShapePoint + 1];

		fPart = Distance(c0, c1);
		if (m_bForwards)
			fRemaining = fPart * (1 - m_fCRProgress);
		else
			fRemaining = fPart * m_fCRProgress;
	}

	if (fDistance < fRemaining && fPart > 0.f)
	{
		// we're still on the same part of the record
		if (m_bForwards)
			m_fCRProgress += fDistance / fPart;
		else
			m_fCRProgress -= fDistance / fPart;
		fTime = 0.f;
	}
	else
	{
		// we've passed the end of this shape point
		if (iSpeed > 0)
			fTime -= (fRemaining * SECSPERHOUR) / iSpeed;
		else
			fTime = 0.f;
		if (m_bForwards) {
			if ((signed)m_iCRShapePoint < pRecord->nShapePoints - 1)
			{
				m_iCRShapePoint++;
				m_fCRProgress = 0.f;
//				return SetProgress(fTime, iSpeed, iNextRecord);
				return true;
			}
			else
			{
				// check to see if we're stopped for a traffic light
				if (!g_pMapDB->UseTrafficLights() || CanCarGoThrough(g_pMapDB->GetVertex(pRecord->pVertices[pRecord->nVertices - 1]), m_iCurrentRecord))
				{
					// go to next record and return with the unused time
					iNextRecord = GetNextRecord(pRecord->pVertices[pRecord->nVertices - 1], m_iCurrentRecord);
					if (iNextRecord != (unsigned)-1)
					{
						m_iCurrentRecord = iNextRecord;
						if (g_pMapDB->GetRecord(m_iCurrentRecord)->pVertices[0] == pRecord->pVertices[pRecord->nVertices - 1]) {
							m_bForwards = true;
							m_iCRShapePoint = 0;
							m_fCRProgress = 0.f;
						} else {
							m_bForwards = false;
							m_iCRShapePoint = g_pMapDB->GetRecord(m_iCurrentRecord)->nShapePoints - 1;
							m_fCRProgress = 0.f;
						}
						return true;
					}
				}
				else
				{
					// we're stopped here!
					fTime = 0.f;
					m_fCRProgress = 1.f;
				}
			}
		} else {
			if (m_iCRShapePoint > 0)
			{
				m_iCRShapePoint--;
				m_fCRProgress = 1.f;
//				return SetProgress(fTime, iSpeed, iNextRecord);
				return true;
			}
			else
			{
				// check to see if we're stopped for a traffic light
				if (!g_pMapDB->UseTrafficLights() || CanCarGoThrough(g_pMapDB->GetVertex(pRecord->pVertices[0]), m_iCurrentRecord))
				{
					// go to next record
					iNextRecord = GetNextRecord(pRecord->pVertices[0], m_iCurrentRecord);
					if (iNextRecord != (unsigned)-1)
					{
						m_iCurrentRecord = iNextRecord;
						if (g_pMapDB->GetRecord(m_iCurrentRecord)->pVertices[0] == pRecord->pVertices[0]) {
							m_bForwards = true;
							m_iCRShapePoint = 0;
							m_fCRProgress = 0.f;
						} else {
							m_bForwards = false;
							m_iCRShapePoint = g_pMapDB->GetRecord(m_iCurrentRecord)->nShapePoints - 1;
							m_fCRProgress = 0.f;
						}
						return true;
					}
				}
				else
				{
					// we're stopped here!
					fTime = 0.f;
					m_fCRProgress = 0.f;
				}
			}
		}
	}
	return false;
}

unsigned int RandomWalkModel::GetNextRecord(unsigned int iVertex, unsigned int iPrevRecord)
{
	std::vector<unsigned int> vecRecords;
	if (g_pMapDB->GetNextPossibleRecords(vecRecords, iVertex, iPrevRecord))
		return ChooseRandomRecord(vecRecords, iPrevRecord);
	else
		return (unsigned)-1;
}

unsigned int RandomWalkModel::ChooseRandomRecord(const std::vector<unsigned int> & vecRecords, unsigned int iPrevRecord)
{
	unsigned int nChoose = vecRecords.size(), iRandom;

	if (nChoose == 0)
		return (unsigned)-1;

	if (vecRecords.back() == iPrevRecord && nChoose > 1) // don't turn around
		nChoose--;

	iRandom = RandUInt(0, nChoose);

	return vecRecords[iRandom];
}

int RandomWalkModel::Init(const std::map<QString, QString> & mapParams)
{
	if (SimTripModel::Init(mapParams))
		return 1;

	m_strStartAddress = GetParam(mapParams, RANDOMWALKMODEL_START_PARAM, RANDOMWALKMODEL_START_PARAM_DEFAULT);
	if (!StringToAddress(m_strStartAddress, &m_sStartAddress)) {
		m_sStartAddress.iRecord = m_sStartAddress.iVertex = (unsigned)-1;
		return 2;
	}

	return 0;
}

int RandomWalkModel::PreRun()
{
	if (SimTripModel::PreRun())
		return 1;

	g_pMapDB->AddressToPosition(&m_sStartAddress, m_iCurrentRecord, m_iCRShapePoint, m_fCRProgress);
	m_bForwards = true;

	return 0;
}

int RandomWalkModel::Save(std::map<QString, QString> & mapParams)
{
	QString strValue;
	if (SimTripModel::Save(mapParams))
		return 1;

	mapParams[RANDOMWALKMODEL_START_PARAM] = m_strStartAddress;

	return 0;
}

void RandomWalkModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	SimTripModel::GetParams(mapParams);

	mapParams[RANDOMWALKMODEL_START_PARAM].strValue = RANDOMWALKMODEL_START_PARAM_DEFAULT;
	mapParams[RANDOMWALKMODEL_START_PARAM].strDesc = RANDOMWALKMODEL_START_PARAM_DESC;
	mapParams[RANDOMWALKMODEL_START_PARAM].eType = ModelParameterTypeAddress;
}

int RandomWalkModel::Cleanup()
{
	if (SimTripModel::Cleanup())
		return 1;

	m_sStartAddress.iVertex = m_sStartAddress.iRecord = (unsigned)-1;
	return 0;
}

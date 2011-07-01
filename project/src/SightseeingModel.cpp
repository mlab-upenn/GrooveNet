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

#include "SightseeingModel.h"
#include "StringHelp.h"

#define SIGHTSEEINGMODEL_MAXDISTANCE_PARAM "MAXDISTANCE"
#define SIGHTSEEINGMODEL_MAXDISTANCE_PARAM_DEFAULT "1000"
#define SIGHTSEEINGMODEL_MAXDISTANCE_PARAM_DESC "MAXDISTANCE (meters) -- The maximum distance that the vehicle will travel from the starting point (as the crow flies) until it heads back to the starting point."

SightseeingModel::SightseeingModel(const QString & strModelName)
: RandomWalkModel(strModelName), m_iStartShapePoint((unsigned)-1), m_fStartProgress(1.f), m_fMaxDistance(1000.f)
{
}

SightseeingModel::SightseeingModel(const SightseeingModel & copy)
: RandomWalkModel(copy), m_iStartShapePoint(copy.m_iStartShapePoint), m_fStartProgress(copy.m_fStartProgress), m_fMaxDistance(copy.m_fMaxDistance), m_listPathRecords(copy.m_listPathRecords)
{
}

SightseeingModel::~SightseeingModel()
{
}

SightseeingModel & SightseeingModel::operator = (const SightseeingModel & copy)
{
	RandomWalkModel::operator = (copy);

	m_iStartShapePoint = copy.m_iStartShapePoint;
	m_fStartProgress = copy.m_fStartProgress;
	m_fMaxDistance = copy.m_fMaxDistance;
	m_listPathRecords = copy.m_listPathRecords;

	return *this;
}

bool SightseeingModel::SetProgress(float & fTime, short iSpeed, unsigned int & iNextRecord)
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
		if (m_listPathRecords.empty() && m_iCRShapePoint == m_iStartShapePoint)
		{
			if (m_bForwards)
				fRemaining = fPart * (m_fStartProgress - m_fCRProgress);
			else
				fRemaining = fPart * (m_fCRProgress - m_fStartProgress);
		}
		else
		{
			if (m_bForwards)
				fRemaining = fPart * (1 - m_fCRProgress);
			else
				fRemaining = fPart * m_fCRProgress;
		}
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
			if ((signed)m_iCRShapePoint < (m_listPathRecords.empty() ? (signed)m_iStartShapePoint : (pRecord->nShapePoints - 1)))
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
			if (m_iCRShapePoint > (m_listPathRecords.empty() ? m_iStartShapePoint : 0))
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

unsigned int SightseeingModel::GetNextRecord(unsigned int iVertex, unsigned int iPrevRecord)
{
	if (m_listPathRecords.empty())
		return (unsigned)-1;
	else {
		unsigned int iNewRecord = m_listPathRecords.back();
		if (iNewRecord == (unsigned)-1)
		{
			if (Distance(m_sStartAddress.ptCoordinates, GetCurrentPosition()) * METERSPERMILE < m_fMaxDistance)
				return RandomWalkModel::GetNextRecord(iVertex, iPrevRecord);
			else
			{
				bool bBackwardsStart = !m_bForwards, bBackwardsFinish;
				// end address
				if (m_sStartAddress.iRecord != (unsigned)-1 || m_sStartAddress.iVertex != (unsigned)-1)
				{
					m_listPathRecords = g_pMapDB->ShortestPath(iVertex, iPrevRecord, (unsigned)-1, 0.f, m_sStartAddress.iVertex, m_sStartAddress.iRecord, m_iStartShapePoint, m_fStartProgress, bBackwardsStart, bBackwardsFinish);
					if (!m_listPathRecords.empty())
						m_bForwards = !bBackwardsStart;
					m_listPathRecords.push_front((unsigned)-1); // for random walking
					iNewRecord = m_listPathRecords.back();
				}
				if (iNewRecord != (unsigned)-1)
				{
					m_listPathRecords.pop_back();
					return iNewRecord;
				}
				else
					return RandomWalkModel::GetNextRecord(iVertex, iPrevRecord);
			}
		}
		else
		{
			m_listPathRecords.pop_back();
			return iNewRecord;
		}
	}
}

int SightseeingModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	QStringList listWaypointStrings;
	QStringList::iterator iterWaypointString;
	if (RandomWalkModel::Init(mapParams))
		return 1;

	m_fMaxDistance = StringToNumber(GetParam(mapParams, SIGHTSEEINGMODEL_MAXDISTANCE_PARAM, SIGHTSEEINGMODEL_MAXDISTANCE_PARAM_DEFAULT));

	return 0;
}

int SightseeingModel::PreRun()
{
	if (RandomWalkModel::PreRun())
		return 1;

	m_iStartShapePoint = m_iCRShapePoint;
	m_fStartProgress = m_fCRProgress;
	m_listPathRecords.clear();
	m_listPathRecords.push_back((unsigned)-1);
	return 0;
}

int SightseeingModel::Save(std::map<QString, QString> & mapParams)
{
	QString strValue;
	std::list<Address>::iterator iterWaypoint;

	if (RandomWalkModel::Save(mapParams))
		return 1;

	mapParams[SIGHTSEEINGMODEL_MAXDISTANCE_PARAM] = QString("%1").arg(m_fMaxDistance);

	return 0;
}

void SightseeingModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	RandomWalkModel::GetParams(mapParams);

	mapParams[SIGHTSEEINGMODEL_MAXDISTANCE_PARAM].strValue = SIGHTSEEINGMODEL_MAXDISTANCE_PARAM_DEFAULT;
	mapParams[SIGHTSEEINGMODEL_MAXDISTANCE_PARAM].strDesc = SIGHTSEEINGMODEL_MAXDISTANCE_PARAM_DESC;
	mapParams[SIGHTSEEINGMODEL_MAXDISTANCE_PARAM].eType = ModelParameterTypeFloat;
}

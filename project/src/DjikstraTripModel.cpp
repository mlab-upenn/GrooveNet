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

#include "DjikstraTripModel.h"

#define DJIKSTRATRIPMODEL_FINISH_PARAM "FINISH"
#define DJIKSTRATRIPMODEL_FINISH_PARAM_DEFAULT ""
#define DJIKSTRATRIPMODEL_FINISH_PARAM_DESC "FINISH (address) -- The destination location for this vehicle. Leave blank to perform random walking."

#define DJIKSTRATRIPMODEL_WAYPOINTS_PARAM "WAYPOINTS"
#define DJIKSTRATRIPMODEL_WAYPOINTS_PARAM_DEFAULT ""
#define DJIKSTRATRIPMODEL_WAYPOINTS_PARAM_DESC "WAYPOINTS (addresses) -- A semicolon-delimited list of waypoints for this vehicle to visit on its trip."

DjikstraTripModel::DjikstraTripModel(const QString & strModelName)
: RandomWalkModel(strModelName), m_iEndShapePoint((unsigned)-1), m_fEndProgress(1.f)
{
}

DjikstraTripModel::DjikstraTripModel(const DjikstraTripModel & copy)
: RandomWalkModel(copy), m_iEndShapePoint(copy.m_iEndShapePoint), m_fEndProgress(copy.m_fEndProgress), m_sFinishAddress(copy.m_sFinishAddress), m_strFinishAddress(copy.m_strFinishAddress), m_strWaypoints(copy.m_strWaypoints)
{
	m_listPathRecords = copy.m_listPathRecords;
	m_listWaypoints = copy.m_listWaypoints;
}

DjikstraTripModel::~DjikstraTripModel()
{
}

DjikstraTripModel & DjikstraTripModel::operator = (const DjikstraTripModel & copy)
{
	RandomWalkModel::operator = (copy);

	m_iEndShapePoint = copy.m_iEndShapePoint;
	m_fEndProgress = copy.m_fEndProgress;
	m_listPathRecords = copy.m_listPathRecords;
	m_listWaypoints = copy.m_listWaypoints;
	m_sFinishAddress = copy.m_sFinishAddress;
	m_strFinishAddress = copy.m_strFinishAddress;
	m_strWaypoints = copy.m_strWaypoints;

	return *this;
}

bool DjikstraTripModel::SetProgress(float & fTime, short iSpeed, unsigned int & iNextRecord)
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
		if (m_listPathRecords.empty() && m_iCRShapePoint == m_iEndShapePoint)
		{
			if (m_bForwards)
				fRemaining = fPart * (m_fEndProgress - m_fCRProgress);
			else
				fRemaining = fPart * (m_fCRProgress - m_fEndProgress);
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
			if ((signed)m_iCRShapePoint < (m_listPathRecords.empty() ? (signed)m_iEndShapePoint : (pRecord->nShapePoints - 1)))
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
			if (m_iCRShapePoint > (m_listPathRecords.empty() ? m_iEndShapePoint : 0))
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

unsigned int DjikstraTripModel::GetNextRecord(unsigned int iVertex, unsigned int iPrevRecord)
{
	if (m_listPathRecords.empty())
		return (unsigned)-1;
	else {
		unsigned int iNewRecord = m_listPathRecords.back();
		if (iNewRecord == (unsigned)-1)
			return RandomWalkModel::GetNextRecord(iVertex, iPrevRecord);
		else {
			m_listPathRecords.pop_back();
			return iNewRecord;
		}
	}
}

int DjikstraTripModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	QStringList listWaypointStrings;
	QStringList::iterator iterWaypointString;
	if (RandomWalkModel::Init(mapParams))
		return 1;

	m_strFinishAddress = GetParam(mapParams, DJIKSTRATRIPMODEL_FINISH_PARAM, DJIKSTRATRIPMODEL_FINISH_PARAM_DEFAULT);
	if (!StringToAddress(m_strFinishAddress, &m_sFinishAddress))
		m_sFinishAddress.iRecord = m_sFinishAddress.iVertex = (unsigned)-1; // signal random walk

	m_strWaypoints = GetParam(mapParams, DJIKSTRATRIPMODEL_WAYPOINTS_PARAM, DJIKSTRATRIPMODEL_WAYPOINTS_PARAM_DEFAULT);

	listWaypointStrings = QStringList::split(';', m_strWaypoints, false);
	for (iterWaypointString = listWaypointStrings.begin(); iterWaypointString != listWaypointStrings.end(); ++iterWaypointString)
	{
		Address sWaypoint;
		if (StringToAddress(*iterWaypointString, &sWaypoint) && sWaypoint.iVertex != (unsigned)-1)
			m_listWaypoints.push_back(sWaypoint);
	}

	return 0;
}

int DjikstraTripModel::PreRun()
{
	std::list<unsigned int> listPath;
	std::list<Address>::iterator iterWaypoint;
	bool bBackwardsStart = false, bBackwardsFinish = false, bStartSet = false;
	Address * pStart = &m_sStartAddress;

	if (RandomWalkModel::PreRun())
		return 1;

	m_listPathRecords.clear();
	bBackwardsStart = !m_bForwards;

	for (iterWaypoint = m_listWaypoints.begin(); iterWaypoint != m_listWaypoints.end(); ++iterWaypoint)
	{
		listPath = g_pMapDB->ShortestPath(pStart, &(*iterWaypoint), bBackwardsStart, bBackwardsFinish);
		if (!listPath.empty()) {
			m_listPathRecords.splice(m_listPathRecords.begin(), listPath);
			if (!bStartSet) {
				m_bForwards = !bBackwardsStart;
				bStartSet = true;
			}
			pStart = &(*iterWaypoint);
			bBackwardsStart = bBackwardsFinish;
		}
	}

	// end address
	if (m_sFinishAddress.iRecord == (unsigned)-1 && m_sFinishAddress.iVertex == (unsigned)-1)
		m_listPathRecords.push_front((unsigned)-1);
	else {
		listPath = g_pMapDB->ShortestPath(pStart, &m_sFinishAddress, bBackwardsStart, bBackwardsFinish);
		if (listPath.empty())
			m_listPathRecords.push_front((unsigned)-1);
		else {
			unsigned int iLastRecord = (unsigned)-1;
			m_listPathRecords.splice(m_listPathRecords.begin(), listPath);
			if (!bStartSet) {
				m_bForwards = !bBackwardsStart;
				bStartSet = true;
			}
			g_pMapDB->AddressToPosition(&m_sFinishAddress, iLastRecord, m_iEndShapePoint, m_fEndProgress);
		}
	}

	if (!m_listPathRecords.empty())
		m_listPathRecords.pop_back();
	return bStartSet ? 0 : 2;
}

int DjikstraTripModel::Save(std::map<QString, QString> & mapParams)
{
	if (RandomWalkModel::Save(mapParams))
		return 1;

	mapParams[DJIKSTRATRIPMODEL_FINISH_PARAM] = m_strFinishAddress;
	mapParams[DJIKSTRATRIPMODEL_WAYPOINTS_PARAM] = m_strWaypoints;

	return 0;
}

void DjikstraTripModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	RandomWalkModel::GetParams(mapParams);

	mapParams[DJIKSTRATRIPMODEL_FINISH_PARAM].strValue = DJIKSTRATRIPMODEL_FINISH_PARAM_DEFAULT;
	mapParams[DJIKSTRATRIPMODEL_FINISH_PARAM].strDesc = DJIKSTRATRIPMODEL_FINISH_PARAM_DESC;
	mapParams[DJIKSTRATRIPMODEL_FINISH_PARAM].eType = ModelParameterTypeAddress;

	mapParams[DJIKSTRATRIPMODEL_WAYPOINTS_PARAM].strValue = DJIKSTRATRIPMODEL_WAYPOINTS_PARAM_DEFAULT;
	mapParams[DJIKSTRATRIPMODEL_WAYPOINTS_PARAM].strDesc = DJIKSTRATRIPMODEL_WAYPOINTS_PARAM_DESC;
	mapParams[DJIKSTRATRIPMODEL_WAYPOINTS_PARAM].eType = ModelParameterTypeAddresses;
}

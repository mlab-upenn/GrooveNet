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

#include "StreetSpeedModel.h"

#include "Simulator.h"
#include "MapDB.h"

#define STREETSPEEDMODEL_TRIP_PARAM "TRIP"
#define STREETSPEEDMODEL_TRIP_PARAM_DEFAULT "NULL"
#define STREETSPEEDMODEL_TRIP_PARAM_DESC "TRIP (model) -- The vehicle's associated trip-planning/navigation model."

StreetSpeedModel::StreetSpeedModel(const QString & strModelName)
: SimMobilityModel(strModelName), m_pTripModel(NULL)
{
}

StreetSpeedModel::StreetSpeedModel(const StreetSpeedModel & copy)
: SimMobilityModel(copy), m_pTripModel(copy.m_pTripModel)
{
}

StreetSpeedModel::~StreetSpeedModel()
{
}

StreetSpeedModel & StreetSpeedModel::operator = (const StreetSpeedModel & copy)
{
	SimMobilityModel::operator = (copy);

	m_pTripModel = copy.m_pTripModel;

	return *this;
}

int StreetSpeedModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	Model * pModel;

	if (SimMobilityModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, STREETSPEEDMODEL_TRIP_PARAM, STREETSPEEDMODEL_TRIP_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 2;
	m_pTripModel = (SimTripModel *)pModel;
	return 0;
}

int StreetSpeedModel::Save(std::map<QString, QString> & mapParams)
{
	if (SimMobilityModel::Save(mapParams))
		return 1;

	mapParams[STREETSPEEDMODEL_TRIP_PARAM] = m_pTripModel == NULL ? "NULL" : m_pTripModel->GetModelName();

	return 0;
}

void StreetSpeedModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	SimMobilityModel::GetParams(mapParams);

	mapParams[STREETSPEEDMODEL_TRIP_PARAM].strValue = STREETSPEEDMODEL_TRIP_PARAM_DEFAULT;
	mapParams[STREETSPEEDMODEL_TRIP_PARAM].strDesc = STREETSPEEDMODEL_TRIP_PARAM_DESC;
	mapParams[STREETSPEEDMODEL_TRIP_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[STREETSPEEDMODEL_TRIP_PARAM].strAuxData = SIMTRIPMODEL_NAME;
}

bool StreetSpeedModel::GetInitialConditions(unsigned int & iRecord, Coords & ptPosition, short & iSpeed, short & iHeading, unsigned char & iLane)
{
	// get initial position from trip model
	if (m_pTripModel == NULL)
	{
		iRecord = (unsigned)-1;
		ptPosition.m_iLong = ptPosition.m_iLat = 0;
		iHeading = 0;
		iSpeed = 0;
		iLane = 0;
	}
	else
	{
		iRecord = m_pTripModel->GetCurrentRecord();
		ptPosition = m_pTripModel->GetCurrentPosition();
		iHeading = m_pTripModel->GetCurrentDirection();
		iSpeed = ChooseSpeed(iRecord);
		iLane = 0;
	}
	return iRecord != (unsigned)-1;
}

bool StreetSpeedModel::DoIteration(float fElapsed, unsigned int & iRecord, Coords & ptPosition, short & iSpeed, short & iHeading, unsigned char & iLane)
{
	bool bMore = false;
	bool bActive = false;
	unsigned int iOldRecord;

	if (m_pTripModel == NULL)
		return bActive;

	iOldRecord = m_pTripModel->GetCurrentRecord();
	do
	{
		if (m_bMultilane)
			SwitchLanes(iRecord, m_pTripModel->GetCRShapePoint(), m_pTripModel->GetCRProgress(), iSpeed, m_pTripModel->IsGoingForwards(), iLane);
		bMore = m_pTripModel->SetProgress(fElapsed, iSpeed, iRecord);
		bActive = (iRecord != (unsigned)-1);
		iRecord = m_pTripModel->GetCurrentRecord();
		if (iRecord != iOldRecord)
		{
			iSpeed = ChooseSpeed(iRecord);
			if (m_bMultilane)
				AssignLane(iOldRecord, iRecord, iLane);
			iOldRecord = iRecord;
		}
	} while (bMore);

	ptPosition = m_pTripModel->GetCurrentPosition();
	iHeading = m_pTripModel->GetCurrentDirection();
	return bActive;
}

short StreetSpeedModel::ChooseSpeed(unsigned int iRecord) const
{
	if (iRecord == (unsigned)-1)
		return 0;
	else
		return (short)round(SECSPERHOUR / TimeFactor(g_pMapDB->GetRecord(iRecord)));
}

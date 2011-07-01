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

#include "FixedMobilityModel.h"

#include "Simulator.h"
#include "MapDB.h"

#define FIXEDMOBILITYMODEL_TRIP_PARAM "TRIP"
#define FIXEDMOBILITYMODEL_TRIP_PARAM_DEFAULT "NULL"
#define FIXEDMOBILITYMODEL_TRIP_PARAM_DESC "TRIP (model) -- The vehicle's associated trip-planning/navigation model."

FixedMobilityModel::FixedMobilityModel(const QString & strModelName)
: SimMobilityModel(strModelName), m_pTripModel(NULL)
{
}

FixedMobilityModel::FixedMobilityModel(const FixedMobilityModel & copy)
: SimMobilityModel(copy), m_pTripModel(copy.m_pTripModel)
{
}

FixedMobilityModel::~FixedMobilityModel()
{
}

FixedMobilityModel & FixedMobilityModel::operator = (const FixedMobilityModel & copy)
{
	SimMobilityModel::operator = (copy);

	m_pTripModel = copy.m_pTripModel;

	return *this;
}

int FixedMobilityModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	Model * pModel;

	if (SimMobilityModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, FIXEDMOBILITYMODEL_TRIP_PARAM, FIXEDMOBILITYMODEL_TRIP_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 2;
	m_pTripModel = (SimTripModel *)pModel;
	return 0;
}

int FixedMobilityModel::Save(std::map<QString, QString> & mapParams)
{
	if (SimMobilityModel::Save(mapParams))
		return 1;

	mapParams[FIXEDMOBILITYMODEL_TRIP_PARAM] = m_pTripModel == NULL ? "NULL" : m_pTripModel->GetModelName();

	return 0;
}

void FixedMobilityModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	SimMobilityModel::GetParams(mapParams);

	mapParams[FIXEDMOBILITYMODEL_TRIP_PARAM].strValue = FIXEDMOBILITYMODEL_TRIP_PARAM_DEFAULT;
	mapParams[FIXEDMOBILITYMODEL_TRIP_PARAM].strDesc = FIXEDMOBILITYMODEL_TRIP_PARAM_DESC;
	mapParams[FIXEDMOBILITYMODEL_TRIP_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[FIXEDMOBILITYMODEL_TRIP_PARAM].strAuxData = SIMTRIPMODEL_NAME;
}

bool FixedMobilityModel::GetInitialConditions(unsigned int & iRecord, Coords & ptPosition, short & iSpeed, short & iHeading, unsigned char & iLane)
{
	// get initial position from trip model
	if (m_pTripModel == NULL)
	{
		iRecord = (unsigned)-1;
		ptPosition.m_iLong = ptPosition.m_iLat = 0;
		iHeading = 0;
		iLane = 0;
	}
	else
	{
		iRecord = m_pTripModel->GetCurrentRecord();
		ptPosition = m_pTripModel->GetCurrentPosition();
		iHeading = m_pTripModel->GetCurrentDirection();
		iLane = 0;
	}
	iSpeed = 0;
	return iRecord != (unsigned)-1;
}

bool FixedMobilityModel::DoIteration(float fElapsed __attribute__((unused)) , unsigned int & iRecord, Coords & ptPosition, short & iSpeed, short & iHeading, unsigned char & iLane)
{
	bool bActive = false;

	if (m_pTripModel == NULL)
		return bActive;

	bActive = (iRecord != (unsigned)-1);
	iRecord = m_pTripModel->GetCurrentRecord();
	ptPosition = m_pTripModel->GetCurrentPosition();
	iSpeed = 0;
	iHeading = m_pTripModel->GetCurrentDirection();
	iLane = 0;
	return bActive;
}

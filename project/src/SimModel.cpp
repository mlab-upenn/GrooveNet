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

#include "SimModel.h"
#include "CarRegistry.h"
#include "StringHelp.h"
#include "Simulator.h"
#include "Network.h"
#include "Logger.h"

#define SIMMOBILITYMODEL_MULTILANE_PARAM "MULTILANE"
#define SIMMOBILITYMODEL_MULTILANE_PARAM_DEFAULT "N"
#define SIMMOBILITYMODEL_MULTILANE_PARAM_DESC "MULTILANE (Yes/No) -- Specify \"Yes\" if you want this vehicle to use multiple lanes, \"No\" otherwise."

SimMobilityModel::SimMobilityModel(const QString & strModelName)
: Model(strModelName), m_ipCar(0), m_bMultilane(false)
{
}

SimMobilityModel::SimMobilityModel(const SimMobilityModel & copy)
: Model(copy), m_ipCar(copy.m_ipCar), m_bMultilane(copy.m_bMultilane)
{
}

SimMobilityModel::~SimMobilityModel()
{
}

SimMobilityModel & SimMobilityModel::operator =(const SimMobilityModel & copy)
{
	Model::operator =(copy);

	m_ipCar = copy.m_ipCar;
	m_bMultilane = copy.m_bMultilane;
	return *this;
}

int SimMobilityModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;

	if (Model::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, SIMMOBILITYMODEL_MULTILANE_PARAM, SIMMOBILITYMODEL_MULTILANE_PARAM_DEFAULT);
	m_bMultilane = StringToBoolean(strValue);

	return 0;
}

int SimMobilityModel::Save(std::map<QString, QString> & mapParams)
{
	if (Model::Save(mapParams))
		return 1;

	mapParams[SIMMOBILITYMODEL_MULTILANE_PARAM] = BooleanToString(m_bMultilane);
	return 0;
}

void SimMobilityModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	Model::GetParams(mapParams);

	mapParams[SIMMOBILITYMODEL_MULTILANE_PARAM].strValue = SIMMOBILITYMODEL_MULTILANE_PARAM_DEFAULT;
	mapParams[SIMMOBILITYMODEL_MULTILANE_PARAM].strDesc = SIMMOBILITYMODEL_MULTILANE_PARAM_DESC;
	mapParams[SIMMOBILITYMODEL_MULTILANE_PARAM].eType = (ModelParameterType)(ModelParameterTypeYesNo | ModelParameterFixed);
}

void SimMobilityModel::AssignLane(unsigned int iOldRecord, unsigned int iRecord, unsigned char & iLane)
{
	if (iRecord == (unsigned)-1)
		return;

	unsigned char iLanes = NumberOfLanes(g_pMapDB->GetRecord(iRecord));
	
	// check to see if we're on a different road or different number of lanes
	if (iOldRecord == (unsigned)-1 || NumberOfLanes(g_pMapDB->GetRecord(iOldRecord)) != iLanes || !IsSameRoad(g_pMapDB->GetRecord(iRecord), g_pMapDB->GetRecord(iOldRecord)))
		iLane = iLanes > 0 ? RandUInt(0, iLanes) : 0;
}

void SimMobilityModel::SwitchLanes(unsigned int iRecord, unsigned short iShapePoint, float fProgress, short iSpeed, bool bForwards, unsigned char & iLane)
{
	if (iRecord == (unsigned)-1)
		return;

	unsigned char i, iNewLane = iLane, iLanes = NumberOfLanes(g_pMapDB->GetRecord(iRecord));
	// see what lanes to check (only adjacent lanes)
	unsigned char iStart = iLane > 0 ? iLane - 1 : 0, iEnd = iLane < iLanes - 1 ? iLane + 1 : iLanes - 1;
	unsigned int iCar;
	std::vector<CarModel *> vecCars;
	std::vector<CarModel *> vecLanes(iLanes, NULL);

	if (iLanes == 0)
	{
		iLane = 0;
		return;
	}

	g_pCarRegistry->acquireLock();
	if (g_pCarRegistry->GetCarsOnRecord(iRecord, bForwards, vecCars))
	{
		for (iCar = 0; iCar < vecCars.size(); iCar++)
		{
			if (vecCars[iCar]->GetIPAddress() != m_ipCar && vecCars[iCar]->GetLane() < vecLanes.size())
			{
				if (vecLanes[vecCars[iCar]->GetLane()] == NULL)
					vecLanes[vecCars[iCar]->GetLane()] = vecCars[iCar];
				else
				{
					unsigned int iShapePointCurrent = vecLanes[vecCars[iCar]->GetLane()]->GetCRShapePoint(), iShapePointNew = vecCars[iCar]->GetCRShapePoint();
					float fProgressCurrent = vecLanes[vecCars[iCar]->GetLane()]->GetCRProgress(), fProgressNew = vecCars[iCar]->GetCRProgress();
					if (bForwards && (iShapePointCurrent > iShapePointNew || iShapePointCurrent == iShapePointNew && fProgressCurrent > fProgressNew))
						vecLanes[vecCars[iCar]->GetLane()] = vecCars[iCar];
					else if(!bForwards && (iShapePointCurrent < iShapePointNew || iShapePointCurrent == iShapePointNew && fProgressCurrent < fProgressNew))
						vecLanes[vecCars[iCar]->GetLane()] = vecCars[iCar];
				}
			}
		}
	}

	if (vecLanes[iLane] != NULL && vecLanes[iLane]->GetCurrentSpeed() < iSpeed)
	{
		bool bCatchup = vecLanes[iLane] != NULL && vecLanes[iLane]->GetCurrentSpeed() < iSpeed;
		short iMaxSpeed = vecLanes[iLane] != NULL ? vecLanes[iLane]->GetCurrentSpeed() : SHRT_MAX;
		float fTime, fMaxTime = bCatchup ? DistanceAlongRecord(g_pMapDB->GetRecord(iRecord), iShapePoint, fProgress, vecLanes[iLane]->GetCRShapePoint(), vecLanes[iLane]->GetCRProgress()) / (iSpeed - vecLanes[iLane]->GetCurrentSpeed()) : 0.f;
		for (i = iStart; i <= iEnd; i++)
		{
			if (vecLanes[i] == NULL || vecLanes[i]->GetCurrentSpeed() >= iSpeed)
				bCatchup = false;
			if (bCatchup)
			{
				fTime = DistanceAlongRecord(g_pMapDB->GetRecord(iRecord), iShapePoint, fProgress, vecLanes[iLane]->GetCRShapePoint(), vecLanes[iLane]->GetCRProgress()) / (iSpeed - vecLanes[iLane]->GetCurrentSpeed());
				if (fTime > fMaxTime)
				{
					fMaxTime = fTime;
					iNewLane = i;
				}
			}
			else
			{
				if (vecLanes[i] == NULL && iMaxSpeed < SHRT_MAX)
				{
					iMaxSpeed = SHRT_MAX;
					iNewLane = i;
				}
				else if (vecLanes[i] != NULL && vecLanes[i]->GetCurrentSpeed() > iMaxSpeed)
				{
					iMaxSpeed = vecLanes[i]->GetCurrentSpeed();
					iNewLane = i;
				}
			}
		}
	}
	g_pCarRegistry->releaseLock();
	iLane = iNewLane;
}


SimTripModel::SimTripModel(const QString & strModelName)
: Model(strModelName), m_ipCar(0)
{
}

SimTripModel::SimTripModel(const SimTripModel & copy)
: Model(copy), m_ipCar(copy.m_ipCar)
{
}

SimTripModel::~SimTripModel()
{
}

SimTripModel & SimTripModel::operator =(const SimTripModel & copy)
{
	Model::operator =(copy);

	m_ipCar = copy.m_ipCar;
	return *this;
}

void SimTripModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	Model::GetParams(mapParams);
}


#define SIMMODEL_STARTTIME_PARAM "START"
#define SIMMODEL_STARTTIME_PARAM_DEFAULT "0"
#define SIMMODEL_STARTTIME_PARAM_DESC "START (seconds) -- The number of seconds from the beginning of the simulation until this vehicle starts moving."
#define SIMMODEL_MOBILITY_PARAM "MOBILITY"
#define SIMMODEL_MOBILITY_PARAM_DEFAULT "NULL"
#define SIMMODEL_MOBILITY_PARAM_DESC "MOBILITY (model) -- The mobility model for this vehicle."
#define SIMMODEL_TRIP_PARAM "TRIP"
#define SIMMODEL_TRIP_PARAM_DEFAULT "NULL"
#define SIMMODEL_TRIP_PARAM_DESC "TRIP (model) -- The trip-planning/navigation model for this vehicle."

SimModel::SimModel(const QString & strModelName)
: CarModel(strModelName), m_pMobilityModel(NULL), m_pTripModel(NULL), m_bActive(false), m_tStartTime(timeval0)
{
}

SimModel::SimModel(const SimModel & copy)
: CarModel(copy), m_pMobilityModel(copy.m_pMobilityModel), m_pTripModel(copy.m_pTripModel),
m_bActive(copy.m_bActive), m_tStartTime(copy.m_tStartTime)
{
}

SimModel::~SimModel()
{
}

SimModel & SimModel::operator =(const SimModel & copy)
{
	CarModel::operator =(copy);

	m_pMobilityModel = copy.m_pMobilityModel;
	m_pTripModel = copy.m_pTripModel;

	m_bActive = copy.m_bActive;
	m_tStartTime = copy.m_tStartTime;

	return *this;
}

QString SimModel::GetCarListColumnText(CarListColumn eColumn) const
{
	switch (eColumn)
	{
	case CarListColumnType:
		return "Local/Simulated";
	default:
		return CarModel::GetCarListColumnText(eColumn);
	}
}

int SimModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	Model * pModel;

	if (CarModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, SIMMODEL_STARTTIME_PARAM, SIMMODEL_STARTTIME_PARAM_DEFAULT);
	m_tStartTime = MakeTime(ValidateNumber(StringToNumber(strValue), 0, HUGE_VAL));

	strValue = GetParam(mapParams, SIMMODEL_MOBILITY_PARAM, SIMMODEL_MOBILITY_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 2;
	m_pMobilityModel = (SimMobilityModel *)pModel;
	if (m_pMobilityModel != NULL)
		m_pMobilityModel->SetCar(m_ipCar);

	strValue = GetParam(mapParams, SIMMODEL_TRIP_PARAM, SIMMODEL_TRIP_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 3;
	m_pTripModel = (SimTripModel *)pModel;
	if (m_pTripModel != NULL)
		m_pTripModel->SetCar(m_ipCar);

	return 0;
}

int SimModel::PreRun()
{
	if (CarModel::PreRun())
		return 1;

	m_bActive = false;
	if (m_pMobilityModel == NULL)
	{
		m_iCurrentRecord = (unsigned)-1;
		m_ptPosition.Set(0, 0);
		m_iSpeed = 0;
		m_iHeading = 0;
		m_iLane = 0;
	}
	else
		m_pMobilityModel->GetInitialConditions(m_iCurrentRecord, m_ptPosition, m_iSpeed, m_iHeading, m_iLane);

	if (m_pTripModel == NULL)
	{
		m_iCurrentRecord = (unsigned)-1;
		m_bForwards = true;
		m_iCRShapePoint = (unsigned)-1;
		m_fCRProgress = 0.f;
	}
	else
	{
		m_iCurrentRecord = m_pTripModel->GetCurrentRecord();
		m_bForwards = m_pTripModel->IsGoingForwards();
		m_iCRShapePoint = m_pTripModel->GetCRShapePoint();
		m_fCRProgress = m_pTripModel->GetCRProgress();
	}

	m_tTimestamp = timeval0;
	return 0;
}

int SimModel::ProcessEvent(SimEvent & event)
{
	if (CarModel::ProcessEvent(event))
		return 1;

	switch (event.GetEventID())
	{
	case EVENT_CARMODEL_UPDATE:
	{
		if (!m_pMobilityModel || !m_pTripModel)
			return 2;

		if (event.GetTimestamp() > g_pSimulator->m_tStart + m_tStartTime)
		{
			//It looks like this is where the default
			//location update packets are being created -MH
			Packet msg;

			if (m_tTimestamp == timeval0)
				m_tTimestamp = g_pSimulator->m_tStart + m_tStartTime;

			m_bActive = m_pMobilityModel->DoIteration(ToFloat(event.GetTimestamp() - m_tTimestamp), m_iCurrentRecord, m_ptPosition, m_iSpeed, m_iHeading, m_iLane);

			m_tTimestamp = event.GetTimestamp();
			m_iCurrentRecord = m_pTripModel->GetCurrentRecord();
			m_bForwards = m_pTripModel->IsGoingForwards();
			m_iCRShapePoint = m_pTripModel->GetCRShapePoint();
			m_fCRProgress = m_pTripModel->GetCRProgress();
	
			// send message to clients
			CreateMessage(&msg);
			if (m_bLogThisCar)
				g_pLogger->WriteMessage(LOGFILE_MESSAGES, &msg);
			if (m_pCommModel != NULL);
				m_pCommModel->TransmitMessage(&msg);
		}
		else
			m_bActive = false;
		return 0;
	}
	default:
		return 0;
	}
}

/*
int SimModel::Iteration(struct timeval tCurrent)
{
	Message msg;

	if (!m_pMobilityModel || !m_pTripModel)
		return 2;

	if (m_tLastIteration > g_pSimulator->m_tStart + m_tStartTime)
		m_bActive = m_pMobilityModel->DoIteration(ToFloat(tCurrent - m_tLastIteration), m_iCurrentRecord, m_ptPosition, m_iSpeed, m_iHeading);
	else
		m_bActive = false;

	if (CarModel::Iteration(tCurrent))
		return 1;

	if (m_bActive)
	{
		m_tTimestamp = m_tLastIteration;
		m_iCurrentRecord = m_pTripModel->GetCurrentRecord();
		m_bForwards = m_pTripModel->IsGoingForwards();
		m_iCRShapePoint = m_pTripModel->GetCRShapePoint();
		m_fCRProgress = m_pTripModel->GetCRProgress();

		// send message to clients
		CreateMessage(msg);
		if (m_bLogThisCar)
			g_pLogger->WriteMessage(LOGFILE_MESSAGES, &msg);
		if (m_pCommModel != NULL)
			m_pCommModel->TransmitMessage(msg);
	}

	return 0;
}
*/
int SimModel::PostRun()
{
	if (CarModel::PostRun())
		return 1;

	m_bActive = false;

	return 0;
}

int SimModel::Save(std::map<QString, QString> & mapParams)
{
	if (CarModel::Save(mapParams))
		return 1;

	mapParams[SIMMODEL_STARTTIME_PARAM] = QString("%1").arg(ToDouble(m_tStartTime));
	mapParams[SIMMODEL_MOBILITY_PARAM] = m_pMobilityModel == NULL ? "NULL" : m_pMobilityModel->GetModelName();
	mapParams[SIMMODEL_TRIP_PARAM] = m_pTripModel == NULL ? "NULL" : m_pTripModel->GetModelName();

	return 0;
}

void SimModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	CarModel::GetParams(mapParams);

	mapParams[SIMMODEL_STARTTIME_PARAM].strValue = SIMMODEL_STARTTIME_PARAM_DEFAULT;
	mapParams[SIMMODEL_STARTTIME_PARAM].strDesc = SIMMODEL_STARTTIME_PARAM_DESC;
	mapParams[SIMMODEL_STARTTIME_PARAM].eType = ModelParameterTypeFloat;
	mapParams[SIMMODEL_STARTTIME_PARAM].strAuxData = QString("%1:").arg(0);

	mapParams[SIMMODEL_MOBILITY_PARAM].strValue = SIMMODEL_MOBILITY_PARAM_DEFAULT;
	mapParams[SIMMODEL_MOBILITY_PARAM].strDesc = SIMMODEL_MOBILITY_PARAM_DESC;
	mapParams[SIMMODEL_MOBILITY_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[SIMMODEL_MOBILITY_PARAM].strAuxData = SIMMOBILITYMODEL_NAME;

	mapParams[SIMMODEL_TRIP_PARAM].strValue = SIMMODEL_TRIP_PARAM_DEFAULT;
	mapParams[SIMMODEL_TRIP_PARAM].strDesc = SIMMODEL_TRIP_PARAM_DESC;
	mapParams[SIMMODEL_TRIP_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[SIMMODEL_TRIP_PARAM].strAuxData = SIMTRIPMODEL_NAME;
}

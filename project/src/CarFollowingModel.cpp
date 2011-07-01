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

#include "CarFollowingModel.h"
#include "Simulator.h"
#include "CarRegistry.h"

#define CARFOLLOWINGMODEL_TRIP_PARAM "TRIP"
#define CARFOLLOWINGMODEL_TRIP_PARAM_DEFAULT "NULL"
#define CARFOLLOWINGMODEL_TRIP_PARAM_DESC "TRIP (model) -- The vehicle's associated trip-planning/navigation model."
#define CARFOLLOWINGMODEL_LEADER_PARAM "LEADER"
#define CARFOLLOWINGMODEL_LEADER_PARAM_DEFAULT "NULL"
#define CARFOLLOWINGMODEL_LEADER_PARAM_DESC "LEADER (model) -- The mobility model to use if this vehicle isn't following any other vehicle."

CarFollowingModel::CarFollowingModel(const QString & strModelName)
: SimMobilityModel(strModelName), m_pTripModel(NULL), m_pLeaderModel(NULL), m_iDesiredSpeed(0)
{
}

CarFollowingModel::CarFollowingModel(const CarFollowingModel & copy)
: SimMobilityModel(copy), m_pTripModel(copy.m_pTripModel), m_pLeaderModel(copy.m_pLeaderModel), m_iDesiredSpeed(copy.m_iDesiredSpeed)
{
}

CarFollowingModel::~CarFollowingModel()
{
}

CarFollowingModel & CarFollowingModel::operator = (const CarFollowingModel & copy)
{
	SimMobilityModel::operator =(copy);

	m_pTripModel = copy.m_pTripModel;
	m_pLeaderModel = copy.m_pLeaderModel;
	m_iDesiredSpeed = copy.m_iDesiredSpeed;
	return *this;
}

int CarFollowingModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	Model * pModel;

	if (SimMobilityModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, CARFOLLOWINGMODEL_TRIP_PARAM, CARFOLLOWINGMODEL_TRIP_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 2;
	m_pTripModel = (SimTripModel *)pModel;

	strValue = GetParam(mapParams, CARFOLLOWINGMODEL_LEADER_PARAM, CARFOLLOWINGMODEL_LEADER_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 3;
	m_pLeaderModel = (SimMobilityModel *)pModel;

	return 0;
}

int CarFollowingModel::Save(std::map<QString, QString> & mapParams)
{
	if (SimMobilityModel::Save(mapParams))
		return 1;

	mapParams[CARFOLLOWINGMODEL_TRIP_PARAM] = m_pTripModel == NULL ? "NULL" : m_pTripModel->GetModelName();
	mapParams[CARFOLLOWINGMODEL_LEADER_PARAM] = m_pLeaderModel == NULL ? "NULL" : m_pLeaderModel->GetModelName();

	return 0;
}

void CarFollowingModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	SimMobilityModel::GetParams(mapParams);

	mapParams[CARFOLLOWINGMODEL_TRIP_PARAM].strValue = CARFOLLOWINGMODEL_TRIP_PARAM_DEFAULT;
	mapParams[CARFOLLOWINGMODEL_TRIP_PARAM].strDesc = CARFOLLOWINGMODEL_TRIP_PARAM_DESC;
	mapParams[CARFOLLOWINGMODEL_TRIP_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[CARFOLLOWINGMODEL_TRIP_PARAM].strAuxData = SIMTRIPMODEL_NAME;

	mapParams[CARFOLLOWINGMODEL_LEADER_PARAM].strValue = CARFOLLOWINGMODEL_LEADER_PARAM_DEFAULT;
	mapParams[CARFOLLOWINGMODEL_LEADER_PARAM].strDesc = CARFOLLOWINGMODEL_LEADER_PARAM_DESC;
	mapParams[CARFOLLOWINGMODEL_LEADER_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[CARFOLLOWINGMODEL_LEADER_PARAM].strAuxData = SIMMOBILITYMODEL_NAME;
}

bool CarFollowingModel::GetInitialConditions(unsigned int & iRecord, Coords & ptPosition, short & iSpeed, short & iHeading, unsigned char & iLane)
{
	bool bSuccess = false;
	if (m_pLeaderModel != NULL)
	{
		bSuccess = m_pLeaderModel->GetInitialConditions(iRecord, ptPosition, iSpeed, iHeading, iLane);
		m_iDesiredSpeed = iSpeed;
	} else
		m_iDesiredSpeed = 0;
	return bSuccess;
}

bool CarFollowingModel::DoIteration(float fElapsed, unsigned int & iRecord, Coords & ptPosition, short & iSpeed, short & iHeading, unsigned char & iLane)
{
	bool bMore = false;
	bool bActive = false;
	unsigned int iOldRecord;

	if (m_pTripModel == NULL || m_pLeaderModel == NULL)
		return bActive;

	do
	{
		if (m_bMultilane)
			SwitchLanes(iRecord, m_pTripModel->GetCRShapePoint(), m_pTripModel->GetCRProgress(), iSpeed, m_pTripModel->IsGoingForwards(), iLane);
		iOldRecord = iRecord;
		bMore = m_pTripModel->SetProgress(fElapsed, iSpeed, iRecord);
		bActive = (iRecord != (unsigned)-1);
		iRecord = m_pTripModel->GetCurrentRecord();
		if (iRecord != iOldRecord)
		{
			m_iDesiredSpeed = m_pLeaderModel->ChooseSpeed(iRecord);
			if (m_bMultilane)
				AssignLane(iOldRecord, iRecord, iLane);
		}
		iSpeed = GetMaximumSpeed(iLane, m_iDesiredSpeed);
	} while (bMore);

	ptPosition = m_pTripModel->GetCurrentPosition();
	iHeading = m_pTripModel->GetCurrentDirection();
	return bActive;
}

short CarFollowingModel::ChooseSpeed(unsigned int iRecord) const
{
	if (m_pLeaderModel)
		return m_pLeaderModel->ChooseSpeed(iRecord);
	else
		return 0;
}

short CarFollowingModel::GetMaximumSpeed(unsigned char iLane, short iDesiredSpeed) const
{
//	std::map<in_addr_t, CarModel *> * pCarRegistry;
	std::map<in_addr_t, CarModel *>::iterator iterCar;
//	CarModel * pCar;
	std::vector<CarModel *> vecCars;
//	std::map<in_addr_t, Message> * pKnownVehicles;
	std::map<in_addr_t, SafetyPacket>::iterator iterKnownVehicle;
	unsigned int iThisRecord = m_pTripModel->GetCurrentRecord()/*, iRecord*/;
	bool bThisForwards = m_pTripModel->IsGoingForwards();
	unsigned short iThisShapePoint = m_pTripModel->GetCRShapePoint(), iShapePoint;
	float fThisProgress = m_pTripModel->GetCRProgress(), fProgress;
	short iSpeed;
	unsigned int i;

	g_pCarRegistry->acquireLock();
	g_pCarRegistry->GetCarsOnRecord(iThisRecord, bThisForwards, vecCars);

	if (bThisForwards)
	{
		for (i = 0; i < vecCars.size(); i++)
		{
			iShapePoint = vecCars[i]->GetCRShapePoint();
			fProgress = vecCars[i]->GetCRProgress();
			iSpeed = vecCars[i]->GetCurrentSpeed();
			if (vecCars[i]->IsActive() && (!m_bMultilane || vecCars[i]->GetLane() == iLane) && iDesiredSpeed > iSpeed && ((iShapePoint == iThisShapePoint && fProgress > fThisProgress)))
				iDesiredSpeed = iSpeed;
		}
	}
	else
	{
		for (i = 0; i < vecCars.size(); i++)
		{
			iShapePoint = vecCars[i]->GetCRShapePoint();
			fProgress = vecCars[i]->GetCRProgress();
			iSpeed = vecCars[i]->GetCurrentSpeed();
			if (vecCars[i]->IsActive() && (!m_bMultilane || vecCars[i]->GetLane() == iLane) && iDesiredSpeed > iSpeed && ((iShapePoint == iThisShapePoint && fProgress < fThisProgress)))
				iDesiredSpeed = iSpeed;
		}
	}
	g_pCarRegistry->releaseLock();
	return iDesiredSpeed;
}

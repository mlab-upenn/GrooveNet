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

#include "SimUnconstrainedModel.h"
#include "CarRegistry.h"
#include "StringHelp.h"
#include "Simulator.h"
#include "Network.h"
#include "Logger.h"

SimUnconstrainedMobilityModel::SimUnconstrainedMobilityModel(const QString & strModelName)
: Model(strModelName), m_ipCar(0)
{
}

SimUnconstrainedMobilityModel::SimUnconstrainedMobilityModel(const SimUnconstrainedMobilityModel & copy)
: Model(copy), m_ipCar(copy.m_ipCar)
{
}

SimUnconstrainedMobilityModel::~SimUnconstrainedMobilityModel()
{
}

SimUnconstrainedMobilityModel & SimUnconstrainedMobilityModel::operator =(const SimUnconstrainedMobilityModel & copy)
{
	Model::operator =(copy);

	m_ipCar = copy.m_ipCar;
	return *this;
}

void SimUnconstrainedMobilityModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	Model::GetParams(mapParams);
}


#define SIMUNCONSTRAINEDMODEL_STARTTIME_PARAM "START"
#define SIMUNCONSTRAINEDMODEL_STARTTIME_PARAM_DEFAULT "0"
#define SIMUNCONSTRAINEDMODEL_STARTTIME_PARAM_DESC "START (seconds) -- The number of seconds from the beginning of the simulation until this vehicle starts moving."
#define SIMUNCONSTRAINEDMODEL_MOBILITY_PARAM "MOBILITY"
#define SIMUNCONSTRAINEDMODEL_MOBILITY_PARAM_DEFAULT "NULL"
#define SIMUNCONSTRAINEDMODEL_MOBILITY_PARAM_DESC "MOBILITY (model) -- The mobility model for this vehicle."

SimUnconstrainedModel::SimUnconstrainedModel(const QString & strModelName)
: CarModel(strModelName), m_pMobilityModel(NULL), m_bActive(false), m_tStartTime(timeval0)
{
}

SimUnconstrainedModel::SimUnconstrainedModel(const SimUnconstrainedModel & copy)
: CarModel(copy), m_pMobilityModel(copy.m_pMobilityModel), m_bActive(copy.m_bActive), m_tStartTime(copy.m_tStartTime)
{
}

SimUnconstrainedModel::~SimUnconstrainedModel()
{
}

SimUnconstrainedModel & SimUnconstrainedModel::operator =(const SimUnconstrainedModel & copy)
{
	CarModel::operator =(copy);

	m_pMobilityModel = copy.m_pMobilityModel;

	m_bActive = copy.m_bActive;
	m_tStartTime = copy.m_tStartTime;

	return *this;
}

QString SimUnconstrainedModel::GetCarListColumnText(CarListColumn eColumn) const
{
	switch (eColumn)
	{
	case CarListColumnType:
		return "Local/Simulated";
	default:
		return CarModel::GetCarListColumnText(eColumn);
	}
}

int SimUnconstrainedModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	Model * pModel;

	if (CarModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, SIMUNCONSTRAINEDMODEL_STARTTIME_PARAM, SIMUNCONSTRAINEDMODEL_STARTTIME_PARAM_DEFAULT);
	m_tStartTime = MakeTime(ValidateNumber(StringToNumber(strValue), 0, HUGE_VAL));

	strValue = GetParam(mapParams, SIMUNCONSTRAINEDMODEL_MOBILITY_PARAM, SIMUNCONSTRAINEDMODEL_MOBILITY_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 2;
	m_pMobilityModel = (SimUnconstrainedMobilityModel *)pModel;
	if (m_pMobilityModel != NULL)
		m_pMobilityModel->SetCar(m_ipCar);

	return 0;
}

int SimUnconstrainedModel::PreRun()
{
	if (CarModel::PreRun())
		return 1;

	m_bActive = false;
	m_iCurrentRecord = (unsigned)-1;
	m_bForwards = true;
	m_iCRShapePoint = (unsigned)-1;
	m_fCRProgress = 0.f;
	m_iLane = 0;
	if (m_pMobilityModel == NULL)
	{
		m_ptPosition.Set(0, 0);
		m_iSpeed = 0;
		m_iHeading = 0;
	}
	else
		m_pMobilityModel->GetInitialConditions(m_ptPosition, m_iSpeed, m_iHeading);

	m_tTimestamp = timeval0;
	return 0;
}

int SimUnconstrainedModel::ProcessEvent(SimEvent & event)
{
	if (CarModel::ProcessEvent(event))
		return 1;

	switch (event.GetEventID())
	{
	case EVENT_CARMODEL_UPDATE:
	{
		if (!m_pMobilityModel)
			return 2;

		if (event.GetTimestamp() > g_pSimulator->m_tStart + m_tStartTime)
		{
			SafetyPacket msg;

			if (m_tTimestamp == timeval0)
				m_tTimestamp = g_pSimulator->m_tStart + m_tStartTime;

			m_bActive = m_pMobilityModel->DoIteration(ToFloat(event.GetTimestamp() - m_tTimestamp), m_ptPosition, m_iSpeed, m_iHeading);

			m_tTimestamp = event.GetTimestamp();
	
			// send message to clients
			CreateMessage(&msg);
			if (m_bLogThisCar)
				g_pLogger->WriteMessage(LOGFILE_MESSAGES, &msg);
			if (m_pCommModel != NULL)
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

int SimUnconstrainedModel::PostRun()
{
	if (CarModel::PostRun())
		return 1;

	m_bActive = false;

	return 0;
}

int SimUnconstrainedModel::Save(std::map<QString, QString> & mapParams)
{
	if (CarModel::Save(mapParams))
		return 1;

	mapParams[SIMUNCONSTRAINEDMODEL_STARTTIME_PARAM] = QString("%1").arg(ToDouble(m_tStartTime));
	mapParams[SIMUNCONSTRAINEDMODEL_MOBILITY_PARAM] = m_pMobilityModel == NULL ? "NULL" : m_pMobilityModel->GetModelName();

	return 0;
}

void SimUnconstrainedModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	CarModel::GetParams(mapParams);

	mapParams[SIMUNCONSTRAINEDMODEL_STARTTIME_PARAM].strValue = SIMUNCONSTRAINEDMODEL_STARTTIME_PARAM_DEFAULT;
	mapParams[SIMUNCONSTRAINEDMODEL_STARTTIME_PARAM].strDesc = SIMUNCONSTRAINEDMODEL_STARTTIME_PARAM_DESC;
	mapParams[SIMUNCONSTRAINEDMODEL_STARTTIME_PARAM].eType = ModelParameterTypeFloat;
	mapParams[SIMUNCONSTRAINEDMODEL_STARTTIME_PARAM].strAuxData = QString("%1:").arg(0);

	mapParams[SIMUNCONSTRAINEDMODEL_MOBILITY_PARAM].strValue = SIMUNCONSTRAINEDMODEL_MOBILITY_PARAM_DEFAULT;
	mapParams[SIMUNCONSTRAINEDMODEL_MOBILITY_PARAM].strDesc = SIMUNCONSTRAINEDMODEL_MOBILITY_PARAM_DESC;
	mapParams[SIMUNCONSTRAINEDMODEL_MOBILITY_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[SIMUNCONSTRAINEDMODEL_MOBILITY_PARAM].strAuxData = SIMUNCONSTRAINEDMOBILITYMODEL_NAME;
}

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

#include "GPSModel.h"
#include "CarRegistry.h"
#include "StringHelp.h"
#include "NMEAProcessor.h"
#include "Network.h"
#include "Logger.h"

GPSProcessor::GPSProcessor()
: QThread(), m_bCancelled(false)
{
}

GPSProcessor::~GPSProcessor()
{
}

void GPSProcessor::start(Priority priority)
{
	while (!wait(MAX_DEADLOCK))
		terminate();
	m_bCancelled = false;
	m_sGPSData.tTimestamp = timeval0;
	m_sGPSData.ptPosition.Set(0, 0);
	m_sGPSData.iSpeed = 0;
	m_sGPSData.iHeading = 0;
	QThread::start(priority);
}

bool GPSProcessor::wait(unsigned long time)
{
	m_bCancelled = true;
	return QThread::wait(time);
}

void GPSProcessor::GetParams(std::map<QString, ModelParameter> & mapParams __attribute__ ((unused)) )
{
}


#define GPSMODEL_PROTOCOL_PARAM "PROTOCOL"
#define GPSMODEL_PROTOCOL_PARAM_DEFAULT NMEAPROCESSOR_NAME
#define GPSMODEL_PROTOCOL_PARAM_DESC "PROTOCOL (GPS protocol) -- The communication protocol to use to interpret GPS data. As of now, only NMEA 0183 is supported."


#define GPSMODEL_TIMEOUT_SECS 5
#define GPSMODEL_TIMEOUT_USECS 0

GPSModel::GPSModel(const QString & strModelName)
: CarModel(strModelName), m_pGPS(NULL)
{
}

GPSModel::GPSModel(const GPSModel & copy)
: CarModel(copy), m_pGPS(copy.m_pGPS)
{
}

GPSModel::~GPSModel()
{
}

GPSModel & GPSModel::operator = (const GPSModel & copy)
{
	CarModel::operator =(copy);

	m_pGPS = copy.m_pGPS;
	return *this;
}

QString GPSModel::GetCarListColumnText(CarListColumn eColumn) const
{
	switch (eColumn)
	{
	case CarListColumnType:
		return "Local/GPS";
	default:
		return CarModel::GetCarListColumnText(eColumn);
	}
}

bool GPSModel::IsActive() const
{
	return m_tTimestamp + MakeTime(GPSMODEL_TIMEOUT_SECS, GPSMODEL_TIMEOUT_USECS) >= GetLastEventTime();
}

int GPSModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	GPSProcessor * pGPS = NULL;
 
	if (CarModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, GPSMODEL_PROTOCOL_PARAM, GPSMODEL_PROTOCOL_PARAM_DEFAULT);

	// compare against names of known GPS processors, and initialize
	if (strValue.compare(NMEAPROCESSOR_NAME) == 0)
		pGPS = (GPSProcessor *)new NMEAProcessor();

	if (pGPS != NULL && !pGPS->Init(mapParams)) {
		delete pGPS;
		pGPS = NULL;
	}
	m_pGPS = pGPS;

	return 0;
}

int GPSModel::PreRun()
{
	if (CarModel::PreRun())
		return 1;

	if (m_pGPS != NULL)
		m_pGPS->start();

	m_tTimestamp = timeval0;
	return 0;
}

int GPSModel::ProcessEvent(SimEvent & event)
{
	bool bUpdated = false;

	if (CarModel::ProcessEvent(event))
		return 1;

	switch (event.GetEventID())
	{
	case EVENT_CARMODEL_UPDATE:
	{
		if (m_pGPS != NULL) {
			GPSData * pData = m_pGPS->acquireLock();
			if (m_tTimestamp < pData->tTimestamp)
			{
				m_tTimestamp = pData->tTimestamp;
				m_ptPosition = pData->ptPosition;
				m_iSpeed = pData->iSpeed;
				m_iHeading = pData->iHeading;
				bUpdated = true;
			}
			m_pGPS->releaseLock();
		}

		// update position in map database if necessary
		if (bUpdated) {
			g_pMapDB->CoordsToRecord(m_ptPosition, m_iCurrentRecord, m_iCRShapePoint, m_fCRProgress);
			m_bForwards = m_iCurrentRecord == (unsigned)-1 || IsVehicleGoingForwards(m_iCRShapePoint, m_iHeading, g_pMapDB->GetRecord(m_iCurrentRecord));
		}
		if (IsActive())
		{
			Packet msg; //Changed to generic packet -MH
			// send message to clients
			CreateMessage(&msg);
			if (m_bLogThisCar)
				g_pLogger->WriteMessage(LOGFILE_MESSAGES, &msg);
			if (m_pCommModel)
				m_pCommModel->TransmitMessage(&msg);
		}
		return 0;
	}
	default:
		return 0;
	}
}

/*
int GPSModel::Iteration(struct timeval tCurrent)
{
	GPSData * pData;
	Message msg;
	bool bUpdated = false;

	if (CarModel::Iteration(tCurrent))
		return 1;

	if (m_pGPS != NULL) {
		pData = m_pGPS->acquireLock();
		if (m_tTimestamp < pData->tTimestamp)
		{
			m_tTimestamp = pData->tTimestamp;
			m_ptPosition = pData->ptPosition;
			m_iSpeed = pData->iSpeed;
			m_iHeading = pData->iHeading;
			bUpdated = true;
		}
		m_pGPS->releaseLock();
	}

	// update position in map database if necessary
	if (bUpdated) {
		g_pMapDB->CoordsToRecord(m_ptPosition, m_iCurrentRecord, m_iCRShapePoint, m_fCRProgress);
		m_bForwards = m_iCurrentRecord == (unsigned)-1 || IsVehicleGoingForwards(m_iCRShapePoint, m_iHeading, g_pMapDB->GetRecord(m_iCurrentRecord));
	}

	if (IsActive())
	{
		// send message to clients
		CreateMessage(msg);
		if (m_bLogThisCar)
			g_pLogger->WriteMessage(LOGFILE_MESSAGES, &msg);
		if (m_pCommModel)
			m_pCommModel->TransmitMessage(msg);
	}

	return 0;
}

int GPSModel::PostIteration()
{
	std::vector<CarModel *> vecNeighbors;
	NeighborMessage msgNeighbors;

	if (CarModel::PostIteration())
		return 1;

	if (IsActive())
	{
		g_pCarRegistry->GetCarsInRange(this, vecNeighbors);

		// populate neighbor list
		msgNeighbors.tMessage = m_tTimestamp;
		msgNeighbors.ipCar = m_ipCar;
		msgNeighbors.iNeighbors = vecNeighbors.size();
		g_pLogger->WriteMessage(LOGFILE_NEIGHBORS, &msgNeighbors);
	}

	return 0;
}
*/
int GPSModel::PostRun()
{
	if (CarModel::PostRun())
		return 1;

	if (m_pGPS != NULL) {
		while (!m_pGPS->wait(MAX_DEADLOCK))
			m_pGPS->terminate();
	}

	return 0;
}

int GPSModel::Save(std::map<QString, QString> & mapParams)
{
	if (CarModel::Save(mapParams))
		return 1;

	if (m_pGPS != NULL) {
		mapParams[GPSMODEL_PROTOCOL_PARAM] = m_pGPS->GetGPSProcessorType();
		m_pGPS->Save(mapParams);
	}

	return 0;
}

void GPSModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	CarModel::GetParams(mapParams);

	mapParams[GPSMODEL_PROTOCOL_PARAM].strValue = GPSMODEL_PROTOCOL_PARAM_DEFAULT;
	mapParams[GPSMODEL_PROTOCOL_PARAM].strDesc = GPSMODEL_PROTOCOL_PARAM_DESC;
	mapParams[GPSMODEL_PROTOCOL_PARAM].eType = (ModelParameterType)(ModelParameterTypeGPS | ModelParameterFixed);
	mapParams[GPSMODEL_PROTOCOL_PARAM].strAuxData = NMEAPROCESSOR_NAME;

	NMEAProcessor::GetParams(mapParams);
}

int GPSModel::Cleanup()
{
	if (CarModel::Cleanup())
		return 1;

	if (m_pGPS != NULL) {
		delete m_pGPS;
		m_pGPS = NULL;
	}

	return 0;
}

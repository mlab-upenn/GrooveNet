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

#include "Global.h"
#include "CarModel.h"
#include "CarRegistry.h"
#include "Simulator.h"
#include "Logger.h"
#include "Network.h"
#include "InfrastructureNodeRegistry.h"
#include "CollisionPhysModel.h"
#include "QMessageList.h"
//Added by qt3to4:
#include <Q3PointArray>

#include "StringHelp.h"

CarLinkModel::CarLinkModel(const QString & strModelName)
: Model(strModelName), m_ipCar(0)
{
}

CarLinkModel::CarLinkModel(const CarLinkModel & copy)
: Model(copy), m_ipCar(copy.m_ipCar), m_mapPackets(copy.m_mapPackets)
{
}

CarLinkModel::~CarLinkModel()
{
}

CarLinkModel & CarLinkModel::operator =(const CarLinkModel & copy)
{
	Model::operator =(copy);

	m_ipCar = copy.m_ipCar;
	m_mapPackets = copy.m_mapPackets;
	return *this;
}

int CarLinkModel::PreRun()
{
	if (Model::PreRun())
		return 1;
	m_mapPackets.clear();
	return 0;
}

bool CarLinkModel::DoUpdate(struct timeval tCurrent)
{
	std::map<PacketSequence, struct timeval>::iterator iterPacket = m_mapPackets.begin(), iterPacketTemp;
	while (iterPacket != m_mapPackets.end())
	{
		if (iterPacket->second <= tCurrent)
		{
			iterPacketTemp = iterPacket;
			++iterPacket;
			m_mapPackets.erase(iterPacketTemp);
		} else
			++iterPacket;
	}
	return true;
}

void CarLinkModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	Model::GetParams(mapParams);
}

bool CarLinkModel::AddReceivedPacket(const Packet * packet)
{
	return m_mapPackets.insert(std::pair<PacketSequence, struct timeval>(packet->m_ID.srcID, packet->GetTimestamp() + packet->GetLifetime())).second;
}

CarPhysModel::CarPhysModel(const QString & strModelName)
: Model(strModelName), m_ipCar(0)
{
}

CarPhysModel::CarPhysModel(const CarPhysModel & copy)
: Model(copy), m_ipCar(copy.m_ipCar)
{
}

CarPhysModel::~CarPhysModel()
{
}

CarPhysModel & CarPhysModel::operator = (const CarPhysModel & copy)
{
	Model::operator =(copy);

	m_ipCar = copy.m_ipCar;
	return *this;
}

void CarPhysModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	Model::GetParams(mapParams);
}


CarCommModel::CarCommModel(const QString & strModelName)
: Model(strModelName), m_ipCar(0)
{
}

CarCommModel::CarCommModel(const CarCommModel & copy)
: Model(copy), m_ipCar(copy.m_ipCar), m_mapMsgHistory(copy.m_mapMsgHistory)
{
}

CarCommModel::~CarCommModel()
{
}

CarCommModel & CarCommModel::operator =(const CarCommModel & copy)
{
	Model::operator =(copy);

	m_ipCar = copy.m_ipCar;
	m_mapMsgHistory = copy.m_mapMsgHistory;
	return *this;
}

int CarCommModel::PreRun()
{
	if (Model::PreRun())
		return 1;

	m_mapMsgHistory.clear();
	return 0;
}

bool CarCommModel::DoUpdate(struct timeval tCurrent __attribute__((unused)) )
{
	std::map<PacketSequence, MessageHistory>::iterator iterMsgHistory;
	unsigned int i;
	for (iterMsgHistory = m_mapMsgHistory.begin(); iterMsgHistory != m_mapMsgHistory.end(); ++iterMsgHistory)
	{
		sort(iterMsgHistory->second.vecMessages.begin(), iterMsgHistory->second.vecMessages.end(), ComparePacketsRX);
		for (i = 0; i < iterMsgHistory->second.vecMessages.size(); i++)
		{
			if (iterMsgHistory->second.vecMessages[i].m_tRX < iterMsgHistory->second.tRelevant)
				break;
		}
		iterMsgHistory->second.vecMessages.resize(i);
	}
	return true;
}

void CarCommModel::AddMessageToHistory(const SafetyPacket & msg)
{
	unsigned int i, iOldest = 0;
	bool bFound = false;

	std::map<PacketSequence, MessageHistory>::iterator iterMsgHistory = m_mapMsgHistory.find(msg.m_ID.srcID);
	if (iterMsgHistory == m_mapMsgHistory.end())
	{
		MessageHistory msgHistory;
		msgHistory.tRelevant = timeval0;
		iterMsgHistory = m_mapMsgHistory.insert(std::pair<PacketSequence, MessageHistory>(msg.m_ID.srcID, msgHistory)).first;
	}
	if (msg.m_tRX >= iterMsgHistory->second.tRelevant)
		iterMsgHistory->second.vecMessages.push_back(msg);

	// add sender to TX history
	for (i = 0; i < iterMsgHistory->second.vecTXHistory.size(); i++)
	{
		if (iterMsgHistory->second.vecTXHistory[i].first == msg.m_ipTX)
		{
			if (iterMsgHistory->second.vecTXHistory[i].second < msg.m_tRX)
				iterMsgHistory->second.vecTXHistory[i].second = msg.m_tRX;
			bFound = true;
			break;
		}
		if (iterMsgHistory->second.vecTXHistory[i].second < iterMsgHistory->second.vecTXHistory[iOldest].second)
			iOldest = i;
	}

	if (!bFound) // insert, (optionally) purge
	{
		if (iterMsgHistory->second.vecTXHistory.size() < TXHISTORYSIZE)
			iterMsgHistory->second.vecTXHistory.push_back(std::pair<in_addr_t, struct timeval>(msg.m_ipTX, msg.m_tRX));
		else
		{
			iterMsgHistory->second.vecTXHistory[iOldest].first = msg.m_ipTX;
			iterMsgHistory->second.vecTXHistory[iOldest].second = msg.m_tRX;
		}
	}
}

void CarCommModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	Model::GetParams(mapParams);
}


#define CARMODEL_PARAM_DELAY "DELAY"
#define CARMODEL_PARAM_DELAY_DEFAULT "0.2"
#define CARMODEL_PARAM_DELAY_DESC "DELAY (seconds) -- The time interval between updating the vehicle's models."
#define CARMODEL_PARAM_CARIP_DEFAULT IPAddressToString(::GetIPAddress())
#define CARMODEL_PARAM_CARIP_DESC "ID (IP address) -- The IP address uniquely identifying this vehicle."
#define CARMODEL_PARAM_LOGCAR_DEFAULT "Y"
#define CARMODEL_PARAM_LOGCAR_DESC "DOLOG (Yes/No) -- Specify \"Yes\" if you want to log this car's statistics, \"No\" otherwise."
#define CARMODEL_LINK_PARAM "LINK"
#define CARMODEL_LINK_PARAM_DEFAULT "NULL"
#define CARMODEL_LINK_PARAM_DESC "LINK (model) -- The vehicle's link layer model."
#define CARMODEL_PHYS_PARAM "PHYS"
#define CARMODEL_PHYS_PARAM_DEFAULT "NULL"
#define CARMODEL_PHYS_PARAM_DESC "PHYS (model) -- The vehicle's physical layer model."
#define CARMODEL_COMM_PARAM "COMM"
#define CARMODEL_COMM_PARAM_DEFAULT "NULL"
#define CARMODEL_COMM_PARAM_DESC "COMM (model) -- The vehicle's communication/rebroadcast model."
#define CARMODEL_TRACKSPEED_PARAM "TRACKSPEED"
#define CARMODEL_TRACKSPEED_PARAM_DEFAULT "N"
#define CARMODEL_TRACKSPEED_PARAM_DESC "TRACKSPEED (Yes/No) -- Specify \"Yes\" if you want this vehicle to record its speed history and send it to other vehicles, \"No\" otherwise."

CarModel::CarModel(const QString & strModelName)
: Model(strModelName), m_pLinkModel(NULL), m_pPhysModel(NULL), m_pCommModel(NULL), m_tDelay(timeval0), m_tTimestamp(timeval0), m_ipCar(0), m_ipOwner(CARMODEL_IPOWNER_LOCAL), m_bLogThisCar(false), m_iSpeed(0), m_iHeading(0), m_iCurrentRecord((unsigned)-1), m_bForwards(true), m_iCRShapePoint((unsigned)-1), m_fCRProgress(0.f), m_iLane(0), m_iMapObjectID(-1), m_iNextSeqNumber(0), m_iNextRXSeqNumber(0), m_bTrackSpeed(false)
{
}

CarModel::CarModel(const CarModel & copy)
: Model(copy), m_pLinkModel(copy.m_pLinkModel), m_pPhysModel(copy.m_pPhysModel), m_pCommModel(copy.m_pCommModel), m_tDelay(copy.m_tDelay), m_tTimestamp(copy.m_tTimestamp), m_ipCar(copy.m_ipCar), m_ipOwner(copy.m_ipOwner), m_bLogThisCar(copy.m_bLogThisCar), m_ptPosition(copy.m_ptPosition), m_iSpeed(copy.m_iSpeed), m_iHeading(copy.m_iHeading), m_iCurrentRecord(copy.m_iCurrentRecord), m_bForwards(copy.m_bForwards), m_iCRShapePoint(copy.m_iCRShapePoint), m_fCRProgress(copy.m_fCRProgress), m_iLane(copy.m_iLane), m_iMapObjectID(copy.m_iMapObjectID), m_mapKnownVehicles(copy.m_mapKnownVehicles), m_iNextSeqNumber(copy.m_iNextSeqNumber), m_iNextRXSeqNumber(copy.m_iNextRXSeqNumber), m_msgNeighbors(copy.m_msgNeighbors), m_bTrackSpeed(copy.m_bTrackSpeed), m_mapTracks(copy.m_mapTracks)
{
}

CarModel::~CarModel()
{
}

CarModel & CarModel::operator =(const CarModel & copy)
{
	Model::operator =(copy);

	m_pLinkModel = copy.m_pLinkModel;
	m_pPhysModel = copy.m_pPhysModel;
	m_pCommModel = copy.m_pCommModel;

	m_tDelay = copy.m_tDelay;
	m_tTimestamp = copy.m_tTimestamp;
	m_ipCar = copy.m_ipCar;
	m_ipOwner = copy.m_ipOwner;
	m_bLogThisCar = copy.m_bLogThisCar;
	m_ptPosition = copy.m_ptPosition;
	m_iSpeed = copy.m_iSpeed;
	m_iHeading = copy.m_iHeading;
	m_iCurrentRecord = copy.m_iCurrentRecord;
	m_bForwards = copy.m_bForwards;
	m_iCRShapePoint = copy.m_iCRShapePoint;
	m_fCRProgress = copy.m_fCRProgress;
	m_iLane = copy.m_iLane;
	m_iMapObjectID = copy.m_iMapObjectID;
	m_mapKnownVehicles = copy.m_mapKnownVehicles;
	m_iNextSeqNumber = copy.m_iNextSeqNumber;
	m_iNextRXSeqNumber = copy.m_iNextRXSeqNumber;

	m_msgNeighbors = copy.m_msgNeighbors;

	m_bTrackSpeed = copy.m_bTrackSpeed;
	m_mapTracks = copy.m_mapTracks;

	return *this;
}

int CarModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	MapCarObject * pObject;
	Model * pModel;

	if (Model::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, CARMODEL_PARAM_DELAY, CARMODEL_PARAM_DELAY_DEFAULT);
	m_tDelay = MakeTime(ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL));

	strValue = GetParam(mapParams, CARMODEL_PARAM_CARIP, CARMODEL_PARAM_CARIP_DEFAULT);
	if (!StringToIPAddress(strValue, m_ipCar))
		return 2;

	strValue = GetParam(mapParams, CARMODEL_PARAM_LOGCAR, CARMODEL_PARAM_LOGCAR_DEFAULT);
	m_bLogThisCar = StringToBoolean(strValue);

	strValue = GetParam(mapParams, CARMODEL_LINK_PARAM, CARMODEL_LINK_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 5;
	m_pLinkModel = (CarLinkModel *)pModel;
	if (m_pLinkModel != NULL)
		m_pLinkModel->SetCar(m_ipCar);
	strValue = GetParam(mapParams, CARMODEL_PHYS_PARAM, CARMODEL_PHYS_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 6;
	m_pPhysModel = (CarPhysModel *)pModel;
	if (m_pPhysModel != NULL)
		m_pPhysModel->SetCar(m_ipCar);

	strValue = GetParam(mapParams, CARMODEL_COMM_PARAM, CARMODEL_COMM_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 4;
	m_pCommModel = (CarCommModel *)pModel;
	if (m_pCommModel != NULL)
		m_pCommModel->SetCar(m_ipCar);

	strValue = GetParam(mapParams, CARMODEL_TRACKSPEED_PARAM, CARMODEL_TRACKSPEED_PARAM_DEFAULT);
	m_bTrackSpeed = StringToBoolean(strValue);

	pObject = new MapCarObject(this);

//HACK: set color based on ip group
	in_addr_t temp = pObject->m_pCar->GetIPAddress();
	temp = temp>>8 & 0xFF;
	if(temp == 100) //red ones
	{
		QColor * redColor = new QColor(0xFF, 0x00, 0x00);
		pObject->SetColor(redColor->rgb());
	}
	else if(temp == 200) //black ones
	{
		QColor * redColor = new QColor(0x00, 0x00, 0x00);
		pObject->SetColor(redColor->rgb());
	}
	else //the others
	{
		pObject->SetColor(g_pMapObjects->GetColor());
	}
//end HACK

	m_iMapObjectID = g_pMapObjects->add(pObject);
	if (m_iMapObjectID == -1)
		delete pObject;

	g_pCarRegistry->addCar(this);

	m_iNextSeqNumber = 0;
	m_iNextRXSeqNumber = 0;



	return 0;
}

int CarModel::PreRun()
{
	if (Model::PreRun())
		return 1;

	m_tTimestamp = timeval0;
	m_ptPosition.Set(0, 0);
	m_iSpeed = 0;
	m_iHeading = 0;
	m_iCurrentRecord = (unsigned)-1;
	m_bForwards = true;
	m_iCRShapePoint = (unsigned)-1;
	m_fCRProgress = 0.f;
	m_iLane = 0;

	m_mapKnownVehicles.clear();
	m_mapTracks.clear();

	m_msgNeighbors.ipCar = m_ipCar;
	m_msgNeighbors.iAccumulatedCollisions = 0;
	m_msgNeighbors.iAccumulatedMessages = 0;

	g_pSimulator->m_EventQueue.AddEvent(SimEvent(g_pSimulator->m_tCurrent, EVENT_PRIORITY_HIGHEST, m_strModelName, m_strModelName, EVENT_CARMODEL_UPDATE));

	return 0;
}

int CarModel::ProcessEvent(SimEvent & event)
{
	if (Model::ProcessEvent(event))
		return 1;

	switch (event.GetEventID())
	{
	case EVENT_CARMODEL_UPDATE:
	{
		if (m_pPhysModel != NULL)
			m_pPhysModel->DoUpdate(event.GetTimestamp());
		if (m_pLinkModel != NULL)
			m_pLinkModel->DoUpdate(event.GetTimestamp());
		if (m_pCommModel != NULL)
			m_pCommModel->DoUpdate(event.GetTimestamp());

		std::map<in_addr_t, SafetyPacket>::iterator iterCarMessage = m_mapKnownVehicles.begin();
		while (iterCarMessage != m_mapKnownVehicles.end())
		{
			if (iterCarMessage->second.m_tTime + MakeTime(NETWORK_TIMEOUT_SECS, NETWORK_TIMEOUT_USECS) < event.GetTimestamp()) {
				std::map<in_addr_t, SafetyPacket>::iterator iterCarMessageTemp = iterCarMessage;
				++iterCarMessage;
				m_mapKnownVehicles.erase(iterCarMessageTemp);
			} else
				++iterCarMessage;
		}

		if (IsActive())
		{
			std::vector<CarModel *> vecNeighbors;
			g_pCarRegistry->GetCommunicatingCarsInRange(this, vecNeighbors);
			m_msgNeighbors.tMessage = m_tTimestamp;
			m_msgNeighbors.iNeighbors = vecNeighbors.size();
			m_msgNeighbors.iCollisionCount = m_pPhysModel != NULL ? (m_pPhysModel->GetCollisionCount() - m_msgNeighbors.iAccumulatedCollisions) : 0;
			m_msgNeighbors.iAccumulatedCollisions += m_msgNeighbors.iCollisionCount;
			m_msgNeighbors.iMessages = m_pPhysModel != NULL ? (m_pPhysModel->GetMessageCount() - m_msgNeighbors.iAccumulatedMessages) : 0;
			m_msgNeighbors.iAccumulatedMessages += m_msgNeighbors.iMessages;
#ifdef MULTILANETEST
			m_msgNeighbors.iLane = m_iLane;
#endif
	
			if (m_msgNeighbors.iCollisionCount > 0 || m_msgNeighbors.iMessages > 0 || m_msgNeighbors.iNeighbors > 0)
				g_pLogger->WriteMessage(LOGFILE_NEIGHBORS, &m_msgNeighbors);
		}
		event.SetTimestamp(event.GetTimestamp() + m_tDelay);
		g_pSimulator->m_EventQueue.AddEvent(event);
		break;
	}
	case EVENT_CARMODEL_RXMESSAGEBEGIN:
	{
		Packet * pPacket = (Packet *)event.GetEventData();
		bool bValid = (pPacket != NULL);
		if (bValid && m_pCommModel != NULL && pPacket->m_ePacketType == ptSafety)
			m_pCommModel->AddMessageToHistory(*(SafetyPacket *)pPacket);
		if (bValid && m_pPhysModel != NULL)
			bValid = m_pPhysModel->BeginProcessPacket(pPacket);
		if (bValid && m_pLinkModel != NULL)
			bValid = m_pLinkModel->BeginProcessPacket(pPacket);
		if (bValid)
			g_pSimulator->m_EventQueue.AddEvent(SimEvent(pPacket->m_tRX, EVENT_PRIORITY_HIGHEST, QString::null, m_strModelName, EVENT_CARMODEL_RXMESSAGEEND, pPacket, DestroyPacket));
		else
			DestroyPacket(pPacket);
		break;
	}
	case EVENT_CARMODEL_RXMESSAGEEND:
	{
		std::map<in_addr_t, SafetyPacket>::iterator iterCarMessage;
		std::map<PacketSequence, Event1Message>::iterator iterEvent1Message;
		Packet * pPacket = (Packet *)event.GetEventData();
		// process messages in the order that they were received
		bool bValid = pPacket != NULL && m_pPhysModel != NULL && m_pPhysModel->EndProcessPacket(pPacket);

		if (bValid)
			bValid = m_pLinkModel != NULL && m_pLinkModel->EndProcessPacket(pPacket);

		if (bValid)
		{
			switch (pPacket->m_ePacketType)
			{
			case ptGeneric:
			{
				Packet * pMsg = (Packet *)pPacket;
				// received message successfully
				//g_pLogger->LogInfo(QString("%1> ***Generic Packet\n").arg(IPAddressToString(pMsg->m_ID.srcID.ipCar)));

				g_pLogger->WriteMessage(LOGFILE_MESSAGES, pMsg);
				g_pSimulator->m_mutexEvent1Log.lock();
				if ((iterEvent1Message = g_pSimulator->m_mapEvent1Log.find(pMsg->m_ID.srcID)) != g_pSimulator->m_mapEvent1Log.end())
					iterEvent1Message->second.iCars++;
				g_pSimulator->m_mutexEvent1Log.unlock();
				break;
			}
			case ptSafety:
			{
				SafetyPacket * pMsg = (SafetyPacket *)pPacket;
				// received message successfully
				g_pLogger->LogInfo(QString("%1> ***Safety: %2\n").arg(IPAddressToString(pMsg->m_ID.srcID.ipCar)).arg((const char *)pMsg->m_pData));
				if((const char *)pMsg->m_pData != NULL)
					m_pMessageList->addMessage(QString("<SafetyPacket from %1 received by %2>  %3").arg(IPAddressToString(pMsg->m_ID.srcID.ipCar)).arg(IPAddressToString(pMsg->m_ipRX)).arg((const char *)pMsg->m_pData));
				if (m_pCommModel)
					m_pCommModel->AddMessageToRebroadcastQueue(*pMsg);
				g_pLogger->WriteMessage(LOGFILE_MESSAGES, pMsg);
				g_pSimulator->m_mutexEvent1Log.lock();
				if ((iterEvent1Message = g_pSimulator->m_mapEvent1Log.find(pMsg->m_ID.srcID)) != g_pSimulator->m_mapEvent1Log.end())
					iterEvent1Message->second.iCars++;
				g_pSimulator->m_mutexEvent1Log.unlock();
				break;
			}
			default:
				break;
			}
			m_pLinkModel->AddReceivedPacket(pPacket);
		}

		DestroyPacket(pPacket);
		break;
	}
	default:
		break;
	}

	return 0;
}

int CarModel::PostRun()
{
	if (Model::PostRun())
		return 1;

	std::vector<CarModel *> vecNeighbors;
	g_pCarRegistry->GetCommunicatingCarsInRange(this, vecNeighbors);
	m_msgNeighbors.tMessage = g_pSimulator->m_tCurrent;
	m_msgNeighbors.iNeighbors = vecNeighbors.size();
	m_msgNeighbors.iCollisionCount = m_pPhysModel != NULL ? m_pPhysModel->GetCollisionCount() : 0;
	m_msgNeighbors.iAccumulatedCollisions = m_msgNeighbors.iCollisionCount;
	m_msgNeighbors.iMessages = m_pPhysModel != NULL ? m_pPhysModel->GetMessageCount() : 0;
	m_msgNeighbors.iAccumulatedMessages = m_msgNeighbors.iMessages;

	g_pLogger->WriteMessage(LOGFILE_NEIGHBORS, &m_msgNeighbors);

	return 0;
}

int CarModel::Save(std::map<QString, QString> & mapParams)
{
	if (Model::Save(mapParams))
		return 1;

	mapParams[CARMODEL_PARAM_DELAY] = QString("%1").arg(ToDouble(m_tDelay));
	mapParams[CARMODEL_PARAM_CARIP] = IPAddressToString(m_ipCar);
	mapParams[CARMODEL_PARAM_LOGCAR] = BooleanToString(m_bLogThisCar);
	mapParams[CARMODEL_LINK_PARAM] = m_pLinkModel == NULL ? "NULL" : m_pLinkModel->GetModelName();
	mapParams[CARMODEL_PHYS_PARAM] = m_pPhysModel == NULL ? "NULL" : m_pPhysModel->GetModelName();
	mapParams[CARMODEL_COMM_PARAM] = m_pCommModel == NULL ? "NULL" : m_pCommModel->GetModelName();
	mapParams[CARMODEL_TRACKSPEED_PARAM] = BooleanToString(m_bTrackSpeed);

	return 0;
}

void CarModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	Model::GetParams(mapParams);

	mapParams[CARMODEL_PARAM_DELAY].strValue = CARMODEL_PARAM_DELAY_DEFAULT;
	mapParams[CARMODEL_PARAM_DELAY].strDesc = CARMODEL_PARAM_DELAY_DESC;
	mapParams[CARMODEL_PARAM_DELAY].eType = (ModelParameterType)(ModelParameterTypeFloat | ModelParameterFixed);
	mapParams[CARMODEL_PARAM_DELAY].strAuxData = QString("%1:").arg(5e-2);

	mapParams[CARMODEL_PARAM_CARIP].strValue = CARMODEL_PARAM_CARIP_DEFAULT;
	mapParams[CARMODEL_PARAM_CARIP].strDesc = CARMODEL_PARAM_CARIP_DESC;
	mapParams[CARMODEL_PARAM_CARIP].eType = (ModelParameterType)(ModelParameterTypeIP | ModelParameterFixed);

	mapParams[CARMODEL_PARAM_LOGCAR].strValue = CARMODEL_PARAM_LOGCAR_DEFAULT;
	mapParams[CARMODEL_PARAM_LOGCAR].strDesc = CARMODEL_PARAM_LOGCAR_DESC;
	mapParams[CARMODEL_PARAM_LOGCAR].eType = (ModelParameterType)(ModelParameterTypeYesNo | ModelParameterFixed);

	mapParams[CARMODEL_LINK_PARAM].strValue = CARMODEL_LINK_PARAM_DEFAULT;
	mapParams[CARMODEL_LINK_PARAM].strDesc = CARMODEL_LINK_PARAM_DESC;
	mapParams[CARMODEL_LINK_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[CARMODEL_LINK_PARAM].strAuxData = CARLINKMODEL_NAME;

	mapParams[CARMODEL_PHYS_PARAM].strValue = CARMODEL_PHYS_PARAM_DEFAULT;
	mapParams[CARMODEL_PHYS_PARAM].strDesc = CARMODEL_PHYS_PARAM_DESC;
	mapParams[CARMODEL_PHYS_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[CARMODEL_PHYS_PARAM].strAuxData = CARPHYSMODEL_NAME;

	mapParams[CARMODEL_COMM_PARAM].strValue = CARMODEL_COMM_PARAM_DEFAULT;
	mapParams[CARMODEL_COMM_PARAM].strDesc = CARMODEL_COMM_PARAM_DESC;
	mapParams[CARMODEL_COMM_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[CARMODEL_COMM_PARAM].strAuxData = CARCOMMMODEL_NAME;

	mapParams[CARMODEL_TRACKSPEED_PARAM].strValue = CARMODEL_TRACKSPEED_PARAM_DEFAULT;
	mapParams[CARMODEL_TRACKSPEED_PARAM].strDesc = CARMODEL_TRACKSPEED_PARAM_DESC;
	mapParams[CARMODEL_TRACKSPEED_PARAM].eType = ModelParameterTypeYesNo;
}

int CarModel::Cleanup()
{
	if (m_iMapObjectID > -1)
	{
		std::map<int, MapObject *> * pObjects = g_pMapObjects->acquireLock();
		std::map<int, MapObject *>::iterator iterObject = pObjects->find(m_iMapObjectID);
		if (iterObject != pObjects->end()) {
			g_pMapObjects->UngetColor(((MapCarObject *)iterObject->second)->GetColor());
			delete iterObject->second;
			pObjects->erase(iterObject);
		}
		m_iMapObjectID = -1;
		g_pMapObjects->releaseLock();
	}

	g_pCarRegistry->removeCar(m_ipCar);

	return 0;
}

QString CarModel::GetCarListColumnText(CarListColumn eColumn) const
{
	switch (eColumn)
	{
	case CarListColumnName:
		return IPAddressToString(m_ipCar);
	case CarListColumnLongitude:
		return DegreesToString(m_ptPosition.m_iLong, 6);
	case CarListColumnLatitude:
		return DegreesToString(m_ptPosition.m_iLat, 6);
	case CarListColumnSpeed:
		return QString("%1 mph").arg(m_iSpeed);
	case CarListColumnHeading:
		return DegreesToString((long)m_iHeading * 10000, 2);
	default:
		return "";
	}
}

unsigned int CarModel::GetNextSequenceNumber()
{
	unsigned int iValue;
	m_mutexSeqNumber.lock();
	iValue = m_iNextSeqNumber;
	m_iNextSeqNumber++;
	m_mutexSeqNumber.unlock();
	return iValue;
}

unsigned int CarModel::GetNextRXSequenceNumber()
{
	unsigned int iValue;
	m_mutexRXSeqNumber.lock();
	iValue = m_iNextRXSeqNumber;
	m_iNextRXSeqNumber++;
	m_mutexRXSeqNumber.unlock();
	return iValue;
}

//2006-10-05 - changed to support both SafetyPackets and regular Packets -MH
void CarModel::CreateMessage(Packet * msg)
{

	switch(msg->m_ePacketType)
	{
	default:
	case ptGeneric:
	{
		msg->m_ID.srcID.iSeqNumber = GetNextSequenceNumber();
		msg->m_ID.srcID.ipCar = m_ipCar;
		msg->m_ID.iRXSeqNumber = (unsigned)-1;
		msg->m_ptTXPosition = m_ptPosition;
		msg->m_ipTX = m_ipCar;
		msg->m_ipRX = 0;
		msg->m_tTX = GetCurrentTime() + MakeTime(RandDouble(0., ToDouble(m_tDelay)));
		msg->m_tRX = msg->m_tTX;
		msg->m_iRSSI = PACKET_RSSI_UNAVAILABLE;
		msg->m_iSNR = PACKET_SNR_UNAVAILABLE;
		break;
	}	
	case ptSafety:
	{
		((SafetyPacket*)msg)->m_ID.srcID.iSeqNumber = GetNextSequenceNumber();
		((SafetyPacket*)msg)->m_ID.srcID.ipCar = m_ipCar;
		((SafetyPacket*)msg)->m_ID.iRXSeqNumber = (unsigned)-1;
	//	msg.m_eType = Message::MessageTypeNormal;
		((SafetyPacket*)msg)->m_tTime = m_tTimestamp;
		((SafetyPacket*)msg)->m_tLifetime = m_tDelay;
		((SafetyPacket*)msg)->m_ptPosition = ((SafetyPacket*)msg)->m_ptTXPosition = m_ptPosition;
		((SafetyPacket*)msg)->m_iSpeed = ((SafetyPacket*)msg)->m_iTXSpeed = m_iSpeed;
		((SafetyPacket*)msg)->m_iHeading = ((SafetyPacket*)msg)->m_iTXHeading = m_iHeading;
		((SafetyPacket*)msg)->m_iRecord = m_iCurrentRecord;
		g_pMapDB->GetRelativeRecord(((SafetyPacket*)msg)->m_iRecord, ((SafetyPacket*)msg)->m_iCountyCode);
		((SafetyPacket*)msg)->m_iTXRecord = ((SafetyPacket*)msg)->m_iRecord;
		((SafetyPacket*)msg)->m_iTXCountyCode = ((SafetyPacket*)msg)->m_iCountyCode;
		((SafetyPacket*)msg)->m_cDirection = ((SafetyPacket*)msg)->m_cTXDirection = m_bForwards ? MESSAGE_DIRECTION_FORWARDS : MESSAGE_DIRECTION_BACKWARDS;
		((SafetyPacket*)msg)->m_iShapePoint = ((SafetyPacket*)msg)->m_iTXShapePoint = m_iCRShapePoint;
		((SafetyPacket*)msg)->m_fProgress = ((SafetyPacket*)msg)->m_fTXProgress = m_fCRProgress;
		((SafetyPacket*)msg)->m_ipTX = m_ipCar;
		((SafetyPacket*)msg)->m_ipRX = 0;
		((SafetyPacket*)msg)->m_tTX = GetCurrentTime() + MakeTime(RandDouble(0., ToDouble(m_tDelay)));
		((SafetyPacket*)msg)->m_tRX = ((SafetyPacket*)msg)->m_tTX;
		((SafetyPacket*)msg)->m_iRSSI = PACKET_RSSI_UNAVAILABLE;
		((SafetyPacket*)msg)->m_iSNR = PACKET_SNR_UNAVAILABLE;
		break;
	}
	}
}

void CarModel::TransmitPacket(const Packet * packet)
{
	std::vector<CarModel *> vecCars;
	std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry;
	std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterNode;
	unsigned int iBytesPerSec = GetTXRate();
	struct timeval tTransmit = iBytesPerSec > 0 ? MakeTime((double)packet->GetLength() / iBytesPerSec) : timeval0;
	unsigned int i;
	Packet * pNewPacket = packet->clone();

	// transmit message to all local cars, as well as to infrastructure nodes
	g_pCarRegistry->acquireLock();
	g_pCarRegistry->GetLocalCars(vecCars);

	for (i = 0; i < vecCars.size(); i++) {
		if (vecCars[i]->GetIPAddress() == packet->m_ID.srcID.ipCar || vecCars[i]->GetIPAddress() == m_ipCar)
			continue;

		pNewPacket->m_ipRX = vecCars[i]->GetIPAddress();
		pNewPacket->m_tRX = pNewPacket->m_tTX + tTransmit;
		pNewPacket->m_iRSSI = PACKET_RSSI_UNAVAILABLE;
		pNewPacket->m_iSNR = PACKET_SNR_UNAVAILABLE;
		vecCars[i]->ReceivePacket(pNewPacket);
	}
	g_pCarRegistry->releaseLock();

	if (m_pCommModel != NULL && m_pCommModel->IsGateway())
	{
		pNodeRegistry = g_pInfrastructureNodeRegistry->acquireLock();
		for (iterNode = pNodeRegistry->begin(); iterNode != pNodeRegistry->end(); ++iterNode)
		{
			if (iterNode->second != NULL)
			{
				pNewPacket->m_ipRX = iterNode->first;
				pNewPacket->m_tRX = pNewPacket->m_tTX + tTransmit;
				pNewPacket->m_iRSSI = PACKET_RSSI_UNAVAILABLE;
				pNewPacket->m_iSNR = PACKET_SNR_UNAVAILABLE;
				iterNode->second->ReceivePacket(pNewPacket);
			}
		}
		g_pInfrastructureNodeRegistry->releaseLock();
	}

	// transmit message to all remaining network cars
	pNewPacket->m_ipRX = 0;
	pNewPacket->m_tRX = timeval0;
	pNewPacket->m_iRSSI = PACKET_RSSI_UNAVAILABLE;
	pNewPacket->m_iSNR = PACKET_SNR_UNAVAILABLE;
	SendPacketToAll(pNewPacket);
	AddHybridPacket((Packet*)packet);
	DestroyPacket(pNewPacket);
}

bool CarModel::ReceivePacket(Packet * packet)
{
	std::map<in_addr_t, std::vector<SafetyPacket> >::iterator iterMessage;
	bool bValid = false;
	Packet * pNewPacket;

	packet->m_ID.iRXSeqNumber = GetNextRXSequenceNumber();
	if (m_pPhysModel != NULL && m_pPhysModel->ReceivePacket(packet))
	{
		if (m_pLinkModel != NULL && m_pLinkModel->ReceivePacket(packet))
		{
			unsigned int iBytesPerSec = GetTXRate();
			pNewPacket = packet->clone();
			struct timeval tTransmit = iBytesPerSec > 0 ? MakeTime((double)pNewPacket->GetLength() / iBytesPerSec) : timeval0;
			g_pSimulator->m_EventQueue.AddEvent(SimEvent(pNewPacket->m_tRX - tTransmit, EVENT_PRIORITY_HIGHEST, QString::null, m_strModelName, EVENT_CARMODEL_RXMESSAGEBEGIN, pNewPacket, DestroyPacket));
			bValid = true;
		}
	}

	return bValid;
}


MapCarObject::MapCarObject(CarModel * pCar)
: MapObject(pCar ? pCar->GetModelName() : QString::null), m_pCar(pCar)
{
}

MapCarObject::MapCarObject(const MapCarObject & copy)
: MapObject(copy), m_pCar(copy.m_pCar), m_clrDraw(copy.m_clrDraw)
{
}

MapCarObject::~MapCarObject()
{
}

MapCarObject & MapCarObject::operator = (const MapCarObject & copy)
{
	MapObject::operator =(copy);

	m_pCar = copy.m_pCar;
	m_clrDraw = copy.m_clrDraw;
	return *this;
}

#define MAPCAROBJECT_ARROW_LENGTH 12.f
#define MAPCAROBJECT_ARROW_WIDTH 10.f
#define MAPCAROBJECT_CIRCLE_RADIUS (MAPCAROBJECT_ARROW_LENGTH - ((MAPCAROBJECT_ARROW_LENGTH * MAPCAROBJECT_ARROW_LENGTH - (MAPCAROBJECT_ARROW_WIDTH/2) * (MAPCAROBJECT_ARROW_WIDTH/2)) / (2 * MAPCAROBJECT_ARROW_LENGTH)))

void MapCarObject::DrawObject(const QRect & rBox, QPainter * pDC, MapObjectState eState) const
{
	short iHeading;
	float cos0, sin0;
	QPoint ptCenter;
	Q3PointArray pts(3);

	if (m_pCar == NULL)
		return;

	m_pCar->m_mutexUpdate.lock();
	iHeading = m_pCar->GetCurrentDirection();
	m_pCar->m_mutexUpdate.unlock();

	ptCenter = rBox.center();
	cos0 = cosf(iHeading * RADIANSPERCENTIDEGREE);
	sin0 = sinf(iHeading * RADIANSPERCENTIDEGREE);

	pts[0] = ptCenter + QPoint((int)round(MAPCAROBJECT_CIRCLE_RADIUS*sin0), (int)round(-MAPCAROBJECT_CIRCLE_RADIUS*cos0));
	pts[1] = ptCenter + QPoint((int)round((MAPCAROBJECT_ARROW_WIDTH/2)*cos0-(MAPCAROBJECT_ARROW_LENGTH-MAPCAROBJECT_CIRCLE_RADIUS)*sin0), (int)round((MAPCAROBJECT_ARROW_LENGTH-MAPCAROBJECT_CIRCLE_RADIUS)*cos0+(MAPCAROBJECT_ARROW_WIDTH/2)*sin0));
	pts[2] = ptCenter + QPoint((int)round(-(MAPCAROBJECT_ARROW_WIDTH/2)*cos0-(MAPCAROBJECT_ARROW_LENGTH-MAPCAROBJECT_CIRCLE_RADIUS)*sin0), (int)round((MAPCAROBJECT_ARROW_LENGTH-MAPCAROBJECT_CIRCLE_RADIUS)*cos0-(MAPCAROBJECT_ARROW_WIDTH/2)*sin0));

	pDC->setPen(QPen(m_clrDraw, eState & MapObjectStateCurrent ? 2 : 0));
	pDC->drawLine(pts[0], pts[1]);
	pDC->drawLine(pts[0], pts[2]);
	if (!(eState & MapObjectStateInactive))
		pDC->drawEllipse((int)round(ptCenter.x() - MAPCAROBJECT_CIRCLE_RADIUS), (int)round(ptCenter.y() - MAPCAROBJECT_CIRCLE_RADIUS), (int)round(2*MAPCAROBJECT_CIRCLE_RADIUS), (int)round(2*MAPCAROBJECT_CIRCLE_RADIUS));
	if (hasReceivedCurrentMsg())
	{
		QBrush brushOld = pDC->brush();
		pDC->setBrush(m_clrDraw);
		pDC->drawPolygon(pts);
		pDC->setBrush(brushOld);
	}
}

void MapCarObject::DrawObject(MapDrawingSettings * pSettings, MapObjectState eState) const
{
	Coords ptPosition;
	short iHeading;
	float cos0, sin0;
	QPoint ptCenter;
	Q3PointArray pts(3);

	if (m_pCar == NULL)
		return;

	m_pCar->m_mutexUpdate.lock();
	ptPosition = m_pCar->GetCurrentPosition();
	iHeading = m_pCar->GetCurrentDirection();
	m_pCar->m_mutexUpdate.unlock();

	pts[0] = MapLongLatToScreen(pSettings, ptPosition);
	cos0 = cosf(iHeading * RADIANSPERCENTIDEGREE);
	sin0 = sinf(iHeading * RADIANSPERCENTIDEGREE);

	pts[1] = pts[0] + QPoint((int)((MAPCAROBJECT_ARROW_WIDTH/2)*cos0-(MAPCAROBJECT_ARROW_LENGTH)*sin0), (int)((MAPCAROBJECT_ARROW_LENGTH)*cos0+(MAPCAROBJECT_ARROW_WIDTH/2)*sin0));
	pts[2] = pts[0] + QPoint((int)(-(MAPCAROBJECT_ARROW_WIDTH/2)*cos0-(MAPCAROBJECT_ARROW_LENGTH)*sin0), (int)((MAPCAROBJECT_ARROW_LENGTH)*cos0-(MAPCAROBJECT_ARROW_WIDTH/2)*sin0));
	ptCenter = pts[0] + QPoint((int)(-MAPCAROBJECT_CIRCLE_RADIUS*(sin0+1)), (int)(MAPCAROBJECT_CIRCLE_RADIUS*(cos0 - 1)));

	pSettings->pMemoryDC->setPen(QPen(m_clrDraw, eState & MapObjectStateCurrent ? 2 : 0));
	pSettings->pMemoryDC->drawLine(pts[0], pts[1]);
	pSettings->pMemoryDC->drawLine(pts[0], pts[2]);
	if (!(eState & MapObjectStateInactive))
		pSettings->pMemoryDC->drawEllipse(ptCenter.x(), ptCenter.y(), (int)(2*MAPCAROBJECT_CIRCLE_RADIUS), (int)(2*MAPCAROBJECT_CIRCLE_RADIUS));
	if (hasReceivedCurrentMsg())
	{
		QBrush brushOld = pSettings->pMemoryDC->brush();
		pSettings->pMemoryDC->setBrush(m_clrDraw);
		pSettings->pMemoryDC->drawPolygon(pts);
		pSettings->pMemoryDC->setBrush(brushOld);
	}
}

QRect MapCarObject::RectVisible(MapDrawingSettings * pSettings) const
{
	Coords ptPosition;
	short iHeading;
	QRect r;
	QPoint ptAt, pt1;
	float cos0, sin0;

	if (m_pCar == NULL)
		return r;

	m_pCar->m_mutexUpdate.lock();
	ptPosition = m_pCar->GetCurrentPosition();
	iHeading = m_pCar->GetCurrentDirection();
	m_pCar->m_mutexUpdate.unlock();

	ptAt = MapLongLatToScreen(pSettings, ptPosition);
	cos0 = cosf(iHeading * RADIANSPERCENTIDEGREE);
	sin0 = sinf(iHeading * RADIANSPERCENTIDEGREE);

	pt1 = ptAt + QPoint((int)(-MAPCAROBJECT_CIRCLE_RADIUS*(sin0+1)), (int)(MAPCAROBJECT_CIRCLE_RADIUS*(cos0 - 1)));
	r.setRect(pt1.x(), pt1.y(), (int)(2 * MAPCAROBJECT_CIRCLE_RADIUS), (int)(2 * MAPCAROBJECT_CIRCLE_RADIUS));

	return r;
}

struct timeval MapCarObject::GetLastModifiedTime() const
{
	if (m_pCar == NULL)
		return timeval0;
	else
		return m_pCar->GetTimestamp();
}

bool MapCarObject::isActive() const
{
	struct timeval tNext = timeval0;
	if (m_pCar != NULL) {
		tNext = m_pCar->GetTimestamp() + m_pCar->GetDelay();
		return m_pCar->IsActive() && g_pSimulator->m_tCurrent != timeval0 && tNext >= g_pSimulator->m_tCurrent;
	} else
		return false;
}

bool MapCarObject::hasReceivedCurrentMsg() const
{
	return m_pCar != NULL && (m_pCar->GetIPAddress() == g_pSimulator->m_msgCurrentTrack.ipCar || m_pCar->HasMessage(g_pSimulator->m_msgCurrentTrack));
}

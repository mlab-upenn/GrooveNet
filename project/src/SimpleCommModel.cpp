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

#include "SimpleCommModel.h"
#include "CarRegistry.h"
#include "InfrastructureNodeRegistry.h"
#include "StringHelp.h"
#include "Simulator.h"

#define SIMPLECOMMMODEL_REBROADCASTINTERVAL_PARAM "REBROADCASTINTERVAL"
#define SIMPLECOMMMODEL_REBROADCASTINTERVAL_PARAM_DEFAULT "1"
#define SIMPLECOMMMODEL_REBROADCASTINTERVAL_PARAM_DESC "REBROADCASTINTERVAL (seconds) -- The upper limit of the base rebroadcast interval for this vehicle."

#define SIMPLECOMMMODEL_DOREBROADCAST_PARAM "REBROADCAST"
#define SIMPLECOMMMODEL_DOREBROADCAST_PARAM_DEFAULT "Yes"
#define SIMPLECOMMMODEL_DOREBROADCAST_PARAM_DESC "REBROADCAST (Yes/No) -- Specify \"Yes\" to enable rebroadcasting for this vehicle, \"No\" to disable it."

#define SIMPLECOMMMODEL_MOBILEGATEWAY_PARAM "GATEWAY"
#define SIMPLECOMMMODEL_MOBILEGATEWAY_PARAM_DEFAULT "No"
#define SIMPLECOMMMODEL_MOBILEGATEWAY_PARAM_DESC "GATEWAY (Yes/No) -- Specify \"Yes\" to enable direct communication with infrastructure nodes (as a mobile gateway), \"No\" otherwise. This parameter has no effect for infrastructure nodes."

#define SIMPLECOMMMODEL_RBXJITTER_PARAM "RBXJITTER"
#define SIMPLECOMMMODEL_RBXJITTER_PARAM_DEFAULT "Yes"
#define SIMPLECOMMMODEL_RBXJITTER_PARAM_DESC "RBXJITTER (Yes/No) -- Specify \"Yes\" to enable jittering of the rebroadcast event, \"No\" to disable it."

SimpleCommModel::SimpleCommModel(const QString & strModelName)
: CarCommModel(strModelName), m_bRebroadcast(true), m_bGateway(false), m_bJitter(true), m_tRebroadcastInterval(MakeTime(1, 0))
{
}

SimpleCommModel::SimpleCommModel(const SimpleCommModel & copy)
: CarCommModel(copy), m_bRebroadcast(copy.m_bRebroadcast), m_bGateway(copy.m_bGateway), m_bJitter(copy.m_bJitter), m_tRebroadcastInterval(copy.m_tRebroadcastInterval)
{
}

SimpleCommModel::~SimpleCommModel()
{
}

SimpleCommModel & SimpleCommModel::operator = (const SimpleCommModel & copy)
{
	CarCommModel::operator =(copy);

	m_bRebroadcast = copy.m_bRebroadcast;
	m_bGateway = copy.m_bGateway;
	m_bJitter = copy.m_bJitter;
	m_tRebroadcastInterval = copy.m_tRebroadcastInterval;
	return *this;
}

int SimpleCommModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;

	if (CarCommModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, SIMPLECOMMMODEL_REBROADCASTINTERVAL_PARAM, SIMPLECOMMMODEL_REBROADCASTINTERVAL_PARAM_DEFAULT);
	m_tRebroadcastInterval = MakeTime(ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL));

	m_bRebroadcast = StringToBoolean(GetParam(mapParams, SIMPLECOMMMODEL_DOREBROADCAST_PARAM, SIMPLECOMMMODEL_DOREBROADCAST_PARAM_DEFAULT));

	m_bGateway = StringToBoolean(GetParam(mapParams, SIMPLECOMMMODEL_MOBILEGATEWAY_PARAM, SIMPLECOMMMODEL_MOBILEGATEWAY_PARAM_DEFAULT));

	m_bJitter = StringToBoolean(GetParam(mapParams, SIMPLECOMMMODEL_RBXJITTER_PARAM, SIMPLECOMMMODEL_RBXJITTER_PARAM_DEFAULT));

	return 0;
}

int SimpleCommModel::PreRun()
{
	if (CarCommModel::PreRun())
		return 1;

	return 0;
}

int SimpleCommModel::ProcessEvent(SimEvent & event)
{
	if (CarCommModel::ProcessEvent(event))
		return 1;

	switch (event.GetEventID())
	{
	case EVENT_CARCOMMMODEL_REBROADCAST:
	{
		RebroadcastMessage * pRBXMsg = (RebroadcastMessage *)event.GetEventData();
		if (pRBXMsg != NULL)
		{
			struct timeval tNext = pRBXMsg->tIntervalHigh + (m_bJitter ? MakeTime(RandDouble(0., ToDouble(m_tRebroadcastInterval))) : m_tRebroadcastInterval);
			pRBXMsg->msg.m_tTX = event.GetTimestamp();
			TransmitMessage(&(pRBXMsg->msg)); // only rebroadcast if there's time left
			pRBXMsg->tIntervalLow = pRBXMsg->tIntervalHigh;
			pRBXMsg->tIntervalHigh = pRBXMsg->tIntervalHigh + m_tRebroadcastInterval;
			if (pRBXMsg->msg.m_tTime + pRBXMsg->msg.m_tLifetime <= tNext)
				delete pRBXMsg;
			else
			{
				event.SetTimestamp(tNext);
				g_pSimulator->m_EventQueue.AddEvent(event);
			}
		}
		break;
	}
	default:
		break;
	}

	return 0;
}

bool SimpleCommModel::DoUpdate(struct timeval tCurrent)
{
	if (!CarCommModel::DoUpdate(tCurrent))
		return false;

	m_mapMsgHistory.clear();
	return true;
}
/*
int SimpleCommModel::Iteration(struct timeval tCurrent)
{
	if (CarCommModel::Iteration(tCurrent))
		return 1;

	std::map<PacketSequence, MessageHistory>::iterator iterMsgHistory;
	for (iterMsgHistory = m_mapMsgHistory.begin(); iterMsgHistory != m_mapMsgHistory.end(); ++iterMsgHistory)
		iterMsgHistory->second.tRelevant = tCurrent;

	while (!m_vecRebroadcast.empty() && m_vecRebroadcast.front().tNext <= tCurrent)
	{
		pop_heap(m_vecRebroadcast.begin(), m_vecRebroadcast.end());
		RebroadcastMessage & msgRebroadcast = m_vecRebroadcast.back();
		// setup for next rebroadcast
		if (msgRebroadcast.msg.m_tTime + msgRebroadcast.msg.m_tLifetime < tCurrent)
		{
			m_mapMsgHistory.erase(msgRebroadcast.msg.m_ID.srcID);
			m_vecRebroadcast.pop_back();
		}
		else
		{
			msgRebroadcast.msg.m_tTX = msgRebroadcast.tNext;
			TransmitMessage(msgRebroadcast.msg); // only rebroadcast if there's time left
			msgRebroadcast.tIntervalLow = msgRebroadcast.tIntervalHigh;
			msgRebroadcast.tIntervalHigh = msgRebroadcast.tIntervalHigh + m_tRebroadcastInterval;
			msgRebroadcast.tNext = msgRebroadcast.tIntervalLow + MakeTime(RandDouble(0., ToDouble(m_tRebroadcastInterval)));
			push_heap(m_vecRebroadcast.begin(), m_vecRebroadcast.end());
		}
	}
	return 0;
}
*/
int SimpleCommModel::Save(std::map<QString, QString> & mapParams)
{
	if (CarCommModel::Save(mapParams))
		return 1;

	mapParams[SIMPLECOMMMODEL_REBROADCASTINTERVAL_PARAM] = QString("%1").arg(ToDouble(m_tRebroadcastInterval));
	mapParams[SIMPLECOMMMODEL_DOREBROADCAST_PARAM] = BooleanToString(m_bRebroadcast);
	mapParams[SIMPLECOMMMODEL_MOBILEGATEWAY_PARAM] = BooleanToString(m_bGateway);
	mapParams[SIMPLECOMMMODEL_RBXJITTER_PARAM] = BooleanToString(m_bJitter);
	return 0;
}

//2006-10-06 - changed to support generic packets in addition to safety packets -MH
bool SimpleCommModel::TransmitMessage(Packet * msg)
{
	bool bSuccess = false;
	std::map<in_addr_t, CarModel *> * pCarRegistry = g_pCarRegistry->acquireLock();
	std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry = g_pInfrastructureNodeRegistry->acquireLock();
	std::map<in_addr_t, CarModel *>::iterator iterCar = pCarRegistry->find(m_ipCar);
	std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterNode = pNodeRegistry->find(m_ipCar);
	if (iterCar != pCarRegistry->end() && iterCar->second != NULL)
	{
		if (iterCar->second->IsActive()) {
			// update message stats
			switch(msg->m_ePacketType)
			{
			default:
			case ptGeneric:
			{
				((Packet*)msg)->m_ptTXPosition = iterCar->second->GetCurrentPosition();
				((Packet*)msg)->m_ipTX = iterCar->second->GetIPAddress();
				iterCar->second->TransmitPacket(msg);
				bSuccess = true;
				break;
			}
			case ptSafety:
			{
				((SafetyPacket*)msg)->m_ptTXPosition = iterCar->second->GetCurrentPosition();
				((SafetyPacket*)msg)->m_iTXSpeed = iterCar->second->GetCurrentSpeed();
				((SafetyPacket*)msg)->m_iTXHeading = iterCar->second->GetCurrentDirection();
				((SafetyPacket*)msg)->m_iTXRecord = iterCar->second->GetCurrentRecord();
				g_pMapDB->GetRelativeRecord(((SafetyPacket*)msg)->m_iTXRecord, ((SafetyPacket*)msg)->m_iTXCountyCode);
				((SafetyPacket*)msg)->m_cTXDirection = iterCar->second->IsGoingForwards() ? MESSAGE_DIRECTION_FORWARDS : MESSAGE_DIRECTION_BACKWARDS;
				((SafetyPacket*)msg)->m_iTXShapePoint = iterCar->second->GetCRShapePoint();
				((SafetyPacket*)msg)->m_fTXProgress = iterCar->second->GetCRProgress();
				((SafetyPacket*)msg)->m_ipTX = iterCar->second->GetIPAddress();
				iterCar->second->TransmitPacket(msg);
				bSuccess = true;
				break;
			}
			}
		}
	}
	else if (iterNode != pNodeRegistry->end() && iterNode->second != NULL)
	{
		switch(msg->m_ePacketType)
		{
		default:
		case ptGeneric:
		{
			((Packet*)msg)->m_ptTXPosition = iterNode->second->GetCurrentPosition();
			((Packet*)msg)->m_ipTX = iterNode->second->GetIPAddress();
			iterNode->second->TransmitPacket(msg);
			bSuccess = true;
			break;
		}
		case ptSafety:
		{
			((SafetyPacket*)msg)->m_ptTXPosition = iterNode->second->GetCurrentPosition();
			((SafetyPacket*)msg)->m_iTXSpeed = 0;
			((SafetyPacket*)msg)->m_iTXHeading = 0;
			((SafetyPacket*)msg)->m_iTXRecord = iterNode->second->GetCurrentRecord();
			g_pMapDB->GetRelativeRecord(((SafetyPacket*)msg)->m_iTXRecord, ((SafetyPacket*)msg)->m_iTXCountyCode);
			((SafetyPacket*)msg)->m_cTXDirection = MESSAGE_DIRECTION_FORWARDS;
			((SafetyPacket*)msg)->m_iTXShapePoint = iterNode->second->GetCRShapePoint();
			((SafetyPacket*)msg)->m_fTXProgress = iterNode->second->GetCRProgress();
			((SafetyPacket*)msg)->m_ipTX = iterNode->second->GetIPAddress();
			iterNode->second->TransmitPacket(msg);
			bSuccess = true;
			break;
		}
		}
	}
	g_pInfrastructureNodeRegistry->releaseLock();
	g_pCarRegistry->releaseLock();
	return bSuccess;
}

void SimpleCommModel::AddMessageToRebroadcastQueue(const SafetyPacket & msg)
{
	if (!m_bRebroadcast)
		return;

	// check to see if this a message that should be rebroadcast

	RebroadcastMessage * pRBXMsg = new RebroadcastMessage;
	pRBXMsg->msg = msg;
	pRBXMsg->tIntervalLow = msg.m_tRX;
	pRBXMsg->tIntervalHigh = pRBXMsg->tIntervalLow + m_tRebroadcastInterval;
	g_pSimulator->m_EventQueue.AddEvent(SimEvent(pRBXMsg->tIntervalLow + (m_bJitter ? MakeTime(RandDouble(0., ToDouble(m_tRebroadcastInterval))) : m_tRebroadcastInterval), EVENT_PRIORITY_LOWEST, m_strModelName, m_strModelName, EVENT_CARCOMMMODEL_REBROADCAST, pRBXMsg, DestroyRebroadcastMessage));
}

void SimpleCommModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	CarCommModel::GetParams(mapParams);

	mapParams[SIMPLECOMMMODEL_REBROADCASTINTERVAL_PARAM].strValue = SIMPLECOMMMODEL_REBROADCASTINTERVAL_PARAM_DEFAULT;
	mapParams[SIMPLECOMMMODEL_REBROADCASTINTERVAL_PARAM].strDesc = SIMPLECOMMMODEL_REBROADCASTINTERVAL_PARAM_DESC;
	mapParams[SIMPLECOMMMODEL_REBROADCASTINTERVAL_PARAM].eType = ModelParameterTypeFloat;
	mapParams[SIMPLECOMMMODEL_REBROADCASTINTERVAL_PARAM].strAuxData = QString("%1:%2").arg(0.5).arg(10.);

	mapParams[SIMPLECOMMMODEL_DOREBROADCAST_PARAM].strValue = SIMPLECOMMMODEL_DOREBROADCAST_PARAM_DEFAULT;
	mapParams[SIMPLECOMMMODEL_DOREBROADCAST_PARAM].strDesc = SIMPLECOMMMODEL_DOREBROADCAST_PARAM_DESC;
	mapParams[SIMPLECOMMMODEL_DOREBROADCAST_PARAM].eType = ModelParameterTypeYesNo;

	mapParams[SIMPLECOMMMODEL_MOBILEGATEWAY_PARAM].strValue = SIMPLECOMMMODEL_MOBILEGATEWAY_PARAM_DEFAULT;
	mapParams[SIMPLECOMMMODEL_MOBILEGATEWAY_PARAM].strDesc = SIMPLECOMMMODEL_MOBILEGATEWAY_PARAM_DESC;
	mapParams[SIMPLECOMMMODEL_MOBILEGATEWAY_PARAM].eType = ModelParameterTypeYesNo;

	mapParams[SIMPLECOMMMODEL_RBXJITTER_PARAM].strValue = SIMPLECOMMMODEL_RBXJITTER_PARAM_DEFAULT;
	mapParams[SIMPLECOMMMODEL_RBXJITTER_PARAM].strDesc = SIMPLECOMMMODEL_RBXJITTER_PARAM_DESC;
	mapParams[SIMPLECOMMMODEL_RBXJITTER_PARAM].eType = ModelParameterTypeYesNo;
}

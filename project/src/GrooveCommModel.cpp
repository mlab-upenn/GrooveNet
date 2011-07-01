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

#include "GrooveCommModel.h"
#include "CarRegistry.h"
#include "InfrastructureNodeRegistry.h"
#include "StringHelp.h"
#include "Simulator.h"

#define GROOVECOMMMODEL_FIRSTRBXJITTER_PARAM "FIRSTRBXJITTER"
#define GROOVECOMMMODEL_FIRSTRBXJITTER_PARAM_DEFAULT "500u"
#define GROOVECOMMMODEL_FIRSTRBXJITTER_PARAM_DESC "FIRSTRBXJITTER (seconds) -- The upper limit for the rebroadcast interval for only the first rebroadcast (requires FASTRBXENABLE = \"Yes\" to have any effect)."

#define GROOVECOMMMODEL_FASTRBXENABLE_PARAM "FASTRBXENABLE"
#define GROOVECOMMMODEL_FASTRBXENABLE_PARAM_DEFAULT "N"
#define GROOVECOMMMODEL_FASTRBXENABLE_PARAM_DESC "FASTRBXENABLE (Yes/No) -- Specify \"Yes\" to enable fast first rebroadcasting (the first rebroadcast will happen much faster than successive rebroadcasting), \"No\" to leave it disabled."

#define BACKOFFCAPOFF 5

GrooveCommModel::GrooveCommModel(const QString & strModelName)
: SimpleCommModel(strModelName), m_tFirstRbxJitter(MakeTime(500e-6)), m_bFastRbx(false)
{
}

GrooveCommModel::GrooveCommModel(const GrooveCommModel & copy)
: SimpleCommModel(copy), m_tFirstRbxJitter(copy.m_tFirstRbxJitter), m_bFastRbx(copy.m_bFastRbx)
{
}

GrooveCommModel::~GrooveCommModel()
{
}

GrooveCommModel & GrooveCommModel::operator = (const GrooveCommModel & copy)
{
	SimpleCommModel::operator =(copy);

	m_tFirstRbxJitter = copy.m_tFirstRbxJitter;
	m_bFastRbx = copy.m_bFastRbx;
	return *this;
}

int GrooveCommModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;

	if (SimpleCommModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, GROOVECOMMMODEL_FIRSTRBXJITTER_PARAM, GROOVECOMMMODEL_FIRSTRBXJITTER_PARAM_DEFAULT);
	m_tFirstRbxJitter = MakeTime(ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL));

	strValue = GetParam(mapParams, GROOVECOMMMODEL_FASTRBXENABLE_PARAM, GROOVECOMMMODEL_FASTRBXENABLE_PARAM_DEFAULT);
	m_bFastRbx = StringToBoolean(strValue);

	return 0;
}

int GrooveCommModel::PreRun()
{
	if (SimpleCommModel::PreRun())
		return 1;

	return 0;
}

int GrooveCommModel::ProcessEvent(SimEvent & event)
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
			struct timeval tNext, tRelevant = event.GetTimestamp() - m_tRebroadcastInterval;
			std::map<PacketSequence, MessageHistory>::iterator iterMsgHistory = m_mapMsgHistory.find(pRBXMsg->msg.m_ID.srcID);
			if (DoRebroadcast(pRBXMsg->msg, tRelevant))
			{
				pRBXMsg->msg.m_tTX = event.GetTimestamp();
				TransmitMessage(&(pRBXMsg->msg)); // only rebroadcast if there's time left
			}

			if (iterMsgHistory == m_mapMsgHistory.end())
			{
				MessageHistory msgHistory;
				msgHistory.tRelevant = tRelevant;
				iterMsgHistory = m_mapMsgHistory.insert(std::pair<PacketSequence, MessageHistory>(pRBXMsg->msg.m_ID.srcID, msgHistory)).first;
				iterMsgHistory->second.vecTXHistory.push_back(std::pair<in_addr_t, struct timeval>(pRBXMsg->msg.m_ipTX, pRBXMsg->msg.m_tRX));
			}
			else
				iterMsgHistory->second.tRelevant = tRelevant;

			pRBXMsg->tIntervalLow = pRBXMsg->tIntervalHigh;
			pRBXMsg->tIntervalHigh = pRBXMsg->tIntervalHigh + GetRbxInterval(pRBXMsg->msg, false);
			if (m_bJitter)
				tNext = pRBXMsg->tIntervalLow + MakeTime(RandDouble(0., ToDouble(pRBXMsg->tIntervalHigh - pRBXMsg->tIntervalLow)));
			else
				tNext = pRBXMsg->tIntervalHigh;
// WHAT IS THIS??????			pRBXMsg->tIntervalHigh = pRBXMsg->tIntervalHigh + m_tRebroadcastInterval;
			if (pRBXMsg->msg.m_tTime + pRBXMsg->msg.m_tLifetime <= tNext)
			{
				m_mapMsgHistory.erase(iterMsgHistory); // don't need it in history anymore
				delete pRBXMsg;
			}
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

bool GrooveCommModel::DoUpdate(struct timeval tCurrent)
{
	return CarCommModel::DoUpdate(tCurrent);
}

int GrooveCommModel::Save(std::map<QString, QString> & mapParams)
{
	if (SimpleCommModel::Save(mapParams))
		return 1;

	mapParams[GROOVECOMMMODEL_FIRSTRBXJITTER_PARAM] = QString("%1").arg(ToDouble(m_tFirstRbxJitter));
	mapParams[GROOVECOMMMODEL_FASTRBXENABLE_PARAM] = BooleanToString(m_bFastRbx);
	return 0;
}

void GrooveCommModel::AddMessageToRebroadcastQueue(const SafetyPacket & msg)
{
	struct timeval tInterval;

	if (!m_bRebroadcast)
		return;

	// check to see if this a message that should be rebroadcast

	RebroadcastMessage * pRBXMsg = new RebroadcastMessage;
	pRBXMsg->msg = msg;
	pRBXMsg->tIntervalLow = msg.m_tRX;
	tInterval = GetRbxInterval(msg, true);
	pRBXMsg->tIntervalHigh = pRBXMsg->tIntervalLow + tInterval;
	if (m_bJitter)
		g_pSimulator->m_EventQueue.AddEvent(SimEvent(pRBXMsg->tIntervalLow + MakeTime(RandDouble(0., ToDouble(tInterval))), EVENT_PRIORITY_LOWEST, m_strModelName, m_strModelName, EVENT_CARCOMMMODEL_REBROADCAST, pRBXMsg, DestroyRebroadcastMessage));
	else
		g_pSimulator->m_EventQueue.AddEvent(SimEvent(pRBXMsg->tIntervalLow + tInterval, EVENT_PRIORITY_LOWEST, m_strModelName, m_strModelName, EVENT_CARCOMMMODEL_REBROADCAST, pRBXMsg, DestroyRebroadcastMessage));
}

bool GrooveCommModel::DoRebroadcast(const SafetyPacket & msg, struct timeval tRelevant) const
{
	// get my position
	std::map<in_addr_t, CarModel *> * pCarRegistry = g_pCarRegistry->acquireLock();
	std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry = g_pInfrastructureNodeRegistry->acquireLock();
	std::map<in_addr_t, CarModel *>::iterator iterCar = pCarRegistry->find(m_ipCar);
	std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterNode = pNodeRegistry->find(m_ipCar);
	bool bRebroadcast = true;
	unsigned int i;

	if (iterCar != pCarRegistry->end() && iterCar->second != NULL)
	{
		std::map<PacketSequence, MessageHistory>::const_iterator iterMsgHistory = m_mapMsgHistory.find(msg.m_ID.srcID);
		if (iterMsgHistory != m_mapMsgHistory.end())
		{
			float fDistance = Distance(iterCar->second->GetCurrentPosition(), msg.m_ptPosition);
			for (i = 0; i < iterMsgHistory->second.vecMessages.size(); i++)
			{
				if (iterMsgHistory->second.vecMessages[i].m_tRX >= tRelevant && Distance(iterMsgHistory->second.vecMessages[i].m_ptTXPosition, msg.m_ptPosition) > fDistance)
				{
					bRebroadcast = false;
					break;
				}
			}
		}


/*		if (m_bLocBased)
		{
			std::map<PacketSequence, MessageHistory>::const_iterator iterMsgHistory = m_mapMsgHistory.find(msg.m_ID.srcID);
			if (iterMsgHistory != m_mapMsgHistory.end())
			{
				float fDistance = Distance(iterCar->second->GetCurrentPosition(), msg.m_ptPosition);
				for (i = 0; i < iterMsgHistory->second.vecMessages.size(); i++)
				{
					if (iterMsgHistory->second.vecMessages[i].m_tRX >= tRelevant && Distance(iterMsgHistory->second.vecMessages[i].m_ptTXPosition, msg.m_ptPosition) > fDistance)
					{
						bRebroadcast = false;
						break;
					}
				}
			}
		}
*/	}
	else if (iterNode == pNodeRegistry->end() || iterNode->second == NULL)
		bRebroadcast = false;

	g_pInfrastructureNodeRegistry->releaseLock();
	g_pCarRegistry->releaseLock();
	return bRebroadcast;
}

struct timeval GrooveCommModel::GetRbxInterval(const SafetyPacket & msg, bool bFirst) const
{
	struct timeval tInterval;
	if (bFirst && m_bFastRbx)
		tInterval = m_tFirstRbxJitter;
	else
		tInterval = m_tRebroadcastInterval;
	return tInterval;
}

void GrooveCommModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	SimpleCommModel::GetParams(mapParams);

	mapParams[GROOVECOMMMODEL_FIRSTRBXJITTER_PARAM].strValue = GROOVECOMMMODEL_FIRSTRBXJITTER_PARAM_DEFAULT;
	mapParams[GROOVECOMMMODEL_FIRSTRBXJITTER_PARAM].strDesc = GROOVECOMMMODEL_FIRSTRBXJITTER_PARAM_DESC;
	mapParams[GROOVECOMMMODEL_FIRSTRBXJITTER_PARAM].eType = ModelParameterTypeFloat;
	mapParams[GROOVECOMMMODEL_FIRSTRBXJITTER_PARAM].strAuxData = QString("%1:%2").arg(500e-6).arg(1.);

	mapParams[GROOVECOMMMODEL_FASTRBXENABLE_PARAM].strValue = GROOVECOMMMODEL_FASTRBXENABLE_PARAM_DEFAULT;
	mapParams[GROOVECOMMMODEL_FASTRBXENABLE_PARAM].strDesc = GROOVECOMMMODEL_FASTRBXENABLE_PARAM_DESC;
	mapParams[GROOVECOMMMODEL_FASTRBXENABLE_PARAM].eType = ModelParameterTypeYesNo;
}

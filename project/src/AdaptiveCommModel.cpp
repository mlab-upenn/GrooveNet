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

#include "AdaptiveCommModel.h"
#include "CarRegistry.h"
#include "InfrastructureNodeRegistry.h"
#include "StringHelp.h"
#include "Simulator.h"

#define ADAPTIVECOMMMODEL_FIRSTRBXJITTER_PARAM "FIRSTRBXJITTER"
#define ADAPTIVECOMMMODEL_FIRSTRBXJITTER_PARAM_DEFAULT "500u"
#define ADAPTIVECOMMMODEL_FIRSTRBXJITTER_PARAM_DESC "FIRSTRBXJITTER (seconds) -- The upper limit for the rebroadcast interval for only the first rebroadcast (requires FASTRBXENABLE = \"Yes\" to have any effect)."

#define ADAPTIVECOMMMODEL_RBXINTERVALHIGHTHRESH_PARAM "RBXINTERVALHIGHTHRESH"
#define ADAPTIVECOMMMODEL_RBXINTERVALHIGHTHRESH_PARAM_DEFAULT "500"
#define ADAPTIVECOMMMODEL_RBXINTERVALHIGHTHRESH_PARAM_DESC "RBXINTERVALHIGHTHRESH (meters) -- The limit below which the rebroadcast interval is always equal to REBROADCASTINTERVAL (requires DISTBASEDENABLE = \"Yes\" to have any effect)."

#define ADAPTIVECOMMMODEL_RBXINTERVALLOWTHRESH_PARAM "RBXINTERVALLOWTHRESH"
#define ADAPTIVECOMMMODEL_RBXINTERVALLOWTHRESH_PARAM_DEFAULT "2000"
#define ADAPTIVECOMMMODEL_RBXINTERVALLOWTHRESH_PARAM_DESC "RBXINTERVALLOWTHRESH (meters) -- The limit above which the rebroadcast interval is always equal to MAXRBXINTERVAL (requires DISTBASEDENABLE = \"Yes\" to have any effect)."

#define ADAPTIVECOMMMODEL_MAXRBXINTERVAL_PARAM "MAXRBXINTERVAL"
#define ADAPTIVECOMMMODEL_MAXRBXINTERVAL_PARAM_DEFAULT "5"
#define ADAPTIVECOMMMODEL_MAXRBXINTERVAL_PARAM_DESC "MAXRBXINTERVAL (seconds) -- The maximum rebroadcast period under distance-based rate adaptation (requires DISTBASEDENABLE = \"Yes\" to have any effect)."

#define ADAPTIVECOMMMODEL_FASTRBXENABLE_PARAM "FASTRBXENABLE"
#define ADAPTIVECOMMMODEL_FASTRBXENABLE_PARAM_DEFAULT "N"
#define ADAPTIVECOMMMODEL_FASTRBXENABLE_PARAM_DESC "FASTRBXENABLE (Yes/No) -- Specify \"Yes\" to enable fast first rebroadcasting (the first rebroadcast will happen much faster than successive rebroadcasting), \"No\" to leave it disabled."

#define ADAPTIVECOMMMODEL_ADAPTIVEENABLE_PARAM "ADAPTIVEENABLE"
#define ADAPTIVECOMMMODEL_ADAPTIVEENABLE_PARAM_DEFAULT "N"
#define ADAPTIVECOMMMODEL_ADAPTIVEENABLE_PARAM_DESC "ADAPTIVEENABLE (Yes/No) -- Specify \"Yes\" to enable binary exponential rate adaptation (throttle rate down according to number of communicating vehicles in range), \"No\" to leave it disabled."

#define ADAPTIVECOMMMODEL_DISTBASEDENABLE_PARAM "DISTBASEDENABLE"
#define ADAPTIVECOMMMODEL_DISTBASEDENABLE_PARAM_DEFAULT "N"
#define ADAPTIVECOMMMODEL_DISTBASEDENABLE_PARAM_DESC "DISTBASEDENABLE (Yes/No) -- Specify \"Yes\" to enable distance-based rate adaptation (throttle rate down over distance), \"No\" to leave it disabled."

#define ADAPTIVECOMMMODEL_LOCBASEDENABLE_PARAM "LOCBASEDENABLE"
#define ADAPTIVECOMMMODEL_LOCBASEDENABLE_PARAM_DEFAULT "N"
#define ADAPTIVECOMMMODEL_LOCBASEDENABLE_PARAM_DESC "LOCBASEDENABLE (Yes/No) -- Specify \"Yes\" to enable location-based suppression (suppress rebroadcasting if somebody has recently broadcast from a location more distant from the origin of the event), \"No\" to leave it disabled."

#define ADAPTIVECOMMMODEL_BACKOFFWINDOW_PARAM "BACKOFFWINDOW"
#define ADAPTIVECOMMMODEL_BACKOFFWINDOW_PARAM_DEFAULT "5"
#define ADAPTIVECOMMMODEL_BACKOFFWINDOW_PARAM_DESC "BACKOFFWINDOW (seconds) -- The time period preceding the present in which the binary exponential rate adaptation records the number of distinct hosts in range."

#define BACKOFFCAPOFF 5

AdaptiveCommModel::AdaptiveCommModel(const QString & strModelName)
: SimpleCommModel(strModelName), m_tFirstRbxJitter(MakeTime(500e-6)), m_tMaxRbxInterval(MakeTime(5.)), m_tBackoffWindow(MakeTime(5.)), m_fRbxThresholdHigh(0.f), m_fRbxThresholdLow(0.f), m_bFastRbx(false), m_bAdaptive(false), m_bDistBased(false), m_bLocBased(false)
{
}

AdaptiveCommModel::AdaptiveCommModel(const AdaptiveCommModel & copy)
: SimpleCommModel(copy), m_tFirstRbxJitter(copy.m_tFirstRbxJitter), m_tMaxRbxInterval(copy.m_tMaxRbxInterval), m_tBackoffWindow(copy.m_tBackoffWindow), m_fRbxThresholdHigh(copy.m_fRbxThresholdHigh), m_fRbxThresholdLow(copy.m_fRbxThresholdLow), m_bFastRbx(copy.m_bFastRbx), m_bAdaptive(copy.m_bAdaptive), m_bDistBased(copy.m_bDistBased), m_bLocBased(copy.m_bLocBased)
{
}

AdaptiveCommModel::~AdaptiveCommModel()
{
}

AdaptiveCommModel & AdaptiveCommModel::operator = (const AdaptiveCommModel & copy)
{
	SimpleCommModel::operator =(copy);

	m_tFirstRbxJitter = copy.m_tFirstRbxJitter;
	m_tMaxRbxInterval = copy.m_tMaxRbxInterval;
	m_tBackoffWindow = copy.m_tBackoffWindow;
	m_fRbxThresholdHigh = copy.m_fRbxThresholdHigh;
	m_fRbxThresholdLow = copy.m_fRbxThresholdLow;
	m_bFastRbx = copy.m_bFastRbx;
	m_bAdaptive = copy.m_bAdaptive;
	m_bDistBased = copy.m_bDistBased;
	m_bLocBased = copy.m_bLocBased;
	return *this;
}

int AdaptiveCommModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;

	if (SimpleCommModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, ADAPTIVECOMMMODEL_FIRSTRBXJITTER_PARAM, ADAPTIVECOMMMODEL_FIRSTRBXJITTER_PARAM_DEFAULT);
	m_tFirstRbxJitter = MakeTime(ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL));

	strValue = GetParam(mapParams, ADAPTIVECOMMMODEL_RBXINTERVALHIGHTHRESH_PARAM, ADAPTIVECOMMMODEL_RBXINTERVALHIGHTHRESH_PARAM_DEFAULT);
	m_fRbxThresholdHigh = ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL);

	strValue = GetParam(mapParams, ADAPTIVECOMMMODEL_RBXINTERVALLOWTHRESH_PARAM, ADAPTIVECOMMMODEL_RBXINTERVALLOWTHRESH_PARAM_DEFAULT);
	m_fRbxThresholdLow = ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL);

	strValue = GetParam(mapParams, ADAPTIVECOMMMODEL_MAXRBXINTERVAL_PARAM, ADAPTIVECOMMMODEL_MAXRBXINTERVAL_PARAM_DEFAULT);
	m_tMaxRbxInterval = MakeTime(ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL));

	strValue = GetParam(mapParams, ADAPTIVECOMMMODEL_FASTRBXENABLE_PARAM, ADAPTIVECOMMMODEL_FASTRBXENABLE_PARAM_DEFAULT);
	m_bFastRbx = StringToBoolean(strValue);

	strValue = GetParam(mapParams, ADAPTIVECOMMMODEL_ADAPTIVEENABLE_PARAM, ADAPTIVECOMMMODEL_ADAPTIVEENABLE_PARAM_DEFAULT);
	m_bAdaptive = StringToBoolean(strValue);

	strValue = GetParam(mapParams, ADAPTIVECOMMMODEL_DISTBASEDENABLE_PARAM, ADAPTIVECOMMMODEL_DISTBASEDENABLE_PARAM_DEFAULT);
	m_bDistBased = StringToBoolean(strValue);

	strValue = GetParam(mapParams, ADAPTIVECOMMMODEL_LOCBASEDENABLE_PARAM, ADAPTIVECOMMMODEL_LOCBASEDENABLE_PARAM_DEFAULT);
	m_bLocBased = StringToBoolean(strValue);

	strValue = GetParam(mapParams, ADAPTIVECOMMMODEL_BACKOFFWINDOW_PARAM, ADAPTIVECOMMMODEL_BACKOFFWINDOW_PARAM_DEFAULT);
	m_tBackoffWindow = MakeTime(ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL));

	return 0;
}

int AdaptiveCommModel::PreRun()
{
	if (SimpleCommModel::PreRun())
		return 1;

	return 0;
}

int AdaptiveCommModel::ProcessEvent(SimEvent & event)
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
			struct timeval tNext, tRelevant = event.GetTimestamp() - m_tMaxRbxInterval;
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

bool AdaptiveCommModel::DoUpdate(struct timeval tCurrent)
{
	return CarCommModel::DoUpdate(tCurrent);
}

int AdaptiveCommModel::Save(std::map<QString, QString> & mapParams)
{
	if (SimpleCommModel::Save(mapParams))
		return 1;

	mapParams[ADAPTIVECOMMMODEL_FIRSTRBXJITTER_PARAM] = QString("%1").arg(ToDouble(m_tFirstRbxJitter));
	mapParams[ADAPTIVECOMMMODEL_RBXINTERVALHIGHTHRESH_PARAM] = QString("%1").arg(m_fRbxThresholdHigh);
	mapParams[ADAPTIVECOMMMODEL_RBXINTERVALLOWTHRESH_PARAM] = QString("%1").arg(m_fRbxThresholdLow);
	mapParams[ADAPTIVECOMMMODEL_MAXRBXINTERVAL_PARAM] = QString("%1").arg(ToDouble(m_tMaxRbxInterval));
	mapParams[ADAPTIVECOMMMODEL_FASTRBXENABLE_PARAM] = BooleanToString(m_bFastRbx);
	mapParams[ADAPTIVECOMMMODEL_ADAPTIVEENABLE_PARAM] = BooleanToString(m_bAdaptive);
	mapParams[ADAPTIVECOMMMODEL_DISTBASEDENABLE_PARAM] = BooleanToString(m_bDistBased);
	mapParams[ADAPTIVECOMMMODEL_LOCBASEDENABLE_PARAM] = BooleanToString(m_bLocBased);
	mapParams[ADAPTIVECOMMMODEL_BACKOFFWINDOW_PARAM] = QString("%1").arg(ToDouble(m_tBackoffWindow));
	return 0;
}

void AdaptiveCommModel::AddMessageToRebroadcastQueue(const SafetyPacket & msg)
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

bool AdaptiveCommModel::DoRebroadcast(const SafetyPacket & msg, struct timeval tRelevant) const
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
		if (m_bLocBased)
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
	}
	else if (iterNode == pNodeRegistry->end() || iterNode->second == NULL)
		bRebroadcast = false;

	g_pInfrastructureNodeRegistry->releaseLock();
	g_pCarRegistry->releaseLock();
	return bRebroadcast;
}

struct timeval AdaptiveCommModel::GetRbxInterval(const SafetyPacket & msg, bool bFirst) const
{
	struct timeval tInterval;
	float fDistance;
	if (bFirst && m_bFastRbx)
		tInterval = m_tFirstRbxJitter;
	else
	{
		if (m_bDistBased)
		{
			bool bDoDistance = true;
			// get my position
			std::map<in_addr_t, CarModel *> * pCarRegistry = g_pCarRegistry->acquireLock();
			std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry = g_pInfrastructureNodeRegistry->acquireLock();
			std::map<in_addr_t, CarModel *>::iterator iterCar = pCarRegistry->find(m_ipCar);
			std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterNode = pNodeRegistry->find(m_ipCar);
			fDistance = 0.f;
		
			if (iterCar != pCarRegistry->end() && iterCar->second != NULL)
				fDistance = Distance(msg.m_ptPosition, iterCar->second->GetCurrentPosition()) * METERSPERMILE;
			else if (iterNode != pNodeRegistry->end() && iterNode->second != NULL)
				fDistance = Distance(msg.m_ptPosition, iterNode->second->GetCurrentPosition()) * METERSPERMILE;
			else
			{
				g_pInfrastructureNodeRegistry->releaseLock();
				g_pCarRegistry->releaseLock();
				tInterval = m_tRebroadcastInterval;
				bDoDistance = false;
			}
		
			if (bDoDistance)
			{
				g_pInfrastructureNodeRegistry->releaseLock();
				g_pCarRegistry->releaseLock();
			
				if (fDistance < m_fRbxThresholdHigh)
					tInterval = m_tRebroadcastInterval;
				else
				{
					if (fDistance < m_fRbxThresholdLow)
					{
//						double fHighFreq = 1., fLowFreq = 0.2;
//						fNewFreq = 0.8 * (m_fRbxThresholdLow - fDistance) / (m_fRbxThresholdLow - m_fRbxThresholdHigh) + 0.2;
//						fNewPeriod = 1./fNewFreq;
//						tInterval = MakeTime(fNewPeriod);
						float fHighFreq = 1.f/ToFloat(m_tRebroadcastInterval), fLowFreq = 1.f/ToFloat(m_tMaxRbxInterval);
						if (fLowFreq > fHighFreq)
							fLowFreq = fHighFreq;
						tInterval = MakeTime(1.f/((fHighFreq - fLowFreq) * (m_fRbxThresholdLow - fDistance) / (m_fRbxThresholdLow - m_fRbxThresholdHigh) + fLowFreq));
					}
					else
						tInterval = m_tMaxRbxInterval;
				}
			}
		}
		else
			tInterval = m_tRebroadcastInterval;

		if (m_bAdaptive)
		{
			std::map<PacketSequence, MessageHistory>::const_iterator iterMsgHistory = m_mapMsgHistory.find(msg.m_ID.srcID);
			unsigned int i, iCount = 0;
			
			if (iterMsgHistory != m_mapMsgHistory.end())
			{
				for (i = 0; i < iterMsgHistory->second.vecTXHistory.size(); i++)
				{
					if (iterMsgHistory->second.vecTXHistory[i].second > msg.m_tRX - m_tBackoffWindow && iterMsgHistory->second.vecTXHistory[i].first != msg.m_ipTX)
						iCount++;
				}
			}

			if (iCount > BACKOFFCAPOFF)
				iCount = BACKOFFCAPOFF; // cap off

			tInterval = MakeTime(ToDouble(tInterval) * (1 << iCount));
		}
	}

	return tInterval;
}

void AdaptiveCommModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	SimpleCommModel::GetParams(mapParams);

	mapParams[ADAPTIVECOMMMODEL_FIRSTRBXJITTER_PARAM].strValue = ADAPTIVECOMMMODEL_FIRSTRBXJITTER_PARAM_DEFAULT;
	mapParams[ADAPTIVECOMMMODEL_FIRSTRBXJITTER_PARAM].strDesc = ADAPTIVECOMMMODEL_FIRSTRBXJITTER_PARAM_DESC;
	mapParams[ADAPTIVECOMMMODEL_FIRSTRBXJITTER_PARAM].eType = ModelParameterTypeFloat;
	mapParams[ADAPTIVECOMMMODEL_FIRSTRBXJITTER_PARAM].strAuxData = QString("%1:%2").arg(500e-6).arg(1.);

	mapParams[ADAPTIVECOMMMODEL_RBXINTERVALHIGHTHRESH_PARAM].strValue = ADAPTIVECOMMMODEL_RBXINTERVALHIGHTHRESH_PARAM_DEFAULT;
	mapParams[ADAPTIVECOMMMODEL_RBXINTERVALHIGHTHRESH_PARAM].strDesc = ADAPTIVECOMMMODEL_RBXINTERVALHIGHTHRESH_PARAM_DESC;
	mapParams[ADAPTIVECOMMMODEL_RBXINTERVALHIGHTHRESH_PARAM].eType = ModelParameterTypeFloat;
	mapParams[ADAPTIVECOMMMODEL_RBXINTERVALHIGHTHRESH_PARAM].strAuxData = QString("%1:%2").arg(200.).arg(1000.);

	mapParams[ADAPTIVECOMMMODEL_RBXINTERVALLOWTHRESH_PARAM].strValue = ADAPTIVECOMMMODEL_RBXINTERVALLOWTHRESH_PARAM_DEFAULT;
	mapParams[ADAPTIVECOMMMODEL_RBXINTERVALLOWTHRESH_PARAM].strDesc = ADAPTIVECOMMMODEL_RBXINTERVALLOWTHRESH_PARAM_DESC;
	mapParams[ADAPTIVECOMMMODEL_RBXINTERVALLOWTHRESH_PARAM].eType = ModelParameterTypeFloat;
	mapParams[ADAPTIVECOMMMODEL_RBXINTERVALLOWTHRESH_PARAM].strAuxData = QString("%1:%2").arg(1000.).arg(HUGE_VAL);

	mapParams[ADAPTIVECOMMMODEL_MAXRBXINTERVAL_PARAM].strValue = ADAPTIVECOMMMODEL_MAXRBXINTERVAL_PARAM_DEFAULT;
	mapParams[ADAPTIVECOMMMODEL_MAXRBXINTERVAL_PARAM].strDesc = ADAPTIVECOMMMODEL_MAXRBXINTERVAL_PARAM_DESC;
	mapParams[ADAPTIVECOMMMODEL_MAXRBXINTERVAL_PARAM].eType = ModelParameterTypeFloat;
	mapParams[ADAPTIVECOMMMODEL_MAXRBXINTERVAL_PARAM].strAuxData = QString("%1:%2").arg(1.).arg(10.);

	mapParams[ADAPTIVECOMMMODEL_FASTRBXENABLE_PARAM].strValue = ADAPTIVECOMMMODEL_FASTRBXENABLE_PARAM_DEFAULT;
	mapParams[ADAPTIVECOMMMODEL_FASTRBXENABLE_PARAM].strDesc = ADAPTIVECOMMMODEL_FASTRBXENABLE_PARAM_DESC;
	mapParams[ADAPTIVECOMMMODEL_FASTRBXENABLE_PARAM].eType = ModelParameterTypeYesNo;

	mapParams[ADAPTIVECOMMMODEL_ADAPTIVEENABLE_PARAM].strValue = ADAPTIVECOMMMODEL_ADAPTIVEENABLE_PARAM_DEFAULT;
	mapParams[ADAPTIVECOMMMODEL_ADAPTIVEENABLE_PARAM].strDesc = ADAPTIVECOMMMODEL_ADAPTIVEENABLE_PARAM_DESC;
	mapParams[ADAPTIVECOMMMODEL_ADAPTIVEENABLE_PARAM].eType = ModelParameterTypeYesNo;

	mapParams[ADAPTIVECOMMMODEL_DISTBASEDENABLE_PARAM].strValue = ADAPTIVECOMMMODEL_DISTBASEDENABLE_PARAM_DEFAULT;
	mapParams[ADAPTIVECOMMMODEL_DISTBASEDENABLE_PARAM].strDesc = ADAPTIVECOMMMODEL_DISTBASEDENABLE_PARAM_DESC;
	mapParams[ADAPTIVECOMMMODEL_DISTBASEDENABLE_PARAM].eType = ModelParameterTypeYesNo;

	mapParams[ADAPTIVECOMMMODEL_LOCBASEDENABLE_PARAM].strValue = ADAPTIVECOMMMODEL_LOCBASEDENABLE_PARAM_DEFAULT;
	mapParams[ADAPTIVECOMMMODEL_LOCBASEDENABLE_PARAM].strDesc = ADAPTIVECOMMMODEL_LOCBASEDENABLE_PARAM_DESC;
	mapParams[ADAPTIVECOMMMODEL_LOCBASEDENABLE_PARAM].eType = ModelParameterTypeYesNo;

	mapParams[ADAPTIVECOMMMODEL_BACKOFFWINDOW_PARAM].strValue = ADAPTIVECOMMMODEL_BACKOFFWINDOW_PARAM_DEFAULT;
	mapParams[ADAPTIVECOMMMODEL_BACKOFFWINDOW_PARAM].strDesc = ADAPTIVECOMMMODEL_BACKOFFWINDOW_PARAM_DESC;
	mapParams[ADAPTIVECOMMMODEL_BACKOFFWINDOW_PARAM].eType = ModelParameterTypeFloat;
	mapParams[ADAPTIVECOMMMODEL_BACKOFFWINDOW_PARAM].strAuxData = QString("%1:%2").arg(0.).arg(HUGE_VAL);
}

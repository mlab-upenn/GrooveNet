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

#include "SimplePhysModel.h"
#include "CarRegistry.h"
#include "InfrastructureNodeRegistry.h"
#include "StringHelp.h"
#include "Logger.h"

#define SIMPLEPHYSMODEL_PARAM_DISTTHRESH "MAXDISTANCE"
#define SIMPLEPHYSMODEL_PARAM_DISTTHRESH_DEFAULT "200"
#define SIMPLEPHYSMODEL_PARAM_DISTTHRESH_DESC "MAXDISTANCE (meters) -- The communication range of this vehicle."

#define SIMPLEPHYSMODEL_MULTICHANNEL_PARAM "MULTICHANNEL"
#define SIMPLEPHYSMODEL_MULTICHANNEL_PARAM_DEFAULT "NO"
#define SIMPLEPHYSMODEL_MULTICHANNEL_PARAM_DESC "MULTICHANNEL (Yes/No) -- Specify \"Yes\" if you want to emulate the multiple-channel DSRC setup or separate GPS, control, and emergency/warning messages, \"No\" otherwise."

SimplePhysModel::SimplePhysModel(const QString & strModelName)
: CarPhysModel(strModelName), m_fDistanceThreshold(200), m_iMessages(0), m_bMultichannel(false)
{
}

SimplePhysModel::SimplePhysModel(const SimplePhysModel & copy)
: CarPhysModel(copy), m_fDistanceThreshold(copy.m_fDistanceThreshold), m_iMessages(copy.m_iMessages), m_bMultichannel(copy.m_bMultichannel)
{
}

SimplePhysModel::~SimplePhysModel()
{
}

SimplePhysModel & SimplePhysModel::operator = (const SimplePhysModel & copy)
{
	CarPhysModel::operator =(copy);

	m_fDistanceThreshold = copy.m_fDistanceThreshold;
	m_iMessages = copy.m_iMessages;
	m_bMultichannel = copy.m_bMultichannel;
	return *this;
}

int SimplePhysModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;

	if (CarPhysModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, SIMPLEPHYSMODEL_PARAM_DISTTHRESH, SIMPLEPHYSMODEL_PARAM_DISTTHRESH_DEFAULT);
	m_fDistanceThreshold = ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL);

	m_bMultichannel = StringToBoolean(GetParam(mapParams, SIMPLEPHYSMODEL_MULTICHANNEL_PARAM, SIMPLEPHYSMODEL_MULTICHANNEL_PARAM_DEFAULT));
	return 0;
}

int SimplePhysModel::PreRun()
{
	if (CarPhysModel::PreRun())
		return 1;

	m_iMessages = 0;
	return 0;
}

int SimplePhysModel::Save(std::map<QString, QString> & mapParams)
{
	if (CarPhysModel::Save(mapParams))
		return 1;

	mapParams[SIMPLEPHYSMODEL_PARAM_DISTTHRESH] = QString("%1").arg(m_fDistanceThreshold);
	mapParams[SIMPLEPHYSMODEL_MULTICHANNEL_PARAM] = BooleanToString(m_bMultichannel);
	return 0;
}

bool SimplePhysModel::EndProcessPacket(Packet * packet)
{
	bool bValid = true;
	// check to see if we need to simulate the physical layer
	if (true)
	{
		// TODO: fill with some guess as to RSSI and SNR
		packet->m_iRSSI = PACKET_RSSI_UNAVAILABLE;
		packet->m_iSNR = PACKET_SNR_UNAVAILABLE;
	}
	return bValid;
}

bool SimplePhysModel::BeginProcessPacket(Packet * packet)
{
	switch (packet->m_ePacketType)
	{
	case ptSafety:
	{
		//it looks like the only case where m_iMessages is
		//not incremented is when Multichannel is set but the
		//message is not an emergency or warning. however, since
		//the message and warning types are now merged this makes
		//this if statement pointless, so I assume we are supposed
		//to always increment m_iMessages now? hopefully it won't
		//break anything... -MH
		/*
		if (m_bMultichannel)
		{
			Message & msg = *(Message *)packet;
			if (msg.m_eType == Message::MessageTypeEmergency || msg.m_eType == Message::MessageTypeWarning)
				m_iMessages++;
		}
		else
			m_iMessages++;
		break;
		*/
		m_iMessages++;
	}
	default:
		break;
	}
	return true;
}

bool SimplePhysModel::ReceivePacket(Packet * packet)
{
	// see if message is in range, if it is let us receive it
	bool bValid = true;

	if (true)
	{
		// update according to model, instead of using real-world result
//		std::map<in_addr_t, CarModel *> * pCarRegistry = g_pCarRegistry->acquireLock();
		std::map<in_addr_t, CarModel *> * pCarRegistry = g_pCarRegistry->getRegistry();
//		std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry = g_pInfrastructureNodeRegistry->acquireLock();
		std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry = g_pInfrastructureNodeRegistry->getRegistry();
		std::map<in_addr_t, CarModel *>::iterator iterCar = pCarRegistry->find(m_ipCar);
//		std::map<in_addr_t, CarModel *>::iterator iterTXCar = pCarRegistry->find(msg.m_ipTX);
		std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterNode = pNodeRegistry->find(m_ipCar);
//		std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterTXNode = pNodeRegistry->find(msg.m_ipTX);
//		if (iterTXNode != pNodeRegistry->end() && iterTXNode->second != NULL)
//		{
//			if (iterCar != pCarRegistry->end() && iterCar->second != NULL)
//				bValid = iterCar->second->IsActive() && iterTXNode->second->m_pPhysModel != NULL && iterTXNode->second->m_pPhysModel->IsCarInRange(iterCar->second->GetCurrentPosition(), msg.m_ptPosition);
//			else if (iterNode != pNodeRegistry->end() && iterNode->second != NULL)
//				bValid = true;
//			else
//				bValid = false;
//		}
//		else
//		{
			if (iterCar != pCarRegistry->end() && iterCar->second != NULL)
				bValid = iterCar->second->IsActive() && IsCarInRange(iterCar->second->GetCurrentPosition(), packet->m_ptTXPosition);
			else if (iterNode != pNodeRegistry->end() && iterNode->second != NULL)
				bValid = IsCarInRange(iterNode->second->GetCurrentPosition(), packet->m_ptTXPosition);
			else
				bValid = false;
//		}
//		g_pInfrastructureNodeRegistry->releaseLock();
//		g_pCarRegistry->releaseLock();
	}
	return bValid;
}

bool SimplePhysModel::IsCarInRange(const Coords & ptCar, const Coords & ptPosition) const
{
	float f = Distance(ptCar, ptPosition) * METERSPERMILE;
//	g_pLogger->LogInfo(QString("%1\n").arg(f));
	return f < m_fDistanceThreshold;
}

void SimplePhysModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	CarPhysModel::GetParams(mapParams);

	mapParams[SIMPLEPHYSMODEL_PARAM_DISTTHRESH].strValue = SIMPLEPHYSMODEL_PARAM_DISTTHRESH_DEFAULT;
	mapParams[SIMPLEPHYSMODEL_PARAM_DISTTHRESH].strDesc = SIMPLEPHYSMODEL_PARAM_DISTTHRESH_DESC;
	mapParams[SIMPLEPHYSMODEL_PARAM_DISTTHRESH].eType = ModelParameterTypeFloat;
	mapParams[SIMPLEPHYSMODEL_PARAM_DISTTHRESH].strAuxData = "0.:";

	mapParams[SIMPLEPHYSMODEL_MULTICHANNEL_PARAM].strValue = SIMPLEPHYSMODEL_MULTICHANNEL_PARAM_DEFAULT;
	mapParams[SIMPLEPHYSMODEL_MULTICHANNEL_PARAM].strDesc = SIMPLEPHYSMODEL_MULTICHANNEL_PARAM_DESC;
	mapParams[SIMPLEPHYSMODEL_MULTICHANNEL_PARAM].eType = (ModelParameterType)(ModelParameterTypeYesNo | ModelParameterFixed);
}

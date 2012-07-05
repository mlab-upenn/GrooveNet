/***************************************************************************
 *   Copyright (C) 2005, InfrastructureNodenegie Mellon University.                       *
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

#include "InfrastructureNodeModel.h"
#include "CarRegistry.h"
#include "InfrastructureNodeRegistry.h"
#include "Simulator.h"
#include "Logger.h"
#include "Network.h"

#include "StringHelp.h"

#define INFRASTRUCTURENODEMODEL_PARAM_DELAY "DELAY"
#define INFRASTRUCTURENODEMODEL_PARAM_DELAY_DEFAULT "0.2"
#define INFRASTRUCTURENODEMODEL_PARAM_DELAY_DESC "DELAY (seconds) -- The time interval between updating the infrastructure node's models."

#define INFRASTRUCTURENODEMODEL_PARAM_NODEIP_DEFAULT IPAddressToString(::GetIPAddress())
#define INFRASTRUCTURENODEMODEL_PARAM_NODEIP_DESC "ID (IP Address) -- The IP address uniquely identifying this infrastructure node."

#define INFRASTRUCTURENODEMODEL_LINK_PARAM "LINK"
#define INFRASTRUCTURENODEMODEL_LINK_PARAM_DEFAULT "NULL"
#define INFRASTRUCTURENODEMODEL_LINK_PARAM_DESC "LINK (model) -- This infrastructure node's link layer model."

#define INFRASTRUCTURENODEMODEL_PHYS_PARAM "PHYS"
#define INFRASTRUCTURENODEMODEL_PHYS_PARAM_DEFAULT "NULL"
#define INFRASTRUCTURENODEMODEL_PHYS_PARAM_DESC "PHYS (model) -- This infrastructure node's physical layer model."

#define INFRASTRUCTURENODEMODEL_COMM_PARAM "COMM"
#define INFRASTRUCTURENODEMODEL_COMM_PARAM_DEFAULT "NULL"
#define INFRASTRUCTURENODEMODEL_COMM_PARAM_DESC "COMM (model) -- This infrastructure node's communication/rebroadcasting model."

#define INFRASTRUCTURENODEMODEL_LOC_PARAM "POSITION"
#define INFRASTRUCTURENODEMODEL_LOC_PARAM_DEFAULT ""
#define INFRASTRUCTURENODEMODEL_LOC_PARAM_DESC "POSITION (address) -- The street address or intersection specifying where this infrastructure node is located."

InfrastructureNodeModel::InfrastructureNodeModel(const QString & strModelName)
: Model(strModelName), m_pLinkModel(NULL), m_pPhysModel(NULL), m_pCommModel(NULL), m_tDelay(timeval0), m_tTimestamp(timeval0), m_ipNode(0), m_iCurrentRecord((unsigned)-1), m_iCRShapePoint((unsigned)-1), m_fCRProgress(0.f), m_iMapObjectID(-1), m_iNextSeqNumber(0), m_iNextRXSeqNumber(0)
{
}

InfrastructureNodeModel::InfrastructureNodeModel(const InfrastructureNodeModel & copy)
: Model(copy), m_pLinkModel(copy.m_pLinkModel), m_pPhysModel(copy.m_pPhysModel), m_pCommModel(copy.m_pCommModel), m_tDelay(copy.m_tDelay), m_tTimestamp(copy.m_tTimestamp), m_ipNode(copy.m_ipNode), m_ptPosition(copy.m_ptPosition), m_iCurrentRecord(copy.m_iCurrentRecord), m_iCRShapePoint(copy.m_iCRShapePoint), m_fCRProgress(copy.m_fCRProgress), m_iMapObjectID(copy.m_iMapObjectID), m_mapKnownVehicles(copy.m_mapKnownVehicles), m_iNextSeqNumber(copy.m_iNextSeqNumber), m_iNextRXSeqNumber(copy.m_iNextRXSeqNumber), m_sPosition(copy.m_sPosition), m_strPosition(copy.m_strPosition)
{
}

InfrastructureNodeModel::~InfrastructureNodeModel()
{
}

InfrastructureNodeModel & InfrastructureNodeModel::operator =(const InfrastructureNodeModel & copy)
{
	Model::operator =(copy);

	m_pLinkModel = copy.m_pLinkModel;
	m_pPhysModel = copy.m_pPhysModel;
	m_pCommModel = copy.m_pCommModel;

	m_tDelay = copy.m_tDelay;
	m_tTimestamp = copy.m_tTimestamp;
	m_ipNode = copy.m_ipNode;
	m_ptPosition = copy.m_ptPosition;
	m_iCurrentRecord = copy.m_iCurrentRecord;
	m_iCRShapePoint = copy.m_iCRShapePoint;
	m_fCRProgress = copy.m_fCRProgress;
	m_iMapObjectID = copy.m_iMapObjectID;
	m_mapKnownVehicles = copy.m_mapKnownVehicles;
	m_iNextSeqNumber = copy.m_iNextSeqNumber;
	m_iNextRXSeqNumber = copy.m_iNextRXSeqNumber;
	m_sPosition = copy.m_sPosition;
	m_strPosition = copy.m_strPosition;

	return *this;
}

int InfrastructureNodeModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	MapInfrastructureNodeObject * pObject;
	Model * pModel;

	if (Model::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, INFRASTRUCTURENODEMODEL_PARAM_DELAY, INFRASTRUCTURENODEMODEL_PARAM_DELAY_DEFAULT);
	m_tDelay = MakeTime(ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL));

	strValue = GetParam(mapParams, INFRASTRUCTURENODEMODEL_PARAM_NODEIP, INFRASTRUCTURENODEMODEL_PARAM_NODEIP_DEFAULT);
	if (!StringToIPAddress(strValue, m_ipNode))
		return 2;

	strValue = GetParam(mapParams, INFRASTRUCTURENODEMODEL_LINK_PARAM, INFRASTRUCTURENODEMODEL_LINK_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 3;
	m_pLinkModel = (CarLinkModel *)pModel;
	if (m_pLinkModel != NULL)
		m_pLinkModel->SetCar(m_ipNode);

	strValue = GetParam(mapParams, INFRASTRUCTURENODEMODEL_PHYS_PARAM, INFRASTRUCTURENODEMODEL_PHYS_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 4;
	m_pPhysModel = (CarPhysModel *)pModel;
	if (m_pPhysModel != NULL)
		m_pPhysModel->SetCar(m_ipNode);

	strValue = GetParam(mapParams, INFRASTRUCTURENODEMODEL_COMM_PARAM, INFRASTRUCTURENODEMODEL_COMM_PARAM_DEFAULT);
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 5;
	m_pCommModel = (CarCommModel *)pModel;
	if (m_pCommModel != NULL)
		m_pCommModel->SetCar(m_ipNode);

	m_strPosition = GetParam(mapParams, INFRASTRUCTURENODEMODEL_LOC_PARAM, INFRASTRUCTURENODEMODEL_LOC_PARAM_DEFAULT);
	if (!StringToAddress(m_strPosition, &m_sPosition)) {
		m_sPosition.iRecord = m_sPosition.iVertex = (unsigned)-1;
		return 6;
	}

	pObject = new MapInfrastructureNodeObject(this);
	m_iMapObjectID = g_pMapObjects->add(pObject);
	if (m_iMapObjectID == -1)
		delete pObject;

	g_pInfrastructureNodeRegistry->addNode(this);

	m_iNextSeqNumber = 0;
	m_iNextRXSeqNumber = 0;

	return 0;
}

int InfrastructureNodeModel::PreRun()
{
	if (Model::PreRun())
		return 1;

	m_tTimestamp = timeval0;

	if (m_sPosition.iRecord != (unsigned)-1 || m_sPosition.iVertex != (unsigned)-1)
	{
		m_ptPosition = m_sPosition.ptCoordinates;
		if (!g_pMapDB->AddressToPosition(&m_sPosition, m_iCurrentRecord, m_iCRShapePoint, m_fCRProgress))
			m_iCurrentRecord = (unsigned)-1;
	}
	else
	{
		m_ptPosition.Set(0, 0);
		m_iCurrentRecord = (unsigned)-1;
		m_iCRShapePoint = (unsigned)-1;
		m_fCRProgress = 0.f;
	}

	m_mapKnownVehicles.clear();

	g_pSimulator->m_EventQueue.AddEvent(SimEvent(g_pSimulator->m_tCurrent, EVENT_PRIORITY_HIGHEST, m_strModelName, m_strModelName, EVENT_CARMODEL_UPDATE));

	return 0;
}

int InfrastructureNodeModel::ProcessEvent(SimEvent & event)
{
	if (Model::ProcessEvent(event))
		return 1;

	switch (event.GetEventID())
	{
	case EVENT_CARMODEL_UPDATE:
	{
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

		event.SetTimestamp(event.GetTimestamp() + m_tDelay);
		g_pSimulator->m_EventQueue.AddEvent(event);
		break;
	}
	case EVENT_CARMODEL_RXMESSAGEBEGIN:
	{
		Packet * pPacket = (Packet *)event.GetEventData();
		if (m_pCommModel != NULL && pPacket->m_ePacketType == ptSafety)
			m_pCommModel->AddMessageToHistory(*(SafetyPacket *)pPacket);
		if (m_pPhysModel != NULL)
			m_pPhysModel->BeginProcessPacket(pPacket);
		if (m_pLinkModel != NULL)
			m_pLinkModel->BeginProcessPacket(pPacket);
		g_pSimulator->m_EventQueue.AddEvent(SimEvent(pPacket->m_tRX, EVENT_PRIORITY_HIGHEST, QString::null, m_strModelName, EVENT_CARMODEL_RXMESSAGEEND, pPacket, DestroyPacket));
		break;
	}
	case EVENT_CARMODEL_RXMESSAGEEND:
	{
		std::map<in_addr_t, SafetyPacket>::iterator iterCarMessage;
		std::map<PacketSequence, Event1Message>::iterator iterEvent1Message;
		std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry;
		std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterNode;
		Packet * pPacket = (Packet *)event.GetEventData();
		// process messages in the order that they were received
		bool bValid = pPacket != NULL && m_pPhysModel != NULL && m_pPhysModel->EndProcessPacket(pPacket);

		if (bValid)
			bValid = m_pLinkModel != NULL && m_pLinkModel->EndProcessPacket(pPacket);

		if (bValid)
		{
			switch (pPacket->m_ePacketType)
			{
			case ptSafety:
			{
				SafetyPacket * pMsg = (SafetyPacket *)pPacket;
				// received message successfully
				g_pLogger->LogInfo(QString("%1> ***Safety: %2\n").arg(IPAddressToString(pMsg->m_ID.srcID.ipCar)).arg((const char *)pMsg->m_pData));
				g_pLogger->WriteMessage(LOGFILE_MESSAGES, pMsg);
	/*			g_pSimulator->m_mutexEvent1Log.lock();
				if ((iterEvent1Message = g_pSimulator->m_mapEvent1Log.find(pMsg->m_ID.srcID)) != g_pSimulator->m_mapEvent1Log.end())
					iterEvent1Message->second.iCars++;
				g_pSimulator->m_mutexEvent1Log.unlock();
	*/
				break;
			}
			default:
				break;
			}
			pNodeRegistry = g_pInfrastructureNodeRegistry->acquireLock();
			for (iterNode = pNodeRegistry->begin(); iterNode != pNodeRegistry->end(); ++iterNode)
			{
				if (iterNode->second != NULL)
				{
					if (iterNode->second->m_pLinkModel != NULL)
						iterNode->second->m_pLinkModel->AddReceivedPacket(pPacket);

					switch (pPacket->m_ePacketType)
					{
					case ptSafety:
					{
						SafetyPacket * pMsg = (SafetyPacket *)pPacket;
						//removed m_eType m_eType -MH
					/*	switch (pMsg->m_eType)
						{
						case Message::MessageTypeEmergency:
						case Message::MessageTypeWarning:
					*/		if (iterNode->second->m_pCommModel) // handle rebroadcasts
								iterNode->second->m_pCommModel->AddMessageToRebroadcastQueue(*pMsg);
					/*		break;
						default:
							iterCarMessage = iterNode->second->m_mapKnownVehicles.find(pMsg->m_ID.srcID.ipCar);
							if (iterCarMessage == iterNode->second->m_mapKnownVehicles.end())
								iterNode->second->m_mapKnownVehicles.insert(std::pair<in_addr_t, Message>::pair(pMsg->m_ID.srcID.ipCar, *pMsg));
							else if (iterCarMessage->second.m_tTime <= pMsg->m_tTime)
								iterCarMessage->second = *pMsg;
							break;
						}
					*/	break;
					}
					default:
						break;
					}
				}
			}
			g_pInfrastructureNodeRegistry->releaseLock();
		}

		DestroyPacket(pPacket);
		break;
	}
	default:
		break;
	}

	return 0;
}

int InfrastructureNodeModel::Save(std::map<QString, QString> & mapParams)
{
	if (Model::Save(mapParams))
		return 1;

	mapParams[INFRASTRUCTURENODEMODEL_PARAM_DELAY] = QString("%1").arg(ToDouble(m_tDelay));
	mapParams[INFRASTRUCTURENODEMODEL_PARAM_NODEIP] = IPAddressToString(m_ipNode);
	mapParams[INFRASTRUCTURENODEMODEL_LINK_PARAM] = m_pLinkModel == NULL ? "NULL" : m_pLinkModel->GetModelName();
	mapParams[INFRASTRUCTURENODEMODEL_PHYS_PARAM] = m_pPhysModel == NULL ? "NULL" : m_pPhysModel->GetModelName();
	mapParams[INFRASTRUCTURENODEMODEL_COMM_PARAM] = m_pCommModel == NULL ? "NULL" : m_pCommModel->GetModelName();
	mapParams[INFRASTRUCTURENODEMODEL_LOC_PARAM] = m_strPosition;

	return 0;
}

void InfrastructureNodeModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	Model::GetParams(mapParams);

	mapParams[INFRASTRUCTURENODEMODEL_PARAM_DELAY].strValue = INFRASTRUCTURENODEMODEL_PARAM_DELAY_DEFAULT;
	mapParams[INFRASTRUCTURENODEMODEL_PARAM_DELAY].strDesc = INFRASTRUCTURENODEMODEL_PARAM_DELAY_DESC;
	mapParams[INFRASTRUCTURENODEMODEL_PARAM_DELAY].eType = (ModelParameterType)(ModelParameterTypeFloat | ModelParameterFixed);
	mapParams[INFRASTRUCTURENODEMODEL_PARAM_DELAY].strAuxData = QString("%1:").arg(5e-2);

	mapParams[INFRASTRUCTURENODEMODEL_PARAM_NODEIP].strValue = INFRASTRUCTURENODEMODEL_PARAM_NODEIP_DEFAULT;
	mapParams[INFRASTRUCTURENODEMODEL_PARAM_NODEIP].strDesc = INFRASTRUCTURENODEMODEL_PARAM_NODEIP_DESC;
	mapParams[INFRASTRUCTURENODEMODEL_PARAM_NODEIP].eType = (ModelParameterType)(ModelParameterTypeIP | ModelParameterFixed);

	mapParams[INFRASTRUCTURENODEMODEL_LINK_PARAM].strValue = INFRASTRUCTURENODEMODEL_LINK_PARAM_DEFAULT;
	mapParams[INFRASTRUCTURENODEMODEL_LINK_PARAM].strDesc = INFRASTRUCTURENODEMODEL_LINK_PARAM_DESC;
	mapParams[INFRASTRUCTURENODEMODEL_LINK_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[INFRASTRUCTURENODEMODEL_LINK_PARAM].strAuxData = CARLINKMODEL_NAME;

	mapParams[INFRASTRUCTURENODEMODEL_PHYS_PARAM].strValue = INFRASTRUCTURENODEMODEL_PHYS_PARAM_DEFAULT;
	mapParams[INFRASTRUCTURENODEMODEL_PHYS_PARAM].strDesc = INFRASTRUCTURENODEMODEL_PHYS_PARAM_DESC;
	mapParams[INFRASTRUCTURENODEMODEL_PHYS_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[INFRASTRUCTURENODEMODEL_PHYS_PARAM].strAuxData = CARPHYSMODEL_NAME;

	mapParams[INFRASTRUCTURENODEMODEL_COMM_PARAM].strValue = INFRASTRUCTURENODEMODEL_COMM_PARAM_DEFAULT;
	mapParams[INFRASTRUCTURENODEMODEL_COMM_PARAM].strDesc = INFRASTRUCTURENODEMODEL_COMM_PARAM_DESC;
	mapParams[INFRASTRUCTURENODEMODEL_COMM_PARAM].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[INFRASTRUCTURENODEMODEL_COMM_PARAM].strAuxData = CARCOMMMODEL_NAME;

	mapParams[INFRASTRUCTURENODEMODEL_LOC_PARAM].strValue = INFRASTRUCTURENODEMODEL_LOC_PARAM_DEFAULT;
	mapParams[INFRASTRUCTURENODEMODEL_LOC_PARAM].strDesc = INFRASTRUCTURENODEMODEL_LOC_PARAM_DESC;
	mapParams[INFRASTRUCTURENODEMODEL_LOC_PARAM].eType = ModelParameterTypeAddress;
}

int InfrastructureNodeModel::Cleanup()
{
	if (m_iMapObjectID > -1)
	{
		std::map<int, MapObject *> * pObjects = g_pMapObjects->acquireLock();
		std::map<int, MapObject *>::iterator iterObject = pObjects->find(m_iMapObjectID);
		if (iterObject != pObjects->end()) {
			delete iterObject->second;
			pObjects->erase(iterObject);
		}
		m_iMapObjectID = -1;
		g_pMapObjects->releaseLock();
	}

	g_pInfrastructureNodeRegistry->removeNode(m_ipNode);

	return 0;
}

QString InfrastructureNodeModel::GetCarListColumnText(CarListColumn eColumn) const
{
	switch (eColumn)
	{
	case CarListColumnName:
		return IPAddressToString(m_ipNode);
	case CarListColumnType:
		return "Infrastructure Node";
	case CarListColumnLongitude:
		return DegreesToString(m_ptPosition.m_iLong, 6);
	case CarListColumnLatitude:
		return DegreesToString(m_ptPosition.m_iLat, 6);
	default:
		return "";
	}
}

unsigned int InfrastructureNodeModel::GetNextSequenceNumber()
{
	unsigned int iValue;
	m_mutexSeqNumber.lock();
	iValue = m_iNextSeqNumber;
	m_iNextSeqNumber++;
	m_mutexSeqNumber.unlock();
	return iValue;
}

unsigned int InfrastructureNodeModel::GetNextRXSequenceNumber()
{
	unsigned int iValue;
	m_mutexRXSeqNumber.lock();
	iValue = m_iNextRXSeqNumber;
	m_iNextRXSeqNumber++;
	m_mutexRXSeqNumber.unlock();
	return iValue;
}

void InfrastructureNodeModel::TransmitPacket(const Packet * packet)
{
	std::vector<CarModel *> vecCars;
	unsigned int iBytesPerSec = GetTXRate();
	struct timeval tTransmit = iBytesPerSec > 0 ? MakeTime((double)packet->GetLength() / iBytesPerSec) : timeval0;
	Packet * pNewPacket = packet->clone();
	unsigned int i;

	// transmit message to all local cars, as well as to infrastructure nodes
	g_pCarRegistry->acquireLock();
	g_pCarRegistry->GetLocalCars(vecCars);

	for (i = 0; i < vecCars.size(); i++) {
		if (vecCars[i]->GetIPAddress() == packet->m_ID.srcID.ipCar)
			continue;
		
		if(vecCars[i]->m_pCommModel != NULL && vecCars[i]->m_pCommModel->IsGateway())
		{
			pNewPacket->m_ipRX = vecCars[i]->GetIPAddress();
			pNewPacket->m_tRX = pNewPacket->m_tTX + tTransmit;
			pNewPacket->m_iRSSI = PACKET_RSSI_UNAVAILABLE;
			pNewPacket->m_iSNR = PACKET_SNR_UNAVAILABLE;
			vecCars[i]->ReceivePacket(pNewPacket);
		}
	}
	g_pCarRegistry->releaseLock();

	// transmit message to all remaining network cars
	pNewPacket->m_ipRX = 0;
	pNewPacket->m_tRX = timeval0;
	pNewPacket->m_iRSSI = PACKET_RSSI_UNAVAILABLE;
	pNewPacket->m_iSNR = PACKET_SNR_UNAVAILABLE;
	SendPacketToAll(pNewPacket);
	DestroyPacket(pNewPacket);
}

bool InfrastructureNodeModel::ReceivePacket(Packet * packet)
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


MapInfrastructureNodeObject::MapInfrastructureNodeObject(InfrastructureNodeModel * pNode)
: MapObject(pNode ? pNode->GetModelName() : QString::null), m_pNode(pNode)
{
}

MapInfrastructureNodeObject::MapInfrastructureNodeObject(const MapInfrastructureNodeObject & copy)
: MapObject(copy), m_pNode(copy.m_pNode)
{
}

MapInfrastructureNodeObject::~MapInfrastructureNodeObject()
{
}

MapInfrastructureNodeObject & MapInfrastructureNodeObject::operator = (const MapInfrastructureNodeObject & copy)
{
	MapObject::operator =(copy);

	m_pNode = copy.m_pNode;
	return *this;
}

#define MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE 10

void MapInfrastructureNodeObject::DrawObject(const QRect & rBox, QPainter * pDC, MapObjectState eState __attribute__((unused)) ) const
{
	QPoint ptCenter;
	QBrush brushOld = pDC->brush();

	if (m_pNode == NULL)
		return;

	ptCenter = rBox.center();

	if (m_pNode->HasMessage(g_pSimulator->m_msgCurrentTrack)) {
		pDC->setPen("red");
		pDC->setBrush("red");
	} else {
		pDC->setPen("black");
		pDC->setBrush("black");
	}
	pDC->drawRect(ptCenter.x() - MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE / 2, ptCenter.y() - MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE / 2, MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE, MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE);
	pDC->setBrush(brushOld);
}

void MapInfrastructureNodeObject::DrawObject(MapDrawingSettings * pSettings, MapObjectState eState __attribute__((unused)) ) const
{
	Coords ptPosition;
	QPoint ptAt;
	QBrush brushOld = pSettings->pMemoryDC->brush();

	if (m_pNode == NULL)
		return;

	m_pNode->m_mutexUpdate.lock();
	ptPosition = m_pNode->GetCurrentPosition();
	m_pNode->m_mutexUpdate.unlock();

	ptAt = MapLongLatToScreen(pSettings, ptPosition);

	if (m_pNode->HasMessage(g_pSimulator->m_msgCurrentTrack)) {
		pSettings->pMemoryDC->setPen("red");
		pSettings->pMemoryDC->setBrush("red");
	} else {
		pSettings->pMemoryDC->setPen("black");
		pSettings->pMemoryDC->setBrush("black");
	}
	pSettings->pMemoryDC->drawRect(ptAt.x() - MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE / 2, ptAt.y() - MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE / 2, MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE, MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE);
	pSettings->pMemoryDC->setBrush(brushOld);
}

QRect MapInfrastructureNodeObject::RectVisible(MapDrawingSettings * pSettings) const
{
	Coords ptPosition;
	QRect r;
	QPoint ptAt;

	if (m_pNode == NULL)
		return r;

	m_pNode->m_mutexUpdate.lock();
	ptPosition = m_pNode->GetCurrentPosition();
	m_pNode->m_mutexUpdate.unlock();

	ptAt = MapLongLatToScreen(pSettings, ptPosition);

	r.setRect(ptAt.x() - MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE / 2, ptAt.y() - MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE / 2, MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE, MAPINFRASTRUCTURENODEOBJECT_SQUARE_SIDE);

	return r;
}

struct timeval MapInfrastructureNodeObject::GetLastModifiedTime() const
{
	if (m_pNode == NULL)
		return timeval0;
	else
		return m_pNode->GetTimestamp();
}

bool MapInfrastructureNodeObject::isActive() const
{
	struct timeval tNext = timeval0;
	if (m_pNode != NULL) {
		tNext = m_pNode->GetTimestamp() + m_pNode->GetDelay();
		return g_pSimulator->m_tCurrent != timeval0 && tNext >= g_pSimulator->m_tCurrent;
	} else
		return false;
}

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

#include "SimpleLinkModel.h"
#include "CarRegistry.h"
#include "InfrastructureNodeRegistry.h"

SimpleLinkModel::SimpleLinkModel(const QString & strModelName)
: CarLinkModel(strModelName)
{
}

SimpleLinkModel::SimpleLinkModel(const SimpleLinkModel & copy)
: CarLinkModel(copy)
{
}

SimpleLinkModel::~SimpleLinkModel()
{
}

SimpleLinkModel & SimpleLinkModel::operator =(const SimpleLinkModel & copy)
{
	CarLinkModel::operator =(copy);

	return *this;
}

bool SimpleLinkModel::ReceivePacket(Packet * packet)
{
	packet->m_iTTL++;
	return m_ipCar == packet->m_ipRX && packet->GetTimestamp() + packet->GetLifetime() > packet->m_tRX;
}

bool SimpleLinkModel::BeginProcessPacket(Packet * packet)
{
	std::map<in_addr_t, CarModel *> * pCarRegistry = g_pCarRegistry->acquireLock();
	std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry = g_pInfrastructureNodeRegistry->acquireLock();
	std::map<in_addr_t, CarModel *>::iterator iterCar = pCarRegistry->find(m_ipCar);
	std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterNode = pNodeRegistry->find(m_ipCar);
	Coords ptCoords;
	short iDirection = 0;
	if (iterCar != pCarRegistry->end() && iterCar->second != NULL)
	{
		ptCoords = iterCar->second->GetCurrentPosition();
		iDirection = iterCar->second->GetCurrentDirection();
	}
	else if (iterNode != pNodeRegistry->end() && iterNode->second != NULL)
	{
		ptCoords = iterNode->second->GetCurrentPosition();
	}
	g_pInfrastructureNodeRegistry->releaseLock();
	g_pCarRegistry->releaseLock();
	return packet->InValidRegion(ptCoords, iDirection);
}

bool SimpleLinkModel::EndProcessPacket(Packet * packet)
{
	return m_mapPackets.find(packet->m_ID.srcID) == m_mapPackets.end();
}

void SimpleLinkModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	CarLinkModel::GetParams(mapParams);
}

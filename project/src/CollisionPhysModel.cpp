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

#include "CollisionPhysModel.h"
#include "CarRegistry.h"
#include "StringHelp.h"

bool CompareMessageStartTimes(const std::pair<RXPacketSequence, TimeInterval> & x, const std::pair<RXPacketSequence, TimeInterval> & y)
{
	return x.second.first < y.second.first;
}

CollisionPhysModel::CollisionPhysModel(const QString & strModelName)
: SimplePhysModel(strModelName), m_iCollisions(0)
{
}

CollisionPhysModel::CollisionPhysModel(const CollisionPhysModel & copy)
: SimplePhysModel(copy), m_iCollisions(copy.m_iCollisions)
{
	unsigned int i;
	for (i = 0; i < PACKETMESSAGE_TYPENUM; i++)
		m_mapMessages[i] = copy.m_mapMessages[i];
}

CollisionPhysModel::~CollisionPhysModel()
{
}

CollisionPhysModel & CollisionPhysModel::operator = (const CollisionPhysModel & copy)
{
	unsigned int i;

	SimplePhysModel::operator =(copy);

	for (i = 0; i < PACKETMESSAGE_TYPENUM; i++)
		m_mapMessages[i] = copy.m_mapMessages[i];
	m_iCollisions = copy.m_iCollisions;
	return *this;
}

int CollisionPhysModel::PreRun()
{
	unsigned int i;

	if (SimplePhysModel::PreRun())
		return 1;

//	m_vecMessageRXTimes.clear();
	for (i = 0; i < PACKETMESSAGE_TYPENUM; i++)
		m_mapMessages[i].clear();
	m_iCollisions = 0;
	return 0;
}

bool CollisionPhysModel::EndProcessPacket(Packet * packet)
{
	std::map<RXPacketSequence, bool>::iterator iterMessage;
	bool bValid = SimplePhysModel::EndProcessPacket(packet);

	if (bValid)
	{
		unsigned int iChannel = m_bMultichannel ? GetMessageChannel(packet) : 0;
		iterMessage = m_mapMessages[iChannel].find(packet->m_ID);
		bValid = iterMessage != m_mapMessages[iChannel].end();
		if (bValid)
		{
			bValid = !iterMessage->second;
			//no m_eType -MH
			/*
			if (!bValid && (!m_bMultichannel || packet->m_ePacketType == PacketTypeMessage && (((Message *)packet)->m_eType == Message::MessageTypeEmergency || ((Message *)packet)->m_eType == Message::MessageTypeWarning)))
			*/
			if (!bValid && (!m_bMultichannel || packet->m_ePacketType == ptSafety))
				m_iCollisions++;
			m_mapMessages[iChannel].erase(iterMessage);
		}
	}
	return bValid;
}

bool CollisionPhysModel::BeginProcessPacket(Packet * packet)
{
	std::map<RXPacketSequence, bool>::iterator iterMessage;

	SimplePhysModel::BeginProcessPacket(packet);

	unsigned int iChannel = m_bMultichannel ? GetMessageChannel(packet) : 0;
	iterMessage = m_mapMessages[iChannel].begin();
	if (m_mapMessages[iChannel].size() == 1)
		iterMessage->second = true;
	m_mapMessages[iChannel].insert(std::pair<RXPacketSequence, bool>(packet->m_ID, iterMessage != m_mapMessages[iChannel].end()));

	return true;
}

void CollisionPhysModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	SimplePhysModel::GetParams(mapParams);
}

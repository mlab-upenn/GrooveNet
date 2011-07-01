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

#include "Message.h"
#include "MapDB.h"

Packet::Packet(const PacketType ePacketType)
: m_ePacketType(ePacketType), m_ID(rxmsgsequence0), m_tTX(timeval0), m_ipTX(0), m_tRX(timeval0), m_ipRX(0), m_iTTL(0), m_iHeading(0), m_iRSSI(PACKET_RSSI_UNAVAILABLE), m_iSNR(PACKET_SNR_UNAVAILABLE)
{
}

Packet::Packet(const Packet & copy)
: m_ePacketType(copy.m_ePacketType), m_ID(copy.m_ID), m_tTX(copy.m_tTX), m_ipTX(copy.m_ipTX), m_tRX(copy.m_tRX), m_ipRX(copy.m_ipRX), m_ptTXPosition(copy.m_ptTXPosition), m_iTTL(copy.m_iTTL), m_iHeading(copy.m_iHeading), m_iRSSI(copy.m_iRSSI), m_iSNR(copy.m_iSNR)
{
}

Packet::~Packet()
{
}

Packet & Packet::operator = (const Packet & copy)
{
	m_ePacketType = copy.m_ePacketType;
	m_ID = copy.m_ID;

	m_tTX = copy.m_tTX;
	m_ipTX = copy.m_ipTX;

	m_tRX = copy.m_tRX;
	m_ipRX = copy.m_ipRX;

	m_ptTXPosition = copy.m_ptTXPosition;

	m_iTTL = copy.m_iTTL;

	m_iHeading = copy.m_iHeading;

	m_iRSSI = copy.m_iRSSI;
	m_iSNR = copy.m_iSNR;

	return *this;
}

unsigned char * Packet::ToBytes(int & iBytes) const
{
	unsigned char * pBuffer = NULL, * pBytes;

	pBytes = pBuffer = (unsigned char *)malloc(iBytes = PACKET_MINIMUM_LENGTH);

	if (pBytes == NULL)
		return NULL;

	memcpy(pBytes, &m_ePacketType, sizeof(PacketType));
	pBytes += sizeof(PacketType);
	memcpy(pBytes, &m_ID.srcID, sizeof(PacketSequence));
	pBytes += sizeof(PacketSequence);
	memcpy(pBytes, &m_tTX, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(pBytes, &m_ipTX, sizeof(in_addr_t));
	pBytes += sizeof(in_addr_t);
	memcpy(pBytes, &m_ipRX, sizeof(in_addr_t));
	pBytes += sizeof(in_addr_t);
	memcpy(pBytes, &m_ptTXPosition, sizeof(Coords));
	pBytes += sizeof(Coords);
	memcpy(pBytes, &m_iTTL, sizeof(unsigned char));
	pBytes += sizeof(unsigned char);
	memcpy(pBytes, &m_iHeading, sizeof(short));
	pBytes += sizeof(short);

	return pBuffer;
}

bool Packet::FromBytes(unsigned char * & pBytes, int & iBytes)
{
	if (iBytes < (signed)PACKET_MINIMUM_LENGTH)
		return false;

	memcpy(&m_ePacketType, pBytes, sizeof(PacketType));
	pBytes += sizeof(PacketType);
	memcpy(&m_ID.srcID, pBytes, sizeof(PacketSequence));
	pBytes += sizeof(PacketSequence);
	memcpy(&m_tTX, pBytes, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(&m_ipTX, pBytes, sizeof(in_addr_t));
	pBytes += sizeof(in_addr_t);
	memcpy(&m_ipRX, pBytes, sizeof(in_addr_t));
	pBytes += sizeof(in_addr_t);
	memcpy(&m_ptTXPosition, pBytes, sizeof(Coords));
	pBytes += sizeof(Coords);
	memcpy(&m_iTTL, pBytes, sizeof(unsigned char));
	pBytes += sizeof(unsigned char);
	memcpy(&m_iHeading, pBytes, sizeof(short));
	pBytes += sizeof(short);

	iBytes -= PACKET_MINIMUM_LENGTH;
	return true;
}


SafetyPacket::SafetyPacket()
: Packet(ptSafety), m_tTime(timeval0), m_tLifetime(timeval0), m_iSpeed(0), m_iRecord((unsigned)-1), m_iCountyCode((unsigned)-1), m_cDirection(MESSAGE_DIRECTION_FORWARDS), m_iShapePoint((unsigned)-1), m_fProgress(0.f), m_iLane(0), m_iDataLength(0), m_pData(NULL), m_iTXSpeed(0), m_iTXHeading(0), m_iTXRecord((unsigned)-1), m_iTXCountyCode((unsigned)-1), m_cTXDirection(MESSAGE_DIRECTION_FORWARDS), m_iTXShapePoint((unsigned)-1), m_fTXProgress(0.f), m_iTXLane(0)
{
	m_sBoundingRegion.eRegionType = BoundingRegionTypeNone;
}

SafetyPacket::SafetyPacket(const SafetyPacket & copy)
: Packet(copy), m_tTime(copy.m_tTime), m_tLifetime(copy.m_tLifetime), m_ptPosition(copy.m_ptPosition), m_iSpeed(copy.m_iSpeed), m_iRecord(copy.m_iRecord), m_iCountyCode(copy.m_iCountyCode), m_cDirection(copy.m_cDirection), m_iShapePoint(copy.m_iShapePoint), m_fProgress(copy.m_fProgress), m_iLane(copy.m_iLane), m_iDataLength(copy.m_iDataLength), m_pData(copy.m_pData), m_iTXSpeed(copy.m_iTXSpeed), m_iTXHeading(copy.m_iTXHeading), m_iTXRecord(copy.m_iTXRecord), m_iTXCountyCode(copy.m_iTXCountyCode), m_cTXDirection(copy.m_cTXDirection), m_iTXShapePoint(copy.m_iTXShapePoint), m_fTXProgress(copy.m_fTXProgress), m_iTXLane(copy.m_iTXLane), m_sBoundingRegion(copy.m_sBoundingRegion)
{
	if (copy.m_pData != NULL)
		m_pData = (unsigned char *)memcpy(malloc(m_iDataLength), copy.m_pData, m_iDataLength);
}

SafetyPacket::~SafetyPacket()
{
	if (m_pData != NULL)
		free(m_pData);
}

SafetyPacket & SafetyPacket::operator = (const SafetyPacket & copy)
{
	unsigned char * pData = NULL;

	Packet::operator = (copy);

//	m_eType = copy.m_eType;

	m_tTime = copy.m_tTime;
	m_tLifetime = copy.m_tLifetime;
	m_ptPosition = copy.m_ptPosition;
	m_iSpeed = copy.m_iSpeed;
	m_iRecord = copy.m_iRecord;
	m_iCountyCode = copy.m_iCountyCode;
	m_cDirection = copy.m_cDirection;
	m_iShapePoint = copy.m_iShapePoint;
	m_fProgress = copy.m_fProgress;
	m_iLane = copy.m_iLane;
	m_iDataLength = copy.m_iDataLength;

	m_iTXSpeed = copy.m_iTXSpeed;
	m_iTXHeading = copy.m_iTXHeading;
	m_iTXRecord = copy.m_iTXRecord;
	m_iTXCountyCode = copy.m_iTXCountyCode;
	m_cTXDirection = copy.m_cTXDirection;
	m_iTXShapePoint = copy.m_iTXShapePoint;
	m_fTXProgress = copy.m_fTXProgress;
	m_iTXLane = copy.m_iTXLane;

	m_sBoundingRegion = copy.m_sBoundingRegion;

	if (copy.m_pData != NULL)
		pData = (unsigned char *)memcpy(malloc(copy.m_iDataLength), copy.m_pData, copy.m_iDataLength);
	if (m_pData != NULL)
		free(m_pData);
	m_pData = pData;

	return *this;
}

unsigned char * SafetyPacket::ToBytes(int & iBytes) const
{
	unsigned char * pBuffer = NULL, * pBytes;

	pBytes = pBuffer = (unsigned char *)malloc(iBytes = MESSAGE_MINIMUM_LENGTH + m_iDataLength);

	if (pBytes == NULL)
		return NULL;

	// header
	memcpy(pBytes, &m_ePacketType, sizeof(PacketType));
	pBytes += sizeof(PacketType);
	memcpy(pBytes, &m_ID.srcID, sizeof(PacketSequence));
	pBytes += sizeof(PacketSequence);
	memcpy(pBytes, &m_tTX, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(pBytes, &m_ipTX, sizeof(in_addr_t));
	pBytes += sizeof(in_addr_t);
	memcpy(pBytes, &m_ipRX, sizeof(in_addr_t));
	pBytes += sizeof(in_addr_t);
	memcpy(pBytes, &m_ptTXPosition, sizeof(Coords));
	pBytes += sizeof(Coords);
	memcpy(pBytes, &m_iTTL, sizeof(unsigned char));
	pBytes += sizeof(unsigned char);
	memcpy(pBytes, &m_iHeading, sizeof(short));
	pBytes += sizeof(short);

//	memcpy(pBytes, &m_eType, sizeof(MessageType));
//	pBytes += sizeof(MessageType);
	memcpy(pBytes, &m_tTime, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(pBytes, &m_tLifetime, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(pBytes, &m_ptPosition, sizeof(Coords));
	pBytes += sizeof(Coords);
	memcpy(pBytes, &m_iSpeed, sizeof(short));
	pBytes += sizeof(short);
//	memcpy(pBytes, &m_iHeading, sizeof(short));
//	pBytes += sizeof(short);
	memcpy(pBytes, &m_iRecord, sizeof(unsigned int));
	pBytes += sizeof(unsigned int);
	memcpy(pBytes, &m_iCountyCode, sizeof(unsigned short));
	pBytes += sizeof(unsigned short);
	memcpy(pBytes, &m_cDirection, sizeof(char));
	pBytes += sizeof(char);
	memcpy(pBytes, &m_iShapePoint, sizeof(unsigned short));
	pBytes += sizeof(unsigned short);
	memcpy(pBytes, &m_fProgress, sizeof(float));
	pBytes += sizeof(float);
	memcpy(pBytes, &m_iLane, sizeof(unsigned char));
	pBytes += sizeof(unsigned char);
	memcpy(pBytes, &m_iTXSpeed, sizeof(short));
	pBytes += sizeof(short);
	memcpy(pBytes, &m_iTXHeading, sizeof(short));
	pBytes += sizeof(short);
	memcpy(pBytes, &m_iTXRecord, sizeof(unsigned int));
	pBytes += sizeof(unsigned int);
	memcpy(pBytes, &m_iTXCountyCode, sizeof(unsigned short));
	pBytes += sizeof(unsigned short);
	memcpy(pBytes, &m_cTXDirection, sizeof(char));
	pBytes += sizeof(char);
	memcpy(pBytes, &m_iTXShapePoint, sizeof(unsigned short));
	pBytes += sizeof(unsigned short);
	memcpy(pBytes, &m_fTXProgress, sizeof(float));
	pBytes += sizeof(float);
	memcpy(pBytes, &m_iTXLane, sizeof(unsigned char));
	pBytes += sizeof(unsigned char);
	memcpy(pBytes, &m_sBoundingRegion, sizeof(BoundingRegion));
	pBytes += sizeof(BoundingRegion);
	memcpy(pBytes, &m_iDataLength, sizeof(unsigned int));
	pBytes += sizeof(unsigned int);

	if (m_pData != NULL)
		memcpy(pBytes, m_pData, m_iDataLength);
	pBytes += m_iDataLength;

	return pBuffer;
}

bool SafetyPacket::FromBytes(unsigned char * & pBytes, int & iBytes)
{
	if (iBytes < (signed)MESSAGE_MINIMUM_LENGTH)
		return false;

	memcpy(&m_ePacketType, pBytes, sizeof(PacketType));
	pBytes += sizeof(PacketType);
	memcpy(&m_ID.srcID, pBytes, sizeof(PacketSequence));
	pBytes += sizeof(PacketSequence);
	memcpy(&m_tTX, pBytes, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(&m_ipTX, pBytes, sizeof(in_addr_t));
	pBytes += sizeof(in_addr_t);
	memcpy(&m_ipRX, pBytes, sizeof(in_addr_t));
	pBytes += sizeof(in_addr_t);
	memcpy(&m_ptTXPosition, pBytes, sizeof(Coords));
	pBytes += sizeof(Coords);
	memcpy(&m_iTTL, pBytes, sizeof(unsigned char));
	pBytes += sizeof(unsigned char);
	memcpy(&m_iHeading, pBytes, sizeof(short));
	pBytes += sizeof(short);

//	memcpy(&m_eType, pBytes, sizeof(MessageType));
//	pBytes += sizeof(MessageType);
	memcpy(&m_tTime, pBytes, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(&m_tLifetime, pBytes, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(&m_ptPosition, pBytes, sizeof(Coords));
	pBytes += sizeof(Coords);
	memcpy(&m_iSpeed, pBytes, sizeof(short));
	pBytes += sizeof(short);
//	memcpy(&m_iHeading, pBytes, sizeof(short));
//	pBytes += sizeof(short);
	memcpy(&m_iRecord, pBytes, sizeof(unsigned int));
	pBytes += sizeof(unsigned int);
	memcpy(&m_iCountyCode, pBytes, sizeof(unsigned short));
	pBytes += sizeof(unsigned short);
	memcpy(&m_cDirection, pBytes, sizeof(char));
	pBytes += sizeof(char);
	memcpy(&m_iShapePoint, pBytes, sizeof(unsigned short));
	pBytes += sizeof(unsigned short);
	memcpy(&m_fProgress, pBytes, sizeof(float));
	pBytes += sizeof(float);
	memcpy(&m_iLane, pBytes, sizeof(unsigned char));
	pBytes += sizeof(unsigned char);
	memcpy(&m_iTXSpeed, pBytes, sizeof(short));
	pBytes += sizeof(short);
	memcpy(&m_iTXHeading, pBytes, sizeof(short));
	pBytes += sizeof(short);
	memcpy(&m_iTXRecord, pBytes, sizeof(unsigned int));
	pBytes += sizeof(unsigned int);
	memcpy(&m_iTXCountyCode, pBytes, sizeof(unsigned short));
	pBytes += sizeof(unsigned short);
	memcpy(&m_cTXDirection, pBytes, sizeof(char));
	pBytes += sizeof(char);
	memcpy(&m_iTXShapePoint, pBytes, sizeof(unsigned short));
	pBytes += sizeof(unsigned short);
	memcpy(&m_fTXProgress, pBytes, sizeof(float));
	pBytes += sizeof(float);
	memcpy(&m_iTXLane, pBytes, sizeof(unsigned char));
	pBytes += sizeof(unsigned char);
	memcpy(&m_sBoundingRegion, pBytes, sizeof(BoundingRegion));
	pBytes += sizeof(BoundingRegion);
	memcpy(&m_iDataLength, pBytes, sizeof(unsigned int));
	pBytes += sizeof(unsigned int);
	iBytes -= MESSAGE_MINIMUM_LENGTH;

	if (m_pData != NULL)
		free(m_pData);
	if ((signed)m_iDataLength > iBytes) {
		m_pData = NULL;
		m_iDataLength = 0;
		iBytes = 0;
		return false;
	} else if (m_iDataLength > 0) {
		m_pData = (unsigned char *)memcpy(malloc(m_iDataLength), pBytes, m_iDataLength);
		iBytes -= m_iDataLength;
		pBytes += m_iDataLength;
		return true;
	} else {
		m_pData = NULL;
		m_iDataLength = 0;
		return true;
	}
}

bool SafetyPacket::InValidRegion(const Coords & pt, short iDirection) const
{
	unsigned int i;
	switch (m_sBoundingRegion.eRegionType)
	{
	case BoundingRegionTypeBBox:
	{
		Rect rBox(m_sBoundingRegion.vecCoords[0].m_iLong, m_sBoundingRegion.vecCoords[0].m_iLat, m_sBoundingRegion.vecCoords[1].m_iLong, m_sBoundingRegion.vecCoords[1].m_iLat);
		return rBox.normalize().contains(pt);
	}
	case BoundingRegionTypeWaypoint:
	{
		float fProgress = 0.f;
		float fMaxDistance = m_sBoundingRegion.fParam / (EARTHRADIUS * METERSPERMILE * cos(pt.m_iLat * RADIANSPERTIGERDEGREE)) * TIGERDEGREESPERRADIAN;
		fMaxDistance *= fMaxDistance;
		for (i = 1; i < BOUNDINGREGIONCOORDSMAX; i++)
		{
			if (m_sBoundingRegion.vecCoords[i].m_iLong == 0 && m_sBoundingRegion.vecCoords[i].m_iLat == 0)
				break;
			if (PointSegmentDistance(pt, m_sBoundingRegion.vecCoords[i-1], m_sBoundingRegion.vecCoords[i], fProgress) < fMaxDistance)
				return true;
		}
		return false;
	}
	case BoundingRegionTypeCircle:
	{
		return Distance(m_sBoundingRegion.vecCoords[0], pt) * METERSPERMILE <= m_sBoundingRegion.fParam;
	}
	case BoundingRegionTypeDirection:
	{
		Coords flatPt(pt.Flatten()), flatOrg(m_sBoundingRegion.vecCoords[0].Flatten());
		double fHeading = (flatPt - flatOrg).angle();
		double fAngleDifference = fabs(normangle(fHeading - (90 - m_sBoundingRegion.fParam) * RADIANSPERDEGREE));
		return fAngleDifference <= (M_PI / 2);
	}
	default:
		return true;
	}
}

HybridPacket::HybridPacket()
: seqNumber(0), sourceIP(NULL), destIP(NULL)
{
}

HybridPacket::~HybridPacket()
{
}

int HybridPacket::addPacket(Packet* packet)
{
	switch(packet->m_ePacketType)
	{
	case ptGeneric:
		printf("Added Generic Packet\n");
		genericPackets.push_back(packet->clone());
		break;
	case ptSafety:
		printf("Added Safety Packet\n");
		safetyPackets.push_back((SafetyPacket*)packet->clone());
		break;
	default:
		break;
	}
}

unsigned char * HybridPacket::toBytes()
{
	int size = 0;
	int iBytes;
	unsigned int pos = 0;
	std::vector<Packet*>::iterator iterP;
	std::vector<SafetyPacket*>::iterator iterSP;

	size += genericPackets.size() * PACKET_MINIMUM_LENGTH;
	size += safetyPackets.size() * MESSAGE_MINIMUM_LENGTH;
	for(iterSP = safetyPackets.begin(); iterSP != safetyPackets.end(); iterSP++)
	{
		size += (*iterSP)->m_iDataLength;
	}

	unsigned char * data = (unsigned char *)malloc(size+4);
	size += 4;
	pos += 4;
	memcpy(data, &size, 4);
	printf("size in data: %d ", int(*data));

	for(iterP = genericPackets.begin(); iterP != genericPackets.end(); iterP++)
	{
		unsigned char * temp = (*iterP)->ToBytes(iBytes);
		memcpy(data+pos, temp, iBytes);
		pos += PACKET_MINIMUM_LENGTH;
		free(temp);
	}
	for(iterSP = safetyPackets.begin(); iterSP != safetyPackets.end(); iterSP++)
	{
		unsigned char * temp = (*iterSP)->ToBytes(iBytes);
		memcpy(data+pos, temp, iBytes);
		pos += MESSAGE_MINIMUM_LENGTH + (*iterSP)->m_iDataLength;
		free(temp);
	}
	printf("toBytes(): size = %d\n", size);

	return data;
}

int HybridPacket::fromBytes(unsigned char * bytes)
{
	unsigned int bytesRead = 0;
	unsigned char * pos;
	int size;
	PacketType theType;
	Packet * gPacket;
	SafetyPacket * sPacket;
	int iBytes = 0;
	memcpy(&size, bytes, 4);
	bytesRead += 4;

	while(bytesRead < size)
	{
		printf("fromBytes(): bytes read = %d, size = %d\n", bytesRead, size);
		fflush(stdout);
		memcpy(&theType, bytes+bytesRead, sizeof(PacketType));
		switch(theType)
		{
			case ptGeneric:
				iBytes = PACKET_MINIMUM_LENGTH;
				gPacket = new Packet();
				pos = bytes+bytesRead;
				gPacket->FromBytes(pos, iBytes);
				genericPackets.push_back(gPacket);
				bytesRead += PACKET_MINIMUM_LENGTH;
				break;
			case ptSafety:
				//TODO: fix this to be the correct iBytes
				iBytes = MESSAGE_MINIMUM_LENGTH+50;
				sPacket = new SafetyPacket();
				pos = bytes+bytesRead;
				sPacket->FromBytes(pos, iBytes);
				safetyPackets.push_back(sPacket);
				bytesRead += MESSAGE_MINIMUM_LENGTH + sPacket->m_iDataLength;
				break;
		}
	}
}

void HybridPacket::clear()
{
	std::vector<Packet*>::iterator iterP;
	std::vector<SafetyPacket*>::iterator iterSP;
	for(iterP = genericPackets.begin(); iterP != genericPackets.end(); iterP++)
	{
		free(*iterP);
	}
	for(iterSP = safetyPackets.begin(); iterSP != safetyPackets.end(); iterSP++)
	{
		free(*iterSP);
	}
	genericPackets.clear();
	safetyPackets.clear();
}

HybridPacket * HybridPacket::clone()
{
	HybridPacket * newOne = new HybridPacket();
	std::vector<Packet*>::iterator iterP;
	for(iterP = genericPackets.begin(); iterP != genericPackets.end(); iterP++)
	{
		newOne->genericPackets.push_back((*iterP)->clone());
	}
	std::vector<SafetyPacket*>::iterator iterSP;
	for(iterSP = safetyPackets.begin(); iterSP != safetyPackets.end(); iterSP++)
	{
		newOne->safetyPackets.push_back((SafetyPacket*)((*iterSP)->clone()));
	}
	return newOne;
}

int HybridPacket::numPackets()
{
	return genericPackets.size()+safetyPackets.size();
}

SquelchPacket::SquelchPacket()
: Packet(ptSquelch), m_tTime(timeval0), m_tLifetime(timeval0), m_iSpeed(0), m_iRecord((unsigned)-1), m_iCountyCode((unsigned)-1), m_cDirection(MESSAGE_DIRECTION_FORWARDS), m_iShapePoint((unsigned)-1), m_fProgress(0.f)
{
}

SquelchPacket::SquelchPacket(const SquelchPacket & copy)
: Packet(copy), m_tTime(copy.m_tTime), m_tLifetime(copy.m_tLifetime), m_ptPosition(copy.m_ptPosition), m_iSpeed(copy.m_iSpeed), m_iRecord(copy.m_iRecord), m_iCountyCode(copy.m_iCountyCode), m_cDirection(copy.m_cDirection), m_iShapePoint(copy.m_iShapePoint), m_fProgress(copy.m_fProgress)
{
}

SquelchPacket::~SquelchPacket()
{
}

SquelchPacket & SquelchPacket::operator = (const SquelchPacket & copy)
{
	Packet::operator = (copy);

	m_tTime = copy.m_tTime;
	m_tLifetime = copy.m_tLifetime;
	m_ptPosition = copy.m_ptPosition;
	m_iSpeed = copy.m_iSpeed;
//	m_iHeading = copy.m_iHeading;
	m_iRecord = copy.m_iRecord;
	m_iCountyCode = copy.m_iCountyCode;
	m_cDirection = copy.m_cDirection;
	m_iShapePoint = copy.m_iShapePoint;
	m_fProgress = copy.m_fProgress;

	return *this;
}

unsigned char * SquelchPacket::ToBytes(int & iBytes) const
{
	unsigned char * pBuffer = NULL, * pBytes;

	pBytes = pBuffer = (unsigned char *)malloc(iBytes = SQUELCHMSG_MINIMUM_LENGTH);

	if (pBytes == NULL)
		return NULL;

	// header
	memcpy(pBytes, &m_ePacketType, sizeof(PacketType));
	pBytes += sizeof(PacketType);
	memcpy(pBytes, &m_ID.srcID, sizeof(PacketSequence));
	pBytes += sizeof(PacketSequence);
	memcpy(pBytes, &m_tTX, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(pBytes, &m_ipTX, sizeof(in_addr_t));
	pBytes += sizeof(in_addr_t);
	memcpy(pBytes, &m_ipRX, sizeof(in_addr_t));
	pBytes += sizeof(in_addr_t);
	memcpy(pBytes, &m_ptTXPosition, sizeof(Coords));
	pBytes += sizeof(Coords);
	memcpy(pBytes, &m_iTTL, sizeof(unsigned char));
	pBytes += sizeof(unsigned char);

	memcpy(pBytes, &m_tTime, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(pBytes, &m_tLifetime, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(pBytes, &m_ptPosition, sizeof(Coords));
	pBytes += sizeof(Coords);
	memcpy(pBytes, &m_iSpeed, sizeof(short));
	pBytes += sizeof(short);
//	memcpy(pBytes, &m_iHeading, sizeof(short));
//	pBytes += sizeof(short);
	memcpy(pBytes, &m_iRecord, sizeof(unsigned int));
	pBytes += sizeof(unsigned int);
	memcpy(pBytes, &m_iCountyCode, sizeof(unsigned short));
	pBytes += sizeof(unsigned short);
	memcpy(pBytes, &m_cDirection, sizeof(char));
	pBytes += sizeof(char);
	memcpy(pBytes, &m_iShapePoint, sizeof(unsigned short));
	pBytes += sizeof(unsigned short);
	memcpy(pBytes, &m_fProgress, sizeof(float));
	pBytes += sizeof(float);

	return pBuffer;
}

bool SquelchPacket::FromBytes(unsigned char * & pBytes, int & iBytes)
{
	if (iBytes < (signed)SQUELCHMSG_MINIMUM_LENGTH)
		return false;

	memcpy(&m_ePacketType, pBytes, sizeof(PacketType));
	pBytes += sizeof(PacketType);
	memcpy(&m_ID.srcID, pBytes, sizeof(PacketSequence));
	pBytes += sizeof(PacketSequence);
	memcpy(&m_tTX, pBytes, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(&m_ipTX, pBytes, sizeof(in_addr_t));
	pBytes += sizeof(in_addr_t);
	memcpy(&m_ipRX, pBytes, sizeof(in_addr_t));
	pBytes += sizeof(in_addr_t);
	memcpy(&m_ptTXPosition, pBytes, sizeof(Coords));
	pBytes += sizeof(Coords);
	memcpy(&m_iTTL, pBytes, sizeof(unsigned char));
	pBytes += sizeof(unsigned char);

	memcpy(&m_tTime, pBytes, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(&m_tLifetime, pBytes, sizeof(struct timeval));
	pBytes += sizeof(struct timeval);
	memcpy(&m_ptPosition, pBytes, sizeof(Coords));
	pBytes += sizeof(Coords);
	memcpy(&m_iSpeed, pBytes, sizeof(short));
	pBytes += sizeof(short);
//	memcpy(&m_iHeading, pBytes, sizeof(short));
//	pBytes += sizeof(short);
	memcpy(&m_iRecord, pBytes, sizeof(unsigned int));
	pBytes += sizeof(unsigned int);
	memcpy(&m_iCountyCode, pBytes, sizeof(unsigned short));
	pBytes += sizeof(unsigned short);
	memcpy(&m_cDirection, pBytes, sizeof(char));
	pBytes += sizeof(char);
	memcpy(&m_iShapePoint, pBytes, sizeof(unsigned short));
	pBytes += sizeof(unsigned short);
	memcpy(&m_fProgress, pBytes, sizeof(float));
	pBytes += sizeof(float);
	iBytes -= SQUELCHMSG_MINIMUM_LENGTH;

	return true;
}

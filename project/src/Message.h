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

#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <vector>
#include <arpa/inet.h>

#include "Coords.h"
#include "Global.h"

typedef enum PacketTypeEnum
{
	ptGeneric = 0,
	ptSafety,
	ptSquelch,
	ptHybrid,
	ptControl,
	ptTraffic,
	ptApplication,
	ptReserved
} PacketType;

typedef struct PacketSequenceStruct
{
	unsigned int iSeqNumber;
	in_addr_t ipCar;
} PacketSequence;

const PacketSequence msgsequence0 = {0, 0};

typedef struct RXPacketSequenceStruct
{
	PacketSequence srcID;
	unsigned int iRXSeqNumber;
} RXPacketSequence;

const RXPacketSequence rxmsgsequence0 = {msgsequence0, 0};

inline bool operator == (const PacketSequence & x, const PacketSequence & y)
{
	return x.ipCar == y.ipCar && x.iSeqNumber == y.iSeqNumber;
}

inline bool operator < (const PacketSequence & x, const PacketSequence & y)
{
	return x.ipCar < y.ipCar || (x.ipCar == y.ipCar && x.iSeqNumber < y.iSeqNumber);
}

inline bool operator == (const RXPacketSequence & x, const RXPacketSequence & y)
{
	return x.srcID == y.srcID && x.iRXSeqNumber == y.iRXSeqNumber;
}

inline bool operator < (const RXPacketSequence & x, const RXPacketSequence & y)
{
	return x.srcID < y.srcID || (x.srcID == y.srcID && x.iRXSeqNumber < y.iRXSeqNumber);
}

#define PACKET_MINIMUM_LENGTH (sizeof(PacketType)+sizeof(PacketSequence)+sizeof(struct timeval)+2*sizeof(in_addr_t)+sizeof(Coords)+sizeof(unsigned char)+sizeof(short))
#define PACKET_IPRX_OFFSET (sizeof(PacketType)+sizeof(PacketSequence)+sizeof(struct timeval)+sizeof(in_addr_t))
#define PACKET_LIFETIME (MakeTime(5.))
#define PACKET_RSSI_UNAVAILABLE (-1)
#define PACKET_SNR_UNAVAILABLE (0)

#define PACKET_TYPENUM 2
#define PACKETMESSAGE_TYPENUM 7

class Packet
{
public:
	Packet(const PacketType ePacketType = ptGeneric);
	Packet(const Packet & copy);
	virtual ~Packet();

	virtual Packet & operator = (const Packet & copy);
	inline virtual Packet * clone() const
	{
		return new Packet(*this);
	}

	virtual unsigned char * ToBytes(int & iBytes) const;
	virtual bool FromBytes(unsigned char * & pBytes, int & iBytes);
	inline virtual unsigned int GetLength() const
	{
		return PACKET_MINIMUM_LENGTH;
	}
	inline virtual struct timeval GetTimestamp() const
	{
		return m_tRX;
	}
	inline virtual struct timeval GetLifetime() const
	{
		return PACKET_LIFETIME;
	}
	inline virtual bool InValidRegion(const Coords & pt __attribute__((unused)), short iDirection __attribute__((unused)) ) const
	{
		return true;
	}

	// packet identification info
	PacketType m_ePacketType; // default to unknown (0)
	RXPacketSequence m_ID; // sequence number and car IP address

	// transmitter information
	struct timeval m_tTX; // UTC
	in_addr_t m_ipTX; // transmitter of the message

	// receiver information
	struct timeval m_tRX; // UTC (filled in/not transmitted)
	in_addr_t m_ipRX; // intended receiver of the message -- a real IP address

	// more transmitter information
	Coords m_ptTXPosition; // TIGER units

	// more receiver information
	unsigned char m_iTTL; // hop count

	short m_iHeading; // hundredths of degrees clockwise from due N

	// receiver statistics (filled in/not transmitted)
	char m_iRSSI; // units of [dB+95]
	char m_iSNR; // units of [dB]
};

inline bool ComparePackets(const Packet & x, const Packet & y)
{
	return x.m_tTX > y.m_tTX;
}

inline bool ComparePacketsRX(const Packet & x, const Packet & y)
{
	return x.m_tRX > y.m_tRX;
}

inline bool ComparePacketPtrs(const Packet * x, const Packet * y)
{
	return x->m_tTX > y->m_tTX;
}

inline bool ComparePacketPtrsRX(const Packet * x, const Packet * y)
{
	return x->m_tRX > y->m_tRX;
}

inline void DestroyPacket(void * ptr)
{
	if (ptr != NULL)
		delete (Packet *)ptr;
}

#define MESSAGE_MINIMUM_LENGTH (PACKET_MINIMUM_LENGTH+2*sizeof(struct timeval)+sizeof(Coords)+3*sizeof(short)+3*sizeof(unsigned int)+4*sizeof(unsigned short)+2*sizeof(char)+2*sizeof(float)+2*sizeof(unsigned char)+sizeof(SafetyPacket::BoundingRegion))

#define MESSAGE_DIRECTION_FORWARDS ('F')
#define MESSAGE_DIRECTION_BACKWARDS ('B')

#define MESSAGE_TYPENUM 5

#define BOUNDINGREGIONCOORDSMAX 8

class SafetyPacket : public Packet
{
public:
	typedef enum BoundingRegionTypeEnum
	{
		BoundingRegionTypeNone = 0,
		BoundingRegionTypeBBox,
		BoundingRegionTypeWaypoint,
		BoundingRegionTypeCircle,
		BoundingRegionTypeDirection
	} BoundingRegionType;
	
	typedef struct BoundingRegionStruct
	{
		BoundingRegionType eRegionType;
		Coords vecCoords[BOUNDINGREGIONCOORDSMAX+1];
		double fParam;
	} BoundingRegion;

	SafetyPacket();
	SafetyPacket(const SafetyPacket & copy);
	virtual ~SafetyPacket();

	virtual SafetyPacket & operator = (const SafetyPacket & copy);
	inline virtual Packet * clone() const
	{
		return new SafetyPacket(*this);
	}

	virtual unsigned char * ToBytes(int & iBytes) const;
	virtual bool FromBytes(unsigned char * & pBytes, int & iBytes);
	inline virtual unsigned int GetLength() const
	{
		return m_iDataLength + MESSAGE_MINIMUM_LENGTH;
	}
	inline virtual struct timeval GetTimestamp() const
	{
		return m_tTime;
	}
	inline virtual struct timeval GetLifetime() const
	{
		return m_tLifetime;
	}
	virtual bool InValidRegion(const Coords & pt, short iDirection) const;

	// message identification info

	// originator/event information
	struct timeval m_tTime; // UTC, time that data was valid/relevant
	struct timeval m_tLifetime; // relative time until message should expire
	Coords m_ptPosition; // TIGER units
	short m_iSpeed; // mph
	//heading is now part of the generic packet type -MH
	//short m_iHeading; // hundredths of degrees clockwise from due N
	unsigned int m_iRecord; // current record (specified relative to county)
	unsigned short m_iCountyCode; // FIPS code of current county
	char m_cDirection; // 'F' = forwards, 'B' = backwards
	unsigned short m_iShapePoint; // current shape point index
	float m_fProgress; // current progress from shape point
	unsigned char m_iLane; // current lane (always 0 for non-sim vehicles)
	unsigned int m_iDataLength; // length of m_pData buffer
	unsigned char * m_pData; // auxillary data

	// transmitter information
	short m_iTXSpeed; // mph
	short m_iTXHeading; // hundredths of degrees clockwise from due N
	unsigned int m_iTXRecord; // current record (specified relative to county)
	unsigned short m_iTXCountyCode; // FIPS code of current county
	char m_cTXDirection; // 'F' = forwards, 'B' = backwards
	unsigned short m_iTXShapePoint; // current shape point index
	float m_fTXProgress; // current progress from shape point
	unsigned char m_iTXLane; // current lane (always 0 for non-sim vehicles)

	// receiver information

	// other information
	BoundingRegion m_sBoundingRegion;
};

#define HYBRIDPACKET_MINIMUM_LENGTH (sizeof(int)+2*sizeof(in_addr_t)+2*sizeof(int))

class HybridPacket
{
public:
	HybridPacket();
	~HybridPacket();

	int addPacket(Packet* packet);
	unsigned char * toBytes();
	int fromBytes(unsigned char * bytes);
	int numPackets();
	void clear();
	HybridPacket * clone();

	int seqNumber;		//Sequence number of HybridPacket
	in_addr_t sourceIP;	//Source IP of HybridPacket
	in_addr_t destIP;	//destination IP of HybridPacket
	//Vectors containing all the packets that are part of this HybridPacket
	//in toBytes, the lengths of these vectors are also put in the HybridPacket
	std::vector<Packet*> genericPackets;
	std::vector<SafetyPacket*> safetyPackets;
};


#define SQUELCHMSG_MINIMUM_LENGTH (PACKET_MINIMUM_LENGTH+2*sizeof(struct timeval)+sizeof(Coords)+sizeof(short)+sizeof(unsigned int)+2*sizeof(unsigned short)+sizeof(char)+sizeof(float))

class SquelchPacket : public Packet
{
public:
	SquelchPacket();
	SquelchPacket(const SquelchPacket & copy);
	virtual ~SquelchPacket();

	virtual SquelchPacket & operator = (const SquelchPacket & copy);
	inline virtual Packet * clone() const
	{
		return new SquelchPacket(*this);
	}

	virtual unsigned char * ToBytes(int & iBytes) const;
	virtual bool FromBytes(unsigned char * & pBytes, int & iBytes);
	inline virtual unsigned int GetLength() const
	{
		return SQUELCHMSG_MINIMUM_LENGTH;
	}
	inline virtual struct timeval GetTimestamp() const
	{
		return m_tTime;
	}
	inline virtual struct timeval GetLifetime() const
	{
		return m_tLifetime;
	}

	// originator/event information
	struct timeval m_tTime; // UTC, time that data was valid/relevant
	struct timeval m_tLifetime; // relative time until message should expire
	Coords m_ptPosition; // TIGER units
	short m_iSpeed; // mph
//	short m_iHeading; // hundredths of degrees clockwise from due N
	unsigned int m_iRecord; // current record (specified relative to county)
	unsigned short m_iCountyCode; // FIPS code of current county
	char m_cDirection; // 'F' = forwards, 'B' = backwards
	unsigned short m_iShapePoint; // current shape point index
	float m_fProgress; // current progress from shape point

};

typedef struct Event1MessageStruct
{
	struct timeval tMessage, tLifetime; // timestamp
	PacketSequence ID; // sequence # of message associated with event
	float fDistance; // maximum distance the message has traveled
	float fOriginatorDistance; // distance originating car has travelled
	unsigned int iCars; // number of cars that know this message
	// TODO: add anything else important for this test
	Coords ptOrigin; // original position
	Coords ptDest; // destination location, for notifying user when message gets in range
} Event1Message;

inline bool CompareEvent1Messages(const Event1Message & x, const Event1Message & y)
{
	return x.tMessage > y.tMessage;
}

typedef struct NeighborMessageStruct
{
	struct timeval tMessage; // timestamp
	in_addr_t ipCar; // vehicle associated with message
	unsigned int iNeighbors; // number of neighbors that know this message
	unsigned int iCollisionCount; // number of collisions
	unsigned int iAccumulatedCollisions; // accumulated number of collisions
	unsigned int iMessages; // number of messages received
	unsigned int iAccumulatedMessages; // accumulated number of messages received
#ifdef MULTILANETEST
	unsigned int iLane; // lane number of this vehicle (temporary)
#endif
} NeighborMessage;

inline bool CompareNeighborMessages(const NeighborMessage & x, const NeighborMessage & y)
{
	return x.tMessage > y.tMessage;
}

inline Packet * CreatePacket(const PacketType ePacketType)
{
	switch (ePacketType)
	{
		case ptSafety:
			return new SafetyPacket;
		case ptSquelch:
			return new SquelchPacket;
		/*case ptHybrid:
			return new HybridPacket;
		case ptControl:
			return new ControlPacket;
		case ptTraffic:
			return new TrafficPacket;
		case ptApplication:
			return new ApplicationPacket;
		case ptReserved:
			return new ReservedPacket;*/
		default:
			return new Packet;
	}
}

#endif

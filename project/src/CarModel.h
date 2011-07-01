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

#ifndef _CARMODEL_H
#define _CARMODEL_H

#include "Model.h"
#include "MapObjects.h"
#include "Network.h"

#include <arpa/inet.h>

#define CARMODEL_IPOWNER_LOCAL 0

typedef enum CarListColumnEnum
{
	CarListColumnSprite = 0,
	CarListColumnName = 1,
	CarListColumnType = 2,
	CarListColumnLongitude = 3,
	CarListColumnLatitude = 4,
	CarListColumnSpeed = 5,
	CarListColumnHeading = 6,
} CarListColumn;

#define CARMODEL_PARAM_CARIP "ID"
#define CARMODEL_PARAM_LOGCAR "DOLOG"

#define CARMODEL_RSSI_UNAVAILABLE (-1)
#define CARMODEL_SNR_UNAVAILABLE (0)

#define CARLINKMODEL_NAME "CarLinkModel"

class CarLinkModel : public Model
{
public:
	inline virtual QString GetModelType() const
	{
		return CARLINKMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(CARLINKMODEL_NAME) == 0 || (bDescendSufficient && Model::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	CarLinkModel(const QString & strModelName = QString::null);
	CarLinkModel(const CarLinkModel & copy);
	virtual ~CarLinkModel();

	virtual CarLinkModel & operator = (const CarLinkModel & copy);

	virtual int PreRun();
	virtual bool DoUpdate(struct timeval tCurrent);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	inline virtual void SetCar(in_addr_t ipCar)
	{
		m_ipCar = ipCar;
	}
	inline virtual in_addr_t GetCar() const
	{
		return m_ipCar;
	}
	inline virtual bool HasReceivedPacket(const PacketSequence & ID) const
	{
		return m_mapPackets.find(ID) != m_mapPackets.end();
	}

	virtual bool ReceivePacket(Packet * packet) = 0;
	virtual bool BeginProcessPacket(Packet * packet) = 0;
	virtual bool EndProcessPacket(Packet * packet) = 0;
	virtual bool AddReceivedPacket(const Packet * packet);

protected:
	in_addr_t m_ipCar;
	std::map<PacketSequence, struct timeval> m_mapPackets; // make sure we don't have duplicates
};

#define CARPHYSMODEL_NAME "CarPhysModel"

class CarPhysModel : public Model
{
public:
	inline virtual QString GetModelType() const
	{
		return CARPHYSMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(CARPHYSMODEL_NAME) == 0 || (bDescendSufficient && Model::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	CarPhysModel(const QString & strModelName = QString::null);
	CarPhysModel(const CarPhysModel & copy);
	virtual ~CarPhysModel();

	virtual CarPhysModel & operator = (const CarPhysModel & copy);

	inline virtual bool DoUpdate(struct timeval tCurrent __attribute__((unused)) )
	{
		return true;
	}

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	inline virtual void SetCar(in_addr_t ipCar)
	{
		m_ipCar = ipCar;
	}
	inline virtual in_addr_t GetCar() const
	{
		return m_ipCar;
	}

	virtual bool EndProcessPacket(Packet * packet) = 0;
	virtual bool BeginProcessPacket(Packet * packet) = 0;
	virtual bool ReceivePacket(Packet * packet) = 0;
	virtual float GetRXRange(const Packet * packet) const = 0;
	virtual bool IsCarInRange(const Coords & ptCar, const Coords & ptPosition) const = 0;
	inline virtual unsigned int GetCollisionCount() const
	{
		return 0; // doesn't track collisions by default
	}
	inline virtual unsigned int GetMessageCount() const
	{
		return 0; // doesn't track # of messages by default
	}

protected:
	in_addr_t m_ipCar;
};

#define CARCOMMMODEL_NAME "CarCommModel"

#define EVENT_CARCOMMMODEL_REBROADCAST 74251

#define TXHISTORYSIZE 10

class CarCommModel : public Model
{
public:
	typedef struct MessageHistoryStruct
	{
		std::vector<SafetyPacket> vecMessages;
		std::vector<std::pair<in_addr_t, struct timeval> > vecTXHistory;
		struct timeval tRelevant;
	} MessageHistory;

	inline virtual QString GetModelType() const
	{
		return CARCOMMMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(CARCOMMMODEL_NAME) == 0 || (bDescendSufficient && Model::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	CarCommModel(const QString & strModelName = QString::null);
	CarCommModel(const CarCommModel & copy);
	virtual ~CarCommModel();

	virtual CarCommModel & operator = (const CarCommModel & copy);

	virtual int PreRun();
	virtual bool DoUpdate(struct timeval tCurrent);

	virtual bool TransmitMessage(Packet * msg) = 0;
	virtual void AddMessageToRebroadcastQueue(const SafetyPacket & msg) = 0;
	virtual void AddMessageToHistory(const SafetyPacket & msg);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	inline virtual void SetCar(in_addr_t ipCar)
	{
		m_ipCar = ipCar;
	}
	inline virtual in_addr_t GetCar() const
	{
		return m_ipCar;
	}
	inline virtual bool IsGateway() const
	{
		return false;
	}

protected:
	in_addr_t m_ipCar;
	std::map<PacketSequence, MessageHistory > m_mapMsgHistory;
};

#define CARMODEL_NAME "CarModel"

#define EVENT_CARMODEL_UPDATE 592085
#define EVENT_CARMODEL_RXMESSAGEBEGIN 25611012
#define EVENT_CARMODEL_RXMESSAGEEND 123131

typedef struct CongestionInfoStruct
{
	unsigned int iRecord;
	struct timeval tLastUpdated;
	short iSpeed;
} CongestionInfo;

class CarModel : public Model
{
public:
	inline virtual QString GetModelType() const
	{
		return CARMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(CARMODEL_NAME) == 0 || (bDescendSufficient && Model::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	CarModel(const QString & strModelName = QString::null);
	CarModel(const CarModel & copy);
	virtual ~CarModel();

	virtual CarModel & operator = (const CarModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int ProcessEvent(SimEvent & event);
	virtual int PostRun();
	virtual int Save(std::map<QString, QString> & mapParams);
	virtual int Cleanup();

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	inline virtual struct timeval GetDelay() const
	{
		return m_tDelay;
	}
	inline virtual void SetDelay(const struct timeval & tDelay)
	{
		m_tDelay = tDelay;
	}
	inline virtual struct timeval GetTimestamp() const
	{
		return m_tTimestamp;
	}
	inline virtual in_addr_t GetIPAddress() const
	{
		return m_ipCar;
	}
	inline virtual in_addr_t GetOwnerIPAddress() const
	{
		return m_ipOwner;
	}
	inline virtual void SetOwnerIPAddress(in_addr_t ipOwner)
	{
		m_ipOwner = ipOwner;
	}
	inline virtual bool IsLoggingEnabled() const
	{
		return m_bLogThisCar;
	}
	inline virtual Coords GetCurrentPosition() const
	{
		return m_ptPosition;
	}
	inline virtual short GetCurrentSpeed() const
	{
		return m_iSpeed;
	}
	inline virtual short GetCurrentDirection() const
	{
		return m_iHeading;
	}
	inline virtual unsigned int GetCurrentRecord() const
	{
		return m_iCurrentRecord;
	}
	inline virtual bool IsGoingForwards() const
	{
		return m_bForwards;
	}
	inline virtual unsigned short GetCRShapePoint() const
	{
		return m_iCRShapePoint;
	}
	inline virtual float GetCRProgress() const
	{
		return m_fCRProgress;
	}
	inline virtual unsigned char GetLane() const
	{
		return m_iLane;
	}
	inline virtual int GetMapObjectID() const
	{
		return m_iMapObjectID;
	}
	virtual QString GetCarListColumnText(CarListColumn eColumn) const;
	virtual bool IsActive() const = 0;
	virtual void CreateMessage(Packet * msg);
	virtual void TransmitPacket(const Packet * packet);
	virtual bool ReceivePacket(Packet * packet);
	virtual unsigned int GetNextSequenceNumber();
	virtual unsigned int GetNextRXSequenceNumber();

	inline virtual std::map<in_addr_t, SafetyPacket> * GetKnownVehicles(bool bWait = true)
	{
		if (bWait)
		{
			m_mutexMessages.lock();
			return &m_mapKnownVehicles;
		} else
			return m_mutexMessages.tryLock() ? &m_mapKnownVehicles : NULL;
	}
	inline virtual void ReleaseKnownVehicles()
	{
		m_mutexMessages.unlock();
	}
	inline bool HasMessage(const PacketSequence & ID) const
	{
		return m_pLinkModel != NULL && m_pLinkModel->HasReceivedPacket(ID);
	}

	CarLinkModel * m_pLinkModel;
	CarPhysModel * m_pPhysModel;
	CarCommModel * m_pCommModel;

protected:
	struct timeval m_tDelay;
	struct timeval m_tTimestamp;
	in_addr_t m_ipCar;
	in_addr_t m_ipOwner;
	bool m_bLogThisCar;
	Coords m_ptPosition;
	short m_iSpeed;
	short m_iHeading;
	unsigned int m_iCurrentRecord;
	bool m_bForwards;
	unsigned short m_iCRShapePoint;
	float m_fCRProgress;
	unsigned char m_iLane;

	int m_iMapObjectID;

	std::map<in_addr_t, SafetyPacket> m_mapKnownVehicles;
	QMutex m_mutexMessages;
	unsigned int m_iNextSeqNumber, m_iNextRXSeqNumber;
	QMutex m_mutexSeqNumber, m_mutexRXSeqNumber;
	NeighborMessage m_msgNeighbors;

	bool m_bTrackSpeed;
	std::map<unsigned int, CongestionInfo> m_mapTracks;
};

inline bool CompareCarsByRecordProgressForwards(CarModel * x, CarModel * y)
{
	unsigned short xShapePoint, yShapePoint;
	xShapePoint = x->GetCRShapePoint();
	yShapePoint = y->GetCRShapePoint();
	return xShapePoint < yShapePoint || (xShapePoint == yShapePoint && x->GetCRProgress() < y->GetCRProgress());
}

inline bool CompareCarsByRecordProgressBackwards(CarModel * x, CarModel * y)
{
	unsigned short xShapePoint, yShapePoint;
	xShapePoint = x->GetCRShapePoint();
	yShapePoint = y->GetCRShapePoint();
	return xShapePoint > yShapePoint || (xShapePoint == yShapePoint && x->GetCRProgress() > y->GetCRProgress());
}


class MapCarObject : public MapObject
{
public:
	MapCarObject(CarModel * pCar);
	MapCarObject(const MapCarObject & copy);
	virtual ~MapCarObject();

	virtual MapCarObject & operator = (const MapCarObject & copy);

	inline virtual QString GetDescription() const
	{
		return m_pCar != NULL ? m_pCar->GetCarListColumnText(CarListColumnName) : MapObject::GetDescription();
	}
	virtual void DrawObject(const QRect & rBox, QPainter * pPainter, MapObjectState eState) const;
	virtual void DrawObject(MapDrawingSettings * pSettings, MapObjectState eState) const;
	virtual QRect RectVisible(MapDrawingSettings * pSettings) const;

	virtual struct timeval GetLastModifiedTime() const;
	inline virtual QRgb SetColor(QRgb color)
	{
		QRgb ret = m_clrDraw.rgb();
		m_clrDraw.setRgb(color);
		return ret;
	}
	inline virtual QRgb GetColor() const
	{
		return m_clrDraw.rgb();
	}
	virtual bool isActive() const;
	virtual bool hasReceivedCurrentMsg() const;

//protected:
	CarModel * m_pCar;
	QColor m_clrDraw;
};

#endif

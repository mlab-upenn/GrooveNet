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

#ifndef _INFRASTRUCTURENODEMODEL_H
#define _INFRASTRUCTURENODEMODEL_H

#include "CarModel.h"

#define INFRASTRUCTURENODEMODEL_NAME "InfrastructureNodeModel"

#define INFRASTRUCTURENODEMODEL_PARAM_NODEIP "ID"

class InfrastructureNodeModel : public Model
{
public:
	inline virtual QString GetModelType() const
	{
		return INFRASTRUCTURENODEMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(INFRASTRUCTURENODEMODEL_NAME) == 0 || (bDescendSufficient && Model::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	InfrastructureNodeModel(const QString & strModelName = QString::null);
	InfrastructureNodeModel(const InfrastructureNodeModel & copy);
	virtual ~InfrastructureNodeModel();

	virtual InfrastructureNodeModel & operator = (const InfrastructureNodeModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int ProcessEvent(SimEvent & event);
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
		return m_ipNode;
	}
	inline virtual Coords GetCurrentPosition() const
	{
		return m_ptPosition;
	}
	inline virtual unsigned int GetCurrentRecord() const
	{
		return m_iCurrentRecord;
	}
	inline virtual unsigned short GetCRShapePoint() const
	{
		return m_iCRShapePoint;
	}
	inline virtual float GetCRProgress() const
	{
		return m_fCRProgress;
	}
	inline virtual int GetMapObjectID() const
	{
		return m_iMapObjectID;
	}
	virtual QString GetCarListColumnText(CarListColumn eColumn) const;
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
	in_addr_t m_ipNode;
	Coords m_ptPosition;
	unsigned int m_iCurrentRecord;
	unsigned int m_iCRShapePoint;
	float m_fCRProgress;

	int m_iMapObjectID;

	std::map<in_addr_t, SafetyPacket> m_mapKnownVehicles;
	QMutex m_mutexMessages;
	unsigned int m_iNextSeqNumber, m_iNextRXSeqNumber;
	QMutex m_mutexSeqNumber, m_mutexRXSeqNumber;
	Address m_sPosition;
	QString m_strPosition;
};


class MapInfrastructureNodeObject : public MapObject
{
public:
	MapInfrastructureNodeObject(InfrastructureNodeModel * pNode);
	MapInfrastructureNodeObject(const MapInfrastructureNodeObject & copy);
	virtual ~MapInfrastructureNodeObject();

	virtual MapInfrastructureNodeObject & operator = (const MapInfrastructureNodeObject & copy);

	inline virtual QString GetDescription() const
	{
		return m_pNode != NULL ? m_pNode->GetCarListColumnText(CarListColumnName) : MapObject::GetDescription();
	}
	virtual void DrawObject(const QRect & rBox, QPainter * pPainter, MapObjectState eState) const;
	virtual void DrawObject(MapDrawingSettings * pSettings, MapObjectState eState) const;
	virtual QRect RectVisible(MapDrawingSettings * pSettings) const;

	virtual struct timeval GetLastModifiedTime() const;
	virtual bool isActive() const;

protected:
	InfrastructureNodeModel * m_pNode;
};

inline Model * InfrastructureNodeModelCreator(const QString & strModelName)
{
	return new InfrastructureNodeModel(strModelName);
}

#endif

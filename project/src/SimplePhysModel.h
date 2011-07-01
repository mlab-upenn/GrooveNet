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

#ifndef _SIMPLEPHYSMODEL
#define _SIMPLEPHYSMODEL

#include "CarModel.h"

#define SIMPLEPHYSMODEL_NAME "SimplePhysModel"

class SimplePhysModel : public CarPhysModel
{
public:
	inline virtual QString GetModelType() const
	{
		return SIMPLEPHYSMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(SIMPLEPHYSMODEL_NAME) == 0 || (bDescendSufficient && CarPhysModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	SimplePhysModel(const QString & strModelName = QString::null);
	SimplePhysModel(const SimplePhysModel & copy);
	virtual ~SimplePhysModel();

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int Save(std::map<QString, QString> & mapParams);

	virtual SimplePhysModel & operator = (const SimplePhysModel & copy);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual bool EndProcessPacket(Packet * packet);
	virtual bool BeginProcessPacket(Packet * packet);
	virtual bool ReceivePacket(Packet * packet);
	inline virtual float GetRXRange(const Packet * packet __attribute__((unused)) ) const
	{
		return m_fDistanceThreshold;
	}
	virtual bool IsCarInRange(const Coords & ptCar, const Coords & ptPosition) const;
	inline virtual unsigned int GetMessageCount() const
	{
		return m_iMessages;
	}
	inline virtual unsigned int GetMessageChannel(const Packet * packet) const
	{
		switch (packet->m_ePacketType)
		{
		case ptSafety:
			//TODO: fix this -MH
			//return ((const SafetyPacket *)packet)->m_eType + 1;
		case ptSquelch:
			return 6;
		default:
			return 0;
		}
	}

protected:
	float m_fDistanceThreshold;
	unsigned int m_iMessages;
	bool m_bMultichannel;
};

inline Model * SimplePhysModelCreator(const QString & strModelName)
{
	return new SimplePhysModel(strModelName);
}

#endif

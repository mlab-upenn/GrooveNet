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

#ifndef _COLLISIONPHYSMODEL
#define _COLLISIONPHYSMODEL

#include "SimplePhysModel.h"

#define COLLISIONPHYSMODEL_NAME "CollisionPhysModel"

typedef std::pair<struct timeval, struct timeval> TimeInterval;

class CollisionPhysModel : public SimplePhysModel
{
public:
	inline virtual QString GetModelType() const
	{
		return COLLISIONPHYSMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(COLLISIONPHYSMODEL_NAME) == 0 || (bDescendSufficient && SimplePhysModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	CollisionPhysModel(const QString & strModelName = QString::null);
	CollisionPhysModel(const CollisionPhysModel & copy);
	virtual ~CollisionPhysModel();

	virtual CollisionPhysModel & operator = (const CollisionPhysModel & copy);

	virtual int PreRun();

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual bool EndProcessPacket(Packet * packet);
	virtual bool BeginProcessPacket(Packet * packet);
	inline virtual unsigned int GetCollisionCount() const
	{
		return m_iCollisions;
	}

protected:
	std::map<RXPacketSequence, bool> m_mapMessages[PACKETMESSAGE_TYPENUM];
	unsigned int m_iCollisions;
};

inline Model * CollisionPhysModelCreator(const QString & strModelName)
{
	return new CollisionPhysModel(strModelName);
}

#endif

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

#ifndef _INFRASTRUCTURENODEREGISTRY_H
#define _INFRASTRUCTURENODEREGISTRY_H

#include "InfrastructureNodeModel.h"

class InfrastructureNodeRegistry
{
public:
	InfrastructureNodeRegistry();
	~InfrastructureNodeRegistry();

	inline std::map<in_addr_t, InfrastructureNodeModel *> * acquireLock(bool bWait = true)
	{
		if (bWait) {
			m_mutexRegistry.lock();
			return &m_mapRegistry;
		} else
			return m_mutexRegistry.tryLock() ? &m_mapRegistry : NULL;
	}
	inline std::map<in_addr_t, InfrastructureNodeModel *> * getRegistry()
	{
		return &m_mapRegistry;
	}
	inline void releaseLock()
	{
		m_mutexRegistry.unlock();
	}

	inline void addNode(InfrastructureNodeModel * pNode)
	{
		m_mutexRegistry.lock();
		m_mapRegistry.insert(std::pair<in_addr_t, InfrastructureNodeModel *>(pNode->GetIPAddress(), pNode));
		m_mutexRegistry.unlock();
	}
	inline bool removeNode(in_addr_t ipNode)
	{
		bool bRemoved;
		m_mutexRegistry.lock();
		bRemoved = m_mapRegistry.erase(ipNode) > 0;
		m_mutexRegistry.unlock();
		return bRemoved;
	}

protected:
	std::map<in_addr_t, InfrastructureNodeModel *> m_mapRegistry;
	QMutex m_mutexRegistry;

private:
	inline InfrastructureNodeRegistry(const InfrastructureNodeRegistry & copy __attribute__ ((unused)) ) {}
	inline InfrastructureNodeRegistry & operator = (const InfrastructureNodeRegistry & copy __attribute__ ((unused)) ) {return *this;}
};

extern InfrastructureNodeRegistry * g_pInfrastructureNodeRegistry;

#endif

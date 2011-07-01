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

#include "SimBase.h"

SimEvent::SimEvent(struct timeval tTimestamp, unsigned int iPriority, const QString & strModelOrg, const QString & strModelDest, unsigned int iEventID, void * pData, DestroyEventData pfnDestroy)
: m_tTimestamp(tTimestamp), m_iPriority(iPriority), m_strModelOrg(strModelOrg), m_strModelDest(strModelDest), m_iEventID(iEventID), m_pData(pData), m_pfnDestroy(pfnDestroy)
{
}

SimEvent::SimEvent(const SimEvent & copy)
: m_tTimestamp(copy.m_tTimestamp), m_iPriority(copy.m_iPriority), m_strModelOrg(copy.m_strModelOrg), m_strModelDest(copy.m_strModelDest), m_iEventID(copy.m_iEventID), m_pData(copy.m_pData), m_pfnDestroy(copy.m_pfnDestroy)
{
}

SimEvent::~SimEvent()
{
}

SimEvent & SimEvent::operator = (const SimEvent & copy)
{
	m_tTimestamp = copy.m_tTimestamp;
	m_iPriority = copy.m_iPriority;
	m_strModelOrg = copy.m_strModelOrg;
	m_strModelDest = copy.m_strModelDest;
	m_iEventID = copy.m_iEventID;
	m_pData = copy.m_pData;
	m_pfnDestroy = copy.m_pfnDestroy;
	return *this;
}


SimEventQueue::SimEventQueue()
: m_mutexQueue(true)
{
}

SimEventQueue::SimEventQueue(const SimEventQueue & copy)
: m_vecEvents(copy.m_vecEvents), m_mutexQueue(true)
{
}

SimEventQueue::~SimEventQueue()
{
	Clear();
}

SimEventQueue & SimEventQueue::operator = (const SimEventQueue & copy)
{
	m_vecEvents = copy.m_vecEvents;
	return *this;
}

void SimEventQueue::ClearUntil(struct timeval tTimestamp)
{
	m_mutexQueue.lock();
	while (!IsEmpty() && m_vecEvents.front().m_tTimestamp < tTimestamp)
	{
		if (m_vecEvents.front().m_pfnDestroy != NULL && m_vecEvents.front().GetEventData() != NULL)
			(*m_vecEvents.front().m_pfnDestroy)(m_vecEvents.front().GetEventData());
		pop_heap(m_vecEvents.begin(), m_vecEvents.end(), SimEventReverseCompare);
		m_vecEvents.pop_back();
	}
	m_mutexQueue.unlock();
}

void SimEventQueue::Clear()
{
	unsigned int i;
	m_mutexQueue.lock();
	for (i = 0; i < Count(); i++)
	{
		if (m_vecEvents[i].m_pfnDestroy != NULL && m_vecEvents[i].GetEventData() != NULL)
			(*m_vecEvents[i].m_pfnDestroy)(m_vecEvents[i].GetEventData());
	}
	m_vecEvents.clear();
	m_mutexQueue.unlock();
}

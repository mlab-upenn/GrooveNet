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

#ifndef _SIMBASE_H
#define _SIMBASE_H

#include <vector>
#include <qstring.h>
#include <qmutex.h>
#include <algorithm>

#include "Global.h"

#define EVENT_PRIORITY_HIGHEST 0
#define EVENT_PRIORITY_LOWEST (unsigned)-1

#define EVENT_GENERIC 0

using namespace std;

class SimEventQueue;

typedef void (* DestroyEventData)(void *);

class SimEvent
{
public:
	SimEvent(struct timeval tTimestamp, unsigned int iPriority = EVENT_PRIORITY_HIGHEST, const QString & strModelOrg = QString::null, const QString & strModelDest = QString::null, unsigned int iEventID = EVENT_GENERIC, void * pData = NULL, DestroyEventData pfnDestroy = NULL);
	SimEvent(const SimEvent & copy);
	~SimEvent();

	SimEvent & operator = (const SimEvent & copy);

	inline struct timeval GetTimestamp() const
	{
		return m_tTimestamp;
	}
	inline void SetTimestamp(struct timeval tTimestamp)
	{
		m_tTimestamp = tTimestamp;
	}
	inline unsigned int GetPriority() const
	{
		return m_iPriority;
	}
	inline const QString & GetOriginModel() const
	{
		return m_strModelOrg;
	}
	inline const QString & GetDestModel() const
	{
		return m_strModelDest;
	}
	inline unsigned int GetEventID() const
	{
		return m_iEventID;
	}
	inline const void * GetEventData() const
	{
		return m_pData;
	}
	inline void * GetEventData()
	{
		return m_pData;
	}

protected:
	struct timeval m_tTimestamp;
	unsigned int m_iPriority;
	QString m_strModelOrg;
	QString m_strModelDest;
	unsigned int m_iEventID;
	void * m_pData;
	DestroyEventData m_pfnDestroy;

	friend class SimEventQueue;
};

inline bool operator < (const SimEvent & x, const SimEvent & y)
{
	return x.GetTimestamp() < y.GetTimestamp() || (x.GetTimestamp() == y.GetTimestamp() && x.GetPriority() < y.GetPriority());
}

inline bool SimEventReverseCompare(const SimEvent & x, const SimEvent & y)
{
	return y.GetTimestamp() < x.GetTimestamp() || (x.GetTimestamp() == y.GetTimestamp() && y.GetPriority() < x.GetPriority());
}


class SimEventQueue
{
public:
	SimEventQueue();
	SimEventQueue(const SimEventQueue & copy);
	~SimEventQueue();

	SimEventQueue & operator = (const SimEventQueue & copy);

	inline void AddEvent(const SimEvent & event)
	{
		m_mutexQueue.lock();
		m_vecEvents.push_back(event);
		push_heap(m_vecEvents.begin(), m_vecEvents.end(), SimEventReverseCompare);
		m_mutexQueue.unlock();
	}
	inline const SimEvent & TopEvent() const
	{
		return m_vecEvents.front();
	}
	inline SimEvent & TopEvent()
	{
		return m_vecEvents.front();
	}
	inline void PopEvent()
	{
		m_mutexQueue.lock();
		pop_heap(m_vecEvents.begin(), m_vecEvents.end(), SimEventReverseCompare);
		m_vecEvents.pop_back();
		m_mutexQueue.unlock();
	}
	inline bool IsEmpty() const
	{
		return m_vecEvents.empty();
	}
	inline std::vector<SimEvent>::size_type Count() const
	{
		return m_vecEvents.size();
	}
	void ClearUntil(struct timeval tTimestamp);
	void Clear();

protected:
	std::vector<SimEvent> m_vecEvents;
	QMutex m_mutexQueue;
};

#endif

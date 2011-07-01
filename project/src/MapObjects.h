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

#ifndef _MAPOBJECTS_H
#define _MAPOBJECTS_H

#include <qstring.h>

#include <stdlib.h>
#include <map>

#include "MapDB.h"

typedef enum MapObjectStateEnum
{
	MapObjectStateNormal = 0,
	MapObjectStateCurrent = 1,
	MapObjectStateActive = 0,
	MapObjectStateInactive = 2
} MapObjectState;

class MapObject
{
public:
	MapObject(const QString & strName = QString::null);
	MapObject(const MapObject & copy);
	virtual ~MapObject();

	virtual MapObject & operator = (const MapObject & copy);

	virtual void DrawObject(const QRect & rBox, QPainter * pPainter, MapObjectState eState) const = 0;
	virtual void DrawObject(MapDrawingSettings * pSettings, MapObjectState eState) const = 0;
	virtual QRect RectVisible(MapDrawingSettings * pSettings) const = 0;

	virtual struct timeval GetLastModifiedTime() const = 0;
	inline virtual const QString & GetName() const
	{
		return m_strName;
	}
	inline virtual QString & GetName()
	{
		return m_strName;
	}
	inline virtual bool isActive() const
	{
		return true;
	}
	inline virtual QString GetDescription() const
	{
		return m_strName;
	}

protected:
	QString m_strName;
};

class MapObjects
{
public:
	MapObjects();
	~MapObjects();

	int add(MapObject * pObject);
	inline std::map<int, MapObject *> * acquireLock(bool bWait = true)
	{
		if (bWait) {
			m_mutexObjects.lock();
			return &m_mapObjects;
		} else
			return m_mutexObjects.tryLock() ? &m_mapObjects : NULL;
	}
	inline void releaseLock()
	{
		m_mutexObjects.unlock();
	}
	// the following two functions are not thread-safe
	// use the acquireLock() function and operate directly on the
	// returned map instead!
	MapObject * get(const int id);
	MapObject * remove(const int id);
	void destroy(const int id);
	void clear();

	void RedrawObjects(MapDrawingSettings * pSettings);

	QRgb GetColor();
	void UngetColor(QRgb color);

protected:
	std::map<int, MapObject *> m_mapObjects;
	QMutex m_mutexObjects;
	int m_iNextIndex;

	std::map<QRgb, unsigned int> m_mapColorCounters;

private:
	// copying not allowed
	inline MapObjects(const MapObjects & copy __attribute__ ((unused)) ) {}
	inline MapObjects & operator = (const MapObjects & copy __attribute__ ((unused)) ) {return *this;}
};

extern MapObjects * g_pMapObjects;

#endif

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

#include "MapObjects.h"

#define MAPOBJECTS_NUM_COLORS 9

//static const char * g_MapObjectColors[MAPOBJECTS_NUM_COLORS] = {"blue", "red", "green", "purple", "orange", "brown", "yellow", "light blue", "light green"};
static QRgb g_MapObjectColors[MAPOBJECTS_NUM_COLORS] = {qRgb(0, 0, 255), qRgb(255, 0, 0), qRgb(0, 255, 0), qRgb(160, 32, 240), qRgb(255, 165, 0), qRgb(165, 42, 42), qRgb(255, 255, 0), qRgb(173, 216, 230), qRgb(144, 238, 144)};

MapObject::MapObject(const QString & strName)
: m_strName(strName)
{
}

MapObject::MapObject(const MapObject & copy)
: m_strName(copy.m_strName)
{
}

MapObject::~MapObject()
{
}

MapObject & MapObject::operator = (const MapObject & copy)
{
	m_strName = copy.m_strName;
	return *this;
}


MapObjects::MapObjects()
: m_iNextIndex(0)
{
	unsigned int i;
	for (i = 0; i < MAPOBJECTS_NUM_COLORS; i++)
		m_mapColorCounters.insert(std::pair<QRgb, unsigned int>(QColor(g_MapObjectColors[i]).rgb(), 0));
}

MapObjects::~MapObjects()
{
	clear();
}

int MapObjects::add(MapObject * pObject)
{
	int ret;
	m_mutexObjects.lock();
	ret = m_iNextIndex;
	if (m_mapObjects.insert(std::pair<int, MapObject *>(m_iNextIndex, pObject)).second)
		m_iNextIndex++;
	else
		ret = -1;
	m_mutexObjects.unlock();
	return ret;
}

MapObject * MapObjects::get(const int id)
{
	std::map<int, MapObject *>::iterator iter = m_mapObjects.find(id);
	if (iter == m_mapObjects.end())
		return NULL;
	else
		return iter->second;
}

MapObject * MapObjects::remove(const int id)
{
	std::map<int, MapObject *>::iterator iter = m_mapObjects.find(id);
	if (iter == m_mapObjects.end())
		return NULL;
	else
	{
		MapObject * pObject = iter->second;
		m_mapObjects.erase(iter);
		return pObject;
	}
}

void MapObjects::destroy(const int id)
{
	std::map<int, MapObject *>::iterator iter;
	m_mutexObjects.lock();
	iter = m_mapObjects.find(id);
	if (iter != m_mapObjects.end())
	{
		if (iter->second) delete iter->second;
		m_mapObjects.erase(iter);
	}
	m_mutexObjects.unlock();
}

void MapObjects::clear()
{
	std::map<int, MapObject *>::iterator iter;
	m_mutexObjects.lock();
	for (iter = m_mapObjects.begin(); iter != m_mapObjects.end(); ++iter)
		if (iter->second) delete iter->second;
	m_mapObjects.clear();
	m_mutexObjects.unlock();
}

void MapObjects::RedrawObjects(MapDrawingSettings * pSettings)
{
	MapObjectState eState;
	MapObject * pCurrentObject = NULL;
	std::map<int, MapObject *>::iterator iter;
	m_mutexObjects.lock();
	for (iter = m_mapObjects.begin(); iter != m_mapObjects.end(); ++iter)
	{
		if (iter->first == pSettings->iCurrentObject)
			pCurrentObject = iter->second;
		else if (iter->second)
		{
			eState = iter->second->isActive() ? MapObjectStateActive: MapObjectStateInactive;
			iter->second->DrawObject(pSettings, eState);
		}
	}
	if (pCurrentObject != NULL)
	{
		eState = (MapObjectState)((pCurrentObject->isActive() ? MapObjectStateActive: MapObjectStateInactive) | MapObjectStateCurrent);
		pCurrentObject->DrawObject(pSettings, eState);
	}
	m_mutexObjects.unlock();
}

QRgb MapObjects::GetColor()
{
	unsigned int i;
	std::map<QRgb, unsigned int>::iterator iterColor, iterMin = m_mapColorCounters.end();
	for (i = 0; i < MAPOBJECTS_NUM_COLORS; i++)
	{
		iterColor = m_mapColorCounters.find(g_MapObjectColors[i]);
		if (iterMin == m_mapColorCounters.end() || iterColor->second < iterMin->second)
			iterMin = iterColor;
	}
	if (iterMin == m_mapColorCounters.end())
		return 0;
	else {
		iterMin->second++;
		return iterMin->first;
	}
}

void MapObjects::UngetColor(QRgb color)
{
	std::map<QRgb, unsigned int>::iterator iter = m_mapColorCounters.find(color);
	if (iter != m_mapColorCounters.end())
		iter->second--;
}

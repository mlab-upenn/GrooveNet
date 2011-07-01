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

#include "QMapObjectTableItem.h"

#include "MapObjects.h"

QMapObjectTableItem::QMapObjectTableItem(QTable * table, EditType ed, int iID)
: QTableItem(table, ed), m_iID(iID)
{
}

QMapObjectTableItem::~QMapObjectTableItem()
{
}

void QMapObjectTableItem::paint(QPainter * p, const QColorGroup & cg, const QRect & cr __attribute__ ((unused)), bool selected)
{
	std::map<int, MapObject *> * pMapObjects;
	std::map<int, MapObject *>::iterator iterMapObject;
	QRect rRect = table()->cellRect(row(), col());

	// update sprite for this car
	pMapObjects = g_pMapObjects->acquireLock();
	iterMapObject = pMapObjects->find(m_iID);
	p->setBackgroundColor(cg.color(selected ? QColorGroup::Highlight : QColorGroup::Base));
	p->eraseRect(rRect);
	if (iterMapObject != pMapObjects->end() && iterMapObject->second != NULL)
	{
		MapObjectState eState = iterMapObject->second->isActive() ? MapObjectStateActive : MapObjectStateInactive;

		if (selected)
			eState = (MapObjectState)(eState | MapObjectStateCurrent);
		iterMapObject->second->DrawObject(rRect, p, eState);
	}
	g_pMapObjects->releaseLock();
}

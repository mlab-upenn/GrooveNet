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

#ifndef _QMAPOBJECTTABLEITEM_H
#define _QMAPOBJECTTABLEITEM_H

#include <qtable.h>

#define MAPOBJECTTABLEITEM_RTTI_VALUE 1141512131

class QMapObjectTableItem : public QTableItem
{
public:
	QMapObjectTableItem(QTable * table, EditType et, int iID);
	virtual ~QMapObjectTableItem();

	virtual void paint(QPainter * p, const QColorGroup & cg, const QRect & cr, bool selected);

	inline virtual void SetID(int iID)
	{
		m_iID = iID;
	}
	inline virtual int GetID() const
	{
		return m_iID;
	}
	inline virtual int rtti() const
	{
		return MAPOBJECTTABLEITEM_RTTI_VALUE;
	}

protected:
	int m_iID;
};

#endif

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

#ifndef _QEXPANDABLETABLEITEM_H
#define _QEXPANDABLETABLEITEM_H

#include <qtable.h>
#include <vector>
#include "Settings.h"

#define EXPANDABLETABLEITEM_RTTI_VALUE 342151012

class QExpandableTableItem : public QTableItem
{
public:
	QExpandableTableItem(QTable * parent, const QString & strText = QString::null);
	virtual ~QExpandableTableItem();

	inline virtual int rtti() const { return EXPANDABLETABLEITEM_RTTI_VALUE; }

	inline virtual void AddChild(const QString & strKey, QTableItem * pItem)
	{
		m_mapChildren.insert(std::pair<QString, QTableItem *>(strKey, pItem));
	}
	inline virtual QTableItem * GetChild(const QString & strKey)
	{
		std::map<QString, QTableItem *>::iterator iterChild = m_mapChildren.find(strKey);
		return iterChild == m_mapChildren.end() ? NULL : iterChild->second;
	}
	inline virtual void RemoveChild(const QString & strKey)
	{
		m_mapChildren.erase(strKey);
	}
	virtual unsigned int ChildCount() const;
	virtual void ShowItem(bool bShown);
	virtual void Expand();
	virtual void Collapse();
	inline virtual bool IsExpanded() const
	{
		return m_bExpanded;
	}

protected:
	std::map<QString, QTableItem *> m_mapChildren;
	bool m_bExpanded;
};

#endif

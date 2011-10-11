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

#include "QExpandableTableItem.h"

QExpandableTableItem::QExpandableTableItem(QTable * parent, const QString & strText)
: QTableItem(parent, Never, strText), m_bExpanded(true)
{
	setReplaceable(false);
}

QExpandableTableItem::~QExpandableTableItem()
{
}

unsigned int QExpandableTableItem::ChildCount() const
{
	unsigned int iCount = m_mapChildren.size();
	std::map<QString, QTableItem *>::const_iterator iterChild;
	for (iterChild = m_mapChildren.begin(); iterChild != m_mapChildren.end(); ++iterChild)
	{
		if (iterChild->second->rtti() == rtti()) // same type
			iCount += ((QExpandableTableItem *)iterChild->second)->ChildCount();
	}
	return iCount;
}

void QExpandableTableItem::ShowItem(bool bShown)
{
	std::map<QString, QTableItem *>::iterator iterChild;

	if (m_bExpanded && bShown)
	{
		for (iterChild = m_mapChildren.begin(); iterChild != m_mapChildren.end(); ++iterChild)
		{
			table()->showRow(iterChild->second->row());
			if (iterChild->second->rtti() == rtti()) // same type
				((QExpandableTableItem *)iterChild->second)->ShowItem(bShown);
		}
	}
	else
	{
		for (iterChild = m_mapChildren.begin(); iterChild != m_mapChildren.end(); ++iterChild)
		{
			table()->hideRow(iterChild->second->row());
			if (iterChild->second->rtti() == rtti()) // same type
				((QExpandableTableItem *)iterChild->second)->ShowItem(false);
		}
	}
}

void QExpandableTableItem::Expand()
{
	std::map<QString, QTableItem *>::iterator iterChild;

	m_bExpanded = true;
	for (iterChild = m_mapChildren.begin(); iterChild != m_mapChildren.end(); ++iterChild)
	{
		table()->showRow(iterChild->second->row());
		if (iterChild->second->rtti() == rtti()) // same type
			((QExpandableTableItem *)iterChild->second)->ShowItem(true);
	}
}

void QExpandableTableItem::Collapse()
{
	std::map<QString, QTableItem *>::iterator iterChild;

	m_bExpanded = false;
	for (iterChild = m_mapChildren.begin(); iterChild != m_mapChildren.end(); ++iterChild)
	{
		table()->hideRow(iterChild->second->row());
		if (iterChild->second->rtti() == rtti()) // same type
			((QExpandableTableItem *)iterChild->second)->ShowItem(false);
	}
}


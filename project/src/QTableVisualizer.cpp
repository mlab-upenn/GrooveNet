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

#include "QTableVisualizer.h"

QDraggingTable::QDraggingTable(QWidget * parent, const char * name)
: QTable(parent, name), m_pfnDragObjectCreator(NULL), m_pTableVisualizer(NULL)
{
}

QDraggingTable::QDraggingTable(int numRows, int numCols, QWidget * parent, const char * name)
: QTable(numRows, numCols, parent, name), m_pfnDragObjectCreator(NULL), m_pTableVisualizer(NULL)
{
}

QDraggingTable::~QDraggingTable()
{
}


QTableVisualizer::QTableVisualizer(TableVisualizer * pVisualizer, QWidget * parent, const char * name, Qt::WFlags f)
: QVisualizer(pVisualizer, parent, name, f)
{
	m_pLayout = new QGridLayout(this, 1, 1, 0, -1, "layout");
	m_pTable = new QDraggingTable(this, "table");
	m_pLayout->addWidget(m_pTable, 0, 0);
	connect(m_pTable, SIGNAL(currentChanged(int, int)), this, SLOT(slotCurrentChanged(int, int)));
	connect(m_pTable, SIGNAL(clicked(int, int, int, const QPoint& )), this, SLOT(slotClicked(int, int, int, const QPoint& )));
	connect(m_pTable, SIGNAL(doubleClicked(int, int, int, const QPoint& )), this, SLOT(slotDoubleClicked(int, int, int, const QPoint& )));
	connect(m_pTable, SIGNAL(pressed(int, int, int, const QPoint& )), this, SLOT(slotPressed(int, int, int, const QPoint& )));
	connect(m_pTable, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
	connect(m_pTable, SIGNAL(valueChanged(int, int)), this, SLOT(slotValueChanged(int, int)));
	connect(m_pTable, SIGNAL(contextMenuRequested(int, int, const QPoint& )), this, SLOT(slotContextMenuRequested(int, int, const QPoint& )));
	connect(m_pTable, SIGNAL(dropped(QDropEvent* )), this, SLOT(slotDropped(QDropEvent* )));
	m_pTable->show();
}

QTableVisualizer::~QTableVisualizer()
{
}



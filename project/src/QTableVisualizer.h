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

#ifndef _QTABLEVISUALIZER_H
#define _QTABLEVISUALIZER_H

#include <qtable.h>
#include <qlayout.h>
#include <qobject.h>
#include <qdragobject.h>

#include "QVisualizer.h"
#include "TableVisualizer.h"

typedef QDragObject * (* DragObjectCreator)(TableVisualizer *);

class QDraggingTable : public QTable
{
Q_OBJECT
public:
	QDraggingTable(QWidget * parent = 0, const char * name = 0);
	QDraggingTable(int numRows, int numCols, QWidget * parent = 0, const char * name = 0);
	virtual ~QDraggingTable();

	inline virtual void SetDragObjectCreator(DragObjectCreator pfnDragObjectCreator, TableVisualizer * pTableVisualizer)
	{
		m_pfnDragObjectCreator = pfnDragObjectCreator;
		m_pTableVisualizer = pTableVisualizer;
	}
	inline virtual DragObjectCreator GetDragObjectCreator() const
	{
		return m_pfnDragObjectCreator;
	}

protected:
	inline virtual QDragObject * dragObject()
	{
		return m_pfnDragObjectCreator == NULL ? NULL : (*m_pfnDragObjectCreator)(m_pTableVisualizer);
	}

	DragObjectCreator m_pfnDragObjectCreator;
	TableVisualizer * m_pTableVisualizer;
};

class QTableVisualizer : public QVisualizer
{
Q_OBJECT
public:
	QTableVisualizer(TableVisualizer * pVisualizer, QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);
	virtual ~QTableVisualizer();

	QGridLayout * m_pLayout;
	QDraggingTable * m_pTable;

protected slots:
	inline virtual void slotCurrentChanged(int row, int col) { if (m_pVisualizer) ((TableVisualizer *)m_pVisualizer)->tableCurrentChanged(row, col); }
	inline virtual void slotClicked(int row, int col, int button, const QPoint & mousePos) { if (m_pVisualizer) ((TableVisualizer *)m_pVisualizer)->tableClicked(row, col, button, mousePos); }
	inline virtual void slotDoubleClicked(int row, int col, int button, const QPoint & mousePos) { if (m_pVisualizer) ((TableVisualizer *)m_pVisualizer)->tableDoubleClicked(row, col, button, mousePos); }
	inline virtual void slotPressed(int row, int col, int button, const QPoint & mousePos) { if (m_pVisualizer) ((TableVisualizer *)m_pVisualizer)->tablePressed(row, col, button, mousePos); }
	inline virtual void slotSelectionChanged() { if (m_pVisualizer) ((TableVisualizer *)m_pVisualizer)->tableSelectionChanged(); }
	inline virtual void slotValueChanged(int row, int col) { if (m_pVisualizer) ((TableVisualizer *)m_pVisualizer)->tableValueChanged(row, col); }
	inline virtual void slotContextMenuRequested(int row, int col, const QPoint & pos) { if (m_pVisualizer) ((TableVisualizer *)m_pVisualizer)->tableContextMenuRequested(row, col, pos); }
	inline virtual void slotDropped(QDropEvent * e) { if (m_pVisualizer) ((TableVisualizer *)m_pVisualizer)->tableDropped(e); }

};

#endif


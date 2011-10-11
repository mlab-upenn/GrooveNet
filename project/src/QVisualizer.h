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
#ifndef _QVISUALIZER_H
#define _QVISUALIZER_H

#include <qwidget.h>

#include "Visualizer.h"

/**
@author Daniel Weller
*/
class QVisualizer : public QWidget
{
Q_OBJECT
public:
	QVisualizer(Visualizer * pVisualizer, QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);
	~QVisualizer();

protected:
	virtual void closeEvent(QCloseEvent * e);
	inline virtual void contextMenuEvent(QContextMenuEvent * e) {if (m_pVisualizer) m_pVisualizer->contextMenuEvent(e);}
	inline virtual void dragEnterEvent(QDragEnterEvent * e) {if (m_pVisualizer) m_pVisualizer->dragEnterEvent(e);}
	inline virtual void dragLeaveEvent(QDragLeaveEvent * e) {if (m_pVisualizer) m_pVisualizer->dragLeaveEvent(e);}
	inline virtual void dragMoveEvent(QDragMoveEvent * e) {if (m_pVisualizer) m_pVisualizer->dragMoveEvent(e);}
	inline virtual void dropEvent(QDropEvent * e) {if (m_pVisualizer) m_pVisualizer->dropEvent(e);}
	inline virtual void enterEvent(QEvent * e) {if (m_pVisualizer) m_pVisualizer->enterEvent(e);}
	inline virtual void focusInEvent(QFocusEvent * e) {if (m_pVisualizer) m_pVisualizer->focusInEvent(e);}
	inline virtual void focusOutEvent(QFocusEvent * e) {if (m_pVisualizer) m_pVisualizer->focusOutEvent(e);}
	inline virtual void hideEvent(QHideEvent * e) {if (m_pVisualizer) m_pVisualizer->hideEvent(e);}
	inline virtual void keyPressEvent(QKeyEvent * e) {if (m_pVisualizer) m_pVisualizer->keyPressEvent(e);}
	inline virtual void keyReleaseEvent(QKeyEvent * e) {if (m_pVisualizer) m_pVisualizer->keyReleaseEvent(e);}
	inline virtual void leaveEvent(QEvent * e) {if (m_pVisualizer) m_pVisualizer->leaveEvent(e);}
	inline virtual void mouseDoubleClickEvent(QMouseEvent * e) {if (m_pVisualizer) m_pVisualizer->mouseDoubleClickEvent(e);}
	inline virtual void mouseMoveEvent(QMouseEvent * e) {if (m_pVisualizer) m_pVisualizer->mouseMoveEvent(e);}
	inline virtual void mousePressEvent(QMouseEvent * e) {if (m_pVisualizer) m_pVisualizer->mousePressEvent(e);}
	inline virtual void mouseReleaseEvent(QMouseEvent * e) {if (m_pVisualizer) m_pVisualizer->mouseReleaseEvent(e);}
	inline virtual void moveEvent(QMoveEvent * e) {if (m_pVisualizer) m_pVisualizer->moveEvent(e);}
	inline virtual void paintEvent(QPaintEvent * e) {if (m_pVisualizer) m_pVisualizer->paintEvent(e);}
	inline virtual void resizeEvent(QResizeEvent * e) {if (m_pVisualizer) m_pVisualizer->resizeEvent(e);}
	inline virtual void showEvent(QShowEvent * e) {if (m_pVisualizer) m_pVisualizer->showEvent(e);}
	inline virtual void wheelEvent(QWheelEvent * e) {if (m_pVisualizer) m_pVisualizer->wheelEvent(e);}

	Visualizer * m_pVisualizer;
};

#endif

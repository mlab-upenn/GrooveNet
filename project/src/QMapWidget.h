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

#ifndef _QMAPWIDGET_H
#define _QMAPWIDGET_H

#include <qwidget.h>

#include "MapDB.h"

class QPopupMenu;

class QMapWidget : public QWidget
{
Q_OBJECT
public:
	typedef enum SelectionModeEnum
	{
		SelectionModeNone = 0,
		SelectionModeRect,
		SelectionModePoint,
		SelectionModePoints,
		SelectionModeRecords
	} SelectionMode;

	QMapWidget(QWidget * parent = NULL, const char * name = 0, WFlags f = 0);
	virtual ~QMapWidget();

	inline virtual void SetSelectionMode(SelectionMode eMode)
	{
		m_eSelectionMode = eMode;
	}
	inline virtual SelectionMode GetSelectionMode() const
	{
		return m_eSelectionMode;
	}

	inline virtual Rect GetSelectionRect() const
	{
		return Rect(m_ptSelectionBegin, m_ptSelectionEnd).normalize();
	}
	inline virtual void SetSelectionRect(const Rect & rSelection)
	{
		m_ptSelectionBegin.Set(rSelection.m_iLeft, rSelection.m_iTop);
		m_ptSelectionEnd.Set(rSelection.m_iRight, rSelection.m_iBottom);
		m_mutexSettings.lock();
		InvalidateRect(&m_sDrawSettings, QRect(), false, true);
		update(m_sDrawSettings.rUpdate);
		m_mutexSettings.unlock();
	}
	inline virtual Coords GetSelectionPoint() const
	{
		return m_ptSelectionEnd;
	}
	inline virtual void SetSelectionPoint(const Coords & ptSelection)
	{
		m_ptSelectionBegin = ptSelection;
		m_ptSelectionEnd = ptSelection;
		m_mutexSettings.lock();
		InvalidateRect(&m_sDrawSettings, QRect(), false, true);
		update(m_sDrawSettings.rUpdate);
		m_mutexSettings.unlock();
	}
	inline virtual std::list<Coords> GetSelectionPoints() const
	{
		return m_listPoints;
	}
	inline virtual void SetSelectionPoints(const std::list<Coords> & listPoints)
	{
		m_listPoints = listPoints;
		m_iterPoint = m_listPoints.end();
		m_mutexSettings.lock();
		InvalidateRect(&m_sDrawSettings, QRect(), false, true);
		update(m_sDrawSettings.rUpdate);
		m_mutexSettings.unlock();
	}
	inline virtual std::set<unsigned int> GetSelectionRecords() const
	{
		return m_setRecords;
	}
	inline virtual void SetSelectionRecords(const std::set<unsigned int> & setRecords)
	{
		m_setRecords = setRecords;
		m_mutexSettings.lock();
		InvalidateRect(&m_sDrawSettings, QRect(), false, true);
		update(m_sDrawSettings.rUpdate);
		m_mutexSettings.unlock();
	}

	inline MapDrawingSettings * GetSettings(bool bWait = true)
	{
		if (bWait) {
			m_mutexSettings.lock();
			return &m_sDrawSettings;
		} else
			return m_mutexSettings.tryLock() ? &m_sDrawSettings : NULL;
	}
	inline void ReleaseSettings()
	{
		m_mutexSettings.unlock();
	}

	inline virtual QSize sizeHint() const
	{
		return QSize(300, 300);
	}

	virtual void recenter(const Coords & ptCenter);
	virtual void moveCenter(float fScreensRight, float fScreensDown);
	virtual void zoom(int iZoomOut);
	virtual void rezoom(int iZoom);
	virtual bool processKey(int iQtKey, Qt::ButtonState eButtonState = Qt::NoButton);

signals:
	virtual void selectionChanged();

protected slots:
	virtual void paintEvent(QPaintEvent * e);
	virtual void resizeEvent(QResizeEvent * e);
	virtual void keyPressEvent(QKeyEvent * e);
	virtual void mousePressEvent(QMouseEvent * e);
	virtual void mouseMoveEvent(QMouseEvent * e);
	virtual void mouseReleaseEvent(QMouseEvent * e);
	virtual void mouseDoubleClickEvent(QMouseEvent * e);

protected:
	MapDrawingSettings m_sDrawSettings, m_sPaintSettings;
	QMutex m_mutexSettings;
	QPopupMenu * m_pRightClickMenu;
	SelectionMode m_eSelectionMode;
	Coords m_ptSelectionBegin, m_ptSelectionEnd;
	std::set<unsigned int> m_setRecords;
	std::list<Coords> m_listPoints;
	std::list<Coords>::iterator m_iterPoint;
	bool m_bAddRemoveRecords;
};

#endif

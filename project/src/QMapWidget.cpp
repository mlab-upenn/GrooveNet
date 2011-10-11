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

#include <qpopupmenu.h>

#include "QMapWidget.h"

#include "Settings.h"

#include <qinputdialog.h>
#include <qlineedit.h>

QMapWidget::QMapWidget(QWidget * parent, const char * name, WFlags f)
: QWidget(parent, name, f | Qt::WNoAutoErase), m_eSelectionMode(SelectionModeNone)
{
	m_sPaintSettings.tLastChange = timeval0;
	m_sPaintSettings.pOldMapBmp = NULL;
	m_sPaintSettings.pOldEverythingBmp = NULL;
	m_sPaintSettings.pMemoryDC = new QPainter();
	InitMapDrawingSettings(&m_sDrawSettings);
	m_pRightClickMenu = new QPopupMenu(this);
	m_pRightClickMenu->insertItem("Clear Selection", 0);
	m_pRightClickMenu->insertItem("Recenter", 1);
	m_pRightClickMenu->insertItem("Find Address...", 2);
	setFocusPolicy(QWidget::StrongFocus);
}

QMapWidget::~QMapWidget()
{
}

void QMapWidget::moveCenter(float fScreensRight, float fScreensDown)
{
	QPoint ptNew;

	m_mutexSettings.lock();
	ptNew.setX((int)((fScreensRight + 0.5f) * m_sDrawSettings.iControlWidth));
	ptNew.setY((int)((fScreensDown + 0.5f) * m_sDrawSettings.iControlHeight));
	m_sDrawSettings.ptCenter = MapScreenToLongLat(&m_sDrawSettings, ptNew);
	m_sDrawSettings.ptTopLeft = MapScreenToLongLat(&m_sDrawSettings, QPoint(0, 0));
	m_sDrawSettings.ptBottomRight = MapScreenToLongLat(&m_sDrawSettings, QPoint(m_sDrawSettings.iControlWidth, m_sDrawSettings.iControlHeight));
	InvalidateRect(&m_sDrawSettings);
	update(m_sDrawSettings.rUpdate);
	m_mutexSettings.unlock();
}

void QMapWidget::recenter(const Coords & ptCenter)
{
	m_mutexSettings.lock();
	m_sDrawSettings.ptCenter = ptCenter;
	m_sDrawSettings.ptTopLeft = MapScreenToLongLat(&m_sDrawSettings, QPoint(0, 0));
	m_sDrawSettings.ptBottomRight = MapScreenToLongLat(&m_sDrawSettings, QPoint(m_sDrawSettings.iControlWidth, m_sDrawSettings.iControlHeight));
	InvalidateRect(&m_sDrawSettings);
	update(m_sDrawSettings.rUpdate);
	m_mutexSettings.unlock();
}

void QMapWidget::zoom(int iZoomOut)
{
	m_mutexSettings.lock();
	m_sDrawSettings.iDetailLevel += iZoomOut;
	if (m_sDrawSettings.iDetailLevel < MIN_DETAIL_LEVEL)
		m_sDrawSettings.iDetailLevel = MIN_DETAIL_LEVEL;
	else if (m_sDrawSettings.iDetailLevel > MAX_DETAIL_LEVEL)
		m_sDrawSettings.iDetailLevel = MAX_DETAIL_LEVEL;
	m_sDrawSettings.ptTopLeft = MapScreenToLongLat(&m_sDrawSettings, QPoint(0, 0));
	m_sDrawSettings.ptBottomRight = MapScreenToLongLat(&m_sDrawSettings, QPoint(m_sDrawSettings.iControlWidth, m_sDrawSettings.iControlHeight));
	InvalidateRect(&m_sDrawSettings);
	update(m_sDrawSettings.rUpdate);
	m_mutexSettings.unlock();
}

void QMapWidget::rezoom(int iZoom)
{
	m_mutexSettings.lock();
	m_sDrawSettings.iDetailLevel = iZoom;
	if (m_sDrawSettings.iDetailLevel < MIN_DETAIL_LEVEL)
		m_sDrawSettings.iDetailLevel = MIN_DETAIL_LEVEL;
	else if (m_sDrawSettings.iDetailLevel > MAX_DETAIL_LEVEL)
		m_sDrawSettings.iDetailLevel = MAX_DETAIL_LEVEL;
	m_sDrawSettings.ptTopLeft = MapScreenToLongLat(&m_sDrawSettings, QPoint(0, 0));
	m_sDrawSettings.ptBottomRight = MapScreenToLongLat(&m_sDrawSettings, QPoint(m_sDrawSettings.iControlWidth, m_sDrawSettings.iControlHeight));
	InvalidateRect(&m_sDrawSettings);
	update(m_sDrawSettings.rUpdate);
	m_mutexSettings.unlock();
}

bool QMapWidget::processKey(int iQtKey, Qt::ButtonState eButtonState)
{
	switch (iQtKey)
	{
	case Qt::Key_Left:
		moveCenter(eButtonState & Qt::ControlButton ? -1.f : -0.25f, 0.f);
		return true;
	case Qt::Key_Up:
		moveCenter(0.f, eButtonState & Qt::ControlButton ? -1.f : -0.25f);
		return true;
	case Qt::Key_Right:
		moveCenter(eButtonState & Qt::ControlButton ? 1.f : 0.25f, 0.f);
		return true;
	case Qt::Key_Down:
		moveCenter(0.f, eButtonState & Qt::ControlButton ? 1.f : 0.25f);
		return true;
	case Qt::Key_Plus:
		zoom(-1);
		return true;
	case Qt::Key_Minus:
		zoom(1);
		return true;
	case Qt::Key_Equal:
		rezoom(4);
		return true;
	default:
		return false;
	}
}

void QMapWidget::keyPressEvent(QKeyEvent * e)
{
	bool bHandled = false;
	if (e->text().isEmpty())
	{
		// pressed key stored in key()
		bHandled = processKey(e->key(), e->state());
	}
	else
	{
		// generated text in text()
		unsigned int i;
		for (i = 0; i < e->text().length(); i++)
			bHandled |= processKey(e->text()[i].unicode(), e->state());
	}
	if (bHandled)
		e->accept();
	else
		e->ignore();
}

void QMapWidget::paintEvent(QPaintEvent * e)
{
	QPainter dc(this);
	QRect bounds;

	m_mutexSettings.lock();
	CopyMapDrawingSettings(&m_sPaintSettings, &m_sDrawSettings);
	m_mutexSettings.unlock();

	bounds = e->rect().unite(m_sPaintSettings.rUpdate);
	dc.setClipRect(bounds);

	if (m_sPaintSettings.pMemoryDC == NULL)
		return; // memory DC not active

	m_sPaintSettings.ptTopLeftClip = MapScreenToLongLat(&m_sPaintSettings, bounds.topLeft());
	m_sPaintSettings.ptBottomRightClip = MapScreenToLongLat(&m_sPaintSettings, bounds.bottomRight());

	if (g_pMapDB->GetLastChange() > m_sPaintSettings.tLastChange)
	{
		// if the data has changed, then we must redraw
		m_sPaintSettings.bL1Redraw = true;
		m_sPaintSettings.tLastChange = g_pMapDB->GetLastChange();
	}

	if ((m_sPaintSettings.pOldEverythingBmp == NULL || m_sPaintSettings.pOldMapBmp == NULL || m_sPaintSettings.pOldEverythingBmp->width() != m_sPaintSettings.iControlWidth || m_sPaintSettings.pOldEverythingBmp->height() != m_sPaintSettings.iControlHeight) && m_sPaintSettings.iControlWidth > 0 && m_sPaintSettings.iControlHeight > 0)
	{
		if (m_sPaintSettings.pMemoryDC->isActive())
			m_sPaintSettings.pMemoryDC->end();
		ReplaceOldEverythingBmp(&m_sPaintSettings, new QPixmap(m_sPaintSettings.iControlWidth, m_sPaintSettings.iControlHeight));
		ReplaceOldMapBmp(&m_sPaintSettings, new QPixmap(m_sPaintSettings.iControlWidth, m_sPaintSettings.iControlHeight));
		m_sPaintSettings.pMemoryDC->begin(m_sPaintSettings.pOldEverythingBmp, this);
		m_sPaintSettings.bL1Redraw = true;
	}
	if (m_sPaintSettings.pMemoryDC->isActive())
	{
		if (m_sPaintSettings.bL1Redraw) {
			g_pMapDB->DrawMap(&m_sPaintSettings, bounds);
			QPainter temp;
			temp.begin(m_sPaintSettings.pOldMapBmp, this);
			temp.fillRect(bounds, m_sPaintSettings.clrBackground);
			temp.drawPixmap(bounds.topLeft(), *m_sPaintSettings.pOldEverythingBmp, bounds);
			temp.end();
			m_sPaintSettings.bL2Redraw = true;
		}
		if (m_sPaintSettings.bL2Redraw) {
			m_sPaintSettings.pMemoryDC->drawPixmap(bounds.topLeft(), *m_sPaintSettings.pOldMapBmp, bounds);
			switch (m_eSelectionMode)
			{
			case SelectionModeRect:
			{
				QRect rFocus = QRect(MapLongLatToScreen(&m_sPaintSettings, m_ptSelectionBegin), MapLongLatToScreen(&m_sPaintSettings, m_ptSelectionEnd)).normalize();
				m_sPaintSettings.pMemoryDC->drawWinFocusRect(rFocus);
				break;
			}
			case SelectionModePoint:
			{
				QPoint ptFocus = MapLongLatToScreen(&m_sPaintSettings, m_ptSelectionEnd);
				QRect rFocus(ptFocus.x() - 5, ptFocus.y() - 5, 11, 11);
				m_sPaintSettings.pMemoryDC->setBrush(Qt::black);
				m_sPaintSettings.pMemoryDC->setPen(Qt::NoPen);
				m_sPaintSettings.pMemoryDC->drawPie(rFocus, 0, 5760);
				rFocus = QRect(ptFocus.x() - 4, ptFocus.y() - 4, 9, 9);
				m_sPaintSettings.pMemoryDC->setBrush(Qt::red);
				m_sPaintSettings.pMemoryDC->drawPie(rFocus, 0, 5760);
				break;
			}
			case SelectionModePoints:
			{
				std::list<Coords>::iterator iterPoints;
				for (iterPoints = m_listPoints.begin(); iterPoints != m_listPoints.end(); ++iterPoints)
				{
					QPoint ptFocus = MapLongLatToScreen(&m_sPaintSettings, *iterPoints);
					std::list<Coords>::iterator iterNextPoint = iterPoints;
					if ((++iterNextPoint) != m_listPoints.end())
					{
						QPoint pt2 = MapLongLatToScreen(&m_sPaintSettings, *iterNextPoint);
						m_sPaintSettings.pMemoryDC->setPen(QPen(Qt::red, 3, Qt::DashLine));
						m_sPaintSettings.pMemoryDC->drawLine(ptFocus, pt2);
					}

					QRect rFocus(ptFocus.x() - 5, ptFocus.y() - 5, 11, 11);
					m_sPaintSettings.pMemoryDC->setBrush(Qt::black);
					m_sPaintSettings.pMemoryDC->setPen(Qt::NoPen);
					m_sPaintSettings.pMemoryDC->drawPie(rFocus, 0, 5760);
					rFocus = QRect(ptFocus.x() - 4, ptFocus.y() - 4, 9, 9);
					m_sPaintSettings.pMemoryDC->setBrush(Qt::red);
					m_sPaintSettings.pMemoryDC->drawPie(rFocus, 0, 5760);
				}
				break;
			}
			case SelectionModeRecords:
			{
				g_pMapDB->DrawRecordHighlights(&m_sPaintSettings, m_setRecords, Qt::red);
				break;
			}
			default:
				break;
			}
		}
		g_pMapDB->DrawBorder(&m_sPaintSettings, hasFocus());
		m_sPaintSettings.bL1Redraw = false;
		m_sPaintSettings.bL2Redraw = false;
		m_sPaintSettings.rUpdate = QRect();
		dc.drawPixmap(bounds.topLeft(), *m_sPaintSettings.pOldEverythingBmp, bounds);
	}
	m_mutexSettings.lock();
	m_sDrawSettings.bL1Redraw = m_sPaintSettings.bL1Redraw;
	m_sDrawSettings.bL2Redraw = m_sPaintSettings.bL2Redraw;
	m_sDrawSettings.rUpdate = m_sPaintSettings.rUpdate;
	m_sDrawSettings.tLastChange = m_sPaintSettings.tLastChange;
	m_mutexSettings.unlock();
}

void QMapWidget::resizeEvent(QResizeEvent * e)
{
	m_mutexSettings.lock();
	m_sDrawSettings.iControlWidth = e->size().width();
	m_sDrawSettings.iControlHeight = e->size().height();
	m_sDrawSettings.ptTopLeft = MapScreenToLongLat(&m_sDrawSettings, QPoint(0, 0));
	m_sDrawSettings.ptBottomRight = MapScreenToLongLat(&m_sDrawSettings, QPoint(m_sDrawSettings.iControlWidth, m_sDrawSettings.iControlHeight));
	InvalidateRect(&m_sDrawSettings);
	update(m_sDrawSettings.rUpdate);
	m_mutexSettings.unlock();
}

void QMapWidget::mousePressEvent(QMouseEvent * e)
{
	if (e->button() == Qt::LeftButton)
	{
		switch (m_eSelectionMode)
		{
		case SelectionModeRect:
		case SelectionModePoint:
		{
			m_mutexSettings.lock();
			m_ptSelectionBegin = m_ptSelectionEnd = MapScreenToLongLat(&m_sDrawSettings, e->pos());
			emit selectionChanged();
			InvalidateRect(&m_sDrawSettings, QRect(), false, true);
			update(m_sDrawSettings.rUpdate);
			m_mutexSettings.unlock();
			break;
		}
		case SelectionModePoints:
		{
			m_mutexSettings.lock();
			m_ptSelectionBegin = m_ptSelectionEnd = MapScreenToLongLat(&m_sDrawSettings, e->pos());
			Coords ptTemp = MapScreenToLongLat(&m_sDrawSettings, e->pos() - QPoint(5, 0));
			double fMaxDistance = Distance(ptTemp, m_ptSelectionEnd), fDistance;
			std::list<Coords>::iterator iterPtChosen;
			m_iterPoint = m_listPoints.end();
			for (iterPtChosen = m_listPoints.begin(); iterPtChosen != m_listPoints.end(); ++iterPtChosen)
			{
				if ((fDistance = Distance(*iterPtChosen, m_ptSelectionEnd)) < fMaxDistance)
				{
					fMaxDistance = fDistance;
					m_iterPoint = iterPtChosen;
				}
			}
			if (m_iterPoint == m_listPoints.end())
			{
				// see where to insert a new point
				std::list<Coords>::iterator iterOldPt = iterPtChosen = m_listPoints.begin();
				float fProgress = 0.f;
				ptTemp = MapScreenToLongLat(&m_sDrawSettings, e->pos() - QPoint(3, 0));
				fMaxDistance = (ptTemp.m_iLong - m_ptSelectionEnd.m_iLong) * (ptTemp.m_iLong - m_ptSelectionEnd.m_iLong) + (ptTemp.m_iLat - m_ptSelectionEnd.m_iLat) * (ptTemp.m_iLat - m_ptSelectionEnd.m_iLat);
				if (iterPtChosen != m_listPoints.end())
					++iterPtChosen;
				while (iterPtChosen != m_listPoints.end())
				{
					if ((fDistance = PointSegmentDistance(m_ptSelectionEnd, *iterOldPt, *iterPtChosen, fProgress)) < fMaxDistance)
					{
						fMaxDistance = fDistance;
						m_iterPoint = iterPtChosen;
					}

					iterOldPt = iterPtChosen;
					++iterPtChosen;
				}
				m_iterPoint = m_listPoints.insert(m_iterPoint, m_ptSelectionEnd);
			}
			else
				*m_iterPoint = m_ptSelectionEnd;
			emit selectionChanged();
			InvalidateRect(&m_sDrawSettings, QRect(), false, true);
			update(m_sDrawSettings.rUpdate);
			m_mutexSettings.unlock();
			break;
		}
		case SelectionModeRecords:
		{
			Address sAddress;
			std::set<unsigned int>::iterator iterRecord;
			m_mutexSettings.lock();
			m_ptSelectionBegin = m_ptSelectionEnd = MapScreenToLongLat(&m_sDrawSettings, e->pos());
			if (g_pMapDB->FindCoordinates(&sAddress, m_ptSelectionEnd) && sAddress.iRecord != (unsigned)-1)
			{
				if ((iterRecord = m_setRecords.find(sAddress.iRecord)) == m_setRecords.end())
				{
					m_bAddRemoveRecords = true;
					m_setRecords.insert(sAddress.iRecord);
				}
				else
				{
					m_bAddRemoveRecords = false;
					m_setRecords.erase(iterRecord);
				}
				emit selectionChanged();
				InvalidateRect(&m_sDrawSettings, QRect(), false, true);
				update(m_sDrawSettings.rUpdate);
			} else
				m_bAddRemoveRecords = true;
			m_mutexSettings.unlock();
			break;
		}
		default:
			break;
		}
	}
}

void QMapWidget::mouseMoveEvent(QMouseEvent * e)
{
	if ((e->state() & Qt::LeftButton) == Qt::LeftButton)
	{
		switch (m_eSelectionMode)
		{
		case SelectionModeRect:
		case SelectionModePoint:
		{
			m_mutexSettings.lock();
			m_ptSelectionEnd = MapScreenToLongLat(&m_sDrawSettings, e->pos());
			emit selectionChanged();
			InvalidateRect(&m_sDrawSettings, QRect(), false, true);
			update(m_sDrawSettings.rUpdate);
			m_mutexSettings.unlock();
			break;
		}
		case SelectionModePoints:
		{
			m_mutexSettings.lock();
			m_ptSelectionEnd = MapScreenToLongLat(&m_sDrawSettings, e->pos());
			*m_iterPoint = m_ptSelectionEnd;
			emit selectionChanged();
			InvalidateRect(&m_sDrawSettings, QRect(), false, true);
			update(m_sDrawSettings.rUpdate);
			m_mutexSettings.unlock();
			break;
		}
		case SelectionModeRecords:
		{
			Address sAddress;
			std::set<unsigned int>::iterator iterRecord;
			m_mutexSettings.lock();
			m_ptSelectionEnd = MapScreenToLongLat(&m_sDrawSettings, e->pos());
			if (g_pMapDB->FindCoordinates(&sAddress, m_ptSelectionEnd) && sAddress.iRecord != (unsigned)-1)
			{
				if ((iterRecord = m_setRecords.find(sAddress.iRecord)) == m_setRecords.end() && m_bAddRemoveRecords)
				{
					m_setRecords.insert(sAddress.iRecord);
					emit selectionChanged();
					InvalidateRect(&m_sDrawSettings, QRect(), false, true);
					update(m_sDrawSettings.rUpdate);
				}
				else if (iterRecord != m_setRecords.end() && !m_bAddRemoveRecords)
				{
					m_setRecords.erase(iterRecord);
					emit selectionChanged();
					InvalidateRect(&m_sDrawSettings, QRect(), false, true);
					update(m_sDrawSettings.rUpdate);
				}
			}
			m_mutexSettings.unlock();
			break;
		}
		default:
			break;
		}
	}
}

void QMapWidget::mouseReleaseEvent(QMouseEvent * e)
{
	if (e->button() == Qt::RightButton)
	{
		int iResult = m_pRightClickMenu->exec(e->globalPos());
		Address sAddress;
		QString strText;
		Coords ptCenter;
		bool bOK = true;
		switch (iResult)
		{
		case 0:
			m_mutexSettings.lock();
			m_ptSelectionEnd = m_ptSelectionBegin = Coords();
			m_setRecords.clear();
			m_listPoints.clear();
			m_iterPoint = m_listPoints.end();
			emit selectionChanged();
			InvalidateRect(&m_sDrawSettings, QRect(), false, true);
			update(m_sDrawSettings.rUpdate);
			m_mutexSettings.unlock();
			break;
		case 1:
			m_mutexSettings.lock();
			ptCenter = MapScreenToLongLat(&m_sDrawSettings, e->pos());
			m_mutexSettings.unlock();
			recenter(ptCenter);
			break;
		case 2:
			strText = g_pSettings->m_listAddressHistory.empty() ? QString::null : g_pSettings->m_listAddressHistory.front();
			strText = QInputDialog::getText("GrooveNet", "Enter a street address or intersection to locate:", QLineEdit::Normal, strText, &bOK, this);
			if (bOK && StringToAddress(strText, &sAddress))
			{
				g_pSettings->AddAddress(strText);
				recenter(sAddress.ptCoordinates);
			}
			break;
		default:
			break;
		}
	}
	else if (e->button() == Qt::LeftButton)
	{
		switch (m_eSelectionMode)
		{
		case SelectionModeRect:
		case SelectionModePoint:
		{
			m_mutexSettings.lock();
			m_ptSelectionEnd = MapScreenToLongLat(&m_sDrawSettings, e->pos());
			emit selectionChanged();
			InvalidateRect(&m_sDrawSettings, QRect(), false, true);
			update(m_sDrawSettings.rUpdate);
			m_mutexSettings.unlock();
			break;
		}
		case SelectionModePoints:
		{
			m_mutexSettings.lock();
			m_ptSelectionEnd = MapScreenToLongLat(&m_sDrawSettings, e->pos());
			*m_iterPoint = m_ptSelectionEnd;
			m_iterPoint = m_listPoints.end();
			emit selectionChanged();
			InvalidateRect(&m_sDrawSettings, QRect(), false, true);
			update(m_sDrawSettings.rUpdate);
			m_mutexSettings.unlock();
			break;
		}
		case SelectionModeRecords:
		{
			Address sAddress;
			std::set<unsigned int>::iterator iterRecord;
			m_mutexSettings.lock();
			m_ptSelectionEnd = MapScreenToLongLat(&m_sDrawSettings, e->pos());
			if (g_pMapDB->FindCoordinates(&sAddress, m_ptSelectionEnd) && sAddress.iRecord != (unsigned)-1)
			{
				if ((iterRecord = m_setRecords.find(sAddress.iRecord)) == m_setRecords.end() && m_bAddRemoveRecords)
				{
					m_setRecords.insert(sAddress.iRecord);
					emit selectionChanged();
					InvalidateRect(&m_sDrawSettings, QRect(), false, true);
					update(m_sDrawSettings.rUpdate);
				}
				else if (iterRecord != m_setRecords.end() && !m_bAddRemoveRecords)
				{
					m_setRecords.erase(iterRecord);
					emit selectionChanged();
					InvalidateRect(&m_sDrawSettings, QRect(), false, true);
					update(m_sDrawSettings.rUpdate);
				}
			}
			m_mutexSettings.unlock();
			break;
		}
		default:
			break;
		}
	}
}

void QMapWidget::mouseDoubleClickEvent(QMouseEvent * e)
{
	if (e->button() == Qt::LeftButton)
	{
		switch (m_eSelectionMode)
		{
		case SelectionModePoints:
		{
			m_mutexSettings.lock();
			m_ptSelectionBegin = m_ptSelectionEnd = MapScreenToLongLat(&m_sDrawSettings, e->pos());
			Coords ptTemp = MapScreenToLongLat(&m_sDrawSettings, e->pos() - QPoint(5, 0));
			double fMaxDistance = Distance(ptTemp, m_ptSelectionEnd), fDistance;
			std::list<Coords>::iterator iterPtChosen;
			m_iterPoint = m_listPoints.end();
			for (iterPtChosen = m_listPoints.begin(); iterPtChosen != m_listPoints.end(); ++iterPtChosen)
			{
				if ((fDistance = Distance(*iterPtChosen, m_ptSelectionEnd)) < fMaxDistance)
				{
					fMaxDistance = fDistance;
					m_iterPoint = iterPtChosen;
				}
			}
			if (m_iterPoint != m_listPoints.end())
			{
				m_listPoints.erase(m_iterPoint);
				emit selectionChanged();
				InvalidateRect(&m_sDrawSettings, QRect(), false, true);
				update(m_sDrawSettings.rUpdate);
			}
			m_mutexSettings.unlock();
			break;
		}
		default:
			break;
		}
	}
}


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

#include "MapVisual.h"
#include "Simulator.h"
#include "StringHelp.h"
#include "MainWindow.h"
#include "CarModel.h"

#include <qstatusbar.h>
#include <qdragobject.h>
#include <qapplication.h>

#define MAPVISUAL_PARAM_COORDS "START_POS"
#define MAPVISUAL_PARAM_COORDS_DEFAULT "0,0"
#define MAPVISUAL_PARAM_COORDS_DESC "START_POS (TIGER coordinates) -- The longitude and latitute to initially center the map at (superseded by the FOLLOW parameter)."
#define MAPVISUAL_PARAM_DETAIL "ZOOM"
#define MAPVISUAL_PARAM_DETAIL_DEFAULT "4"
#define MAPVISUAL_PARAM_DETAIL_DESC "ZOOM (integer) -- The scale level of the map (higher is more zoomed out)."
#define MAPVISUAL_PARAM_CURRENTOBJ "FOLLOW"
#define MAPVISUAL_PARAM_CURRENTOBJ_DEFAULT ""
#define MAPVISUAL_PARAM_CURRENTOBJ_DESC "FOLLOW (model) -- A vehicle or infrastructure node to follow."

MapVisual::MapVisual(const QString & strModelName)
: Visualizer(strModelName), m_iInitialZoom(4)
{
	m_PaintSettings.tLastChange = timeval0;
	m_PaintSettings.pOldMapBmp = NULL;
	m_PaintSettings.pOldEverythingBmp = NULL;
	m_PaintSettings.pMemoryDC = NULL;
}

MapVisual::MapVisual(const MapVisual & copy)
: Visualizer(copy), m_DrawSettingsTemp(copy.m_DrawSettingsTemp), m_DrawSettings(copy.m_DrawSettings), m_PaintSettings(copy.m_PaintSettings), m_iInitialZoom(copy.m_iInitialZoom), m_ptInitialPosition(copy.m_ptInitialPosition)
{
}

MapVisual::~MapVisual()
{
}

MapVisual & MapVisual::operator = (const MapVisual & copy)
{
	Visualizer::operator = (copy);

	m_DrawSettingsTemp = copy.m_DrawSettingsTemp;
	m_DrawSettings = copy.m_DrawSettings;
	m_PaintSettings = copy.m_PaintSettings;
	m_iInitialZoom = copy.m_iInitialZoom;
	m_ptInitialPosition = copy.m_ptInitialPosition;
	return *this;
}

int MapVisual::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	std::map<int, MapObject *> * pMapObjects;
	std::map<int, MapObject *>::iterator iterMapObject;

	InitMapDrawingSettings(&m_DrawSettingsTemp);
	if (m_PaintSettings.pMemoryDC == NULL)
		m_PaintSettings.pMemoryDC = new QPainter();
	strValue = GetParam(mapParams, MAPVISUAL_PARAM_COORDS, MAPVISUAL_PARAM_COORDS_DEFAULT);
	m_ptInitialPosition.FromString(strValue);
	m_DrawSettingsTemp.ptCenter = m_ptInitialPosition;
	strValue = GetParam(mapParams, MAPVISUAL_PARAM_DETAIL, MAPVISUAL_PARAM_DETAIL_DEFAULT);
	m_DrawSettingsTemp.iDetailLevel = m_iInitialZoom = (int)ValidateNumber(StringToNumber(strValue), MIN_DETAIL_LEVEL, MAX_DETAIL_LEVEL);
	strValue = GetParam(mapParams, MAPVISUAL_PARAM_CURRENTOBJ, MAPVISUAL_PARAM_CURRENTOBJ_DEFAULT);
	pMapObjects = g_pMapObjects->acquireLock();
	for (iterMapObject = pMapObjects->begin(); iterMapObject != pMapObjects->end(); ++iterMapObject) {
		if (iterMapObject->second->GetName().compare(strValue) == 0)
		{
			m_DrawSettingsTemp.iCurrentObject = iterMapObject->first;
			break;
		}
	}
	g_pMapObjects->releaseLock();

	if (Visualizer::Init(mapParams))
		return 1;

	if (m_pWidget != NULL)
		m_pWidget->setAcceptDrops(true);

	return 0;
}

int MapVisual::ProcessEvent(SimEvent & event)
{
	QRect rUpdate, rCurrent;
	int ret = 0;

	if (Visualizer::ProcessEvent(event))
		return 1;

	switch (event.GetEventID())
	{
	case EVENT_VISUALIZER_UPDATE:
		if (m_pWidget == NULL)
			ret = 2;
		else if (m_UpdateMutex.tryLock())
		{
			MapObject * pObject;
	
			g_pMapObjects->acquireLock();
			pObject = g_pMapObjects->get(m_DrawSettingsTemp.iCurrentObject);
			rUpdate.setRect(0, 0, m_DrawSettingsTemp.iControlWidth, m_DrawSettingsTemp.iControlHeight);
			if (pObject == NULL || rUpdate.contains(rCurrent = pObject->RectVisible(&m_DrawSettingsTemp), true)) {
				InvalidateRect(&m_DrawSettingsTemp, QRect(), false, true);
				update();
				g_pMapObjects->releaseLock();
				m_UpdateMutex.unlock();
			}
			else
			{
				Coords ptNewCenter = MapScreenToLongLat(&m_DrawSettingsTemp, rCurrent.center());
				g_pMapObjects->releaseLock();
				m_UpdateMutex.unlock();
				recenter(ptNewCenter);
			}
		}
		else
			ret = 3;
	default:
		break;
	}
	return ret;
}

int MapVisual::PostRun()
{
	if (Model::PostRun())
		return 1;

	InvalidateRect(&m_DrawSettingsTemp, QRect(), false, true);
	update();

	return 0;
}

int MapVisual::Save(std::map<QString, QString> & mapParams)
{
	QString strValue;
	std::map<int, MapObject *> * pMapObjects;
	std::map<int, MapObject *>::iterator iterMapObject;

	mapParams[MAPVISUAL_PARAM_COORDS] = m_ptInitialPosition.ToString();
	mapParams[MAPVISUAL_PARAM_DETAIL] = QString("%1").arg(m_DrawSettingsTemp.iDetailLevel);

	pMapObjects = g_pMapObjects->acquireLock();
	iterMapObject = pMapObjects->find(m_DrawSettingsTemp.iCurrentObject);
	if (iterMapObject != pMapObjects->end())
		mapParams[MAPVISUAL_PARAM_CURRENTOBJ] = iterMapObject->second->GetName();
	else
		mapParams[MAPVISUAL_PARAM_CURRENTOBJ] = "";
	g_pMapObjects->releaseLock();

	if (Visualizer::Save(mapParams))
		return 1;

	return 0;
}

void MapVisual::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	Visualizer::GetParams(mapParams);

	mapParams[MAPVISUAL_PARAM_COORDS].strValue = MAPVISUAL_PARAM_COORDS_DEFAULT;
	mapParams[MAPVISUAL_PARAM_COORDS].strDesc = MAPVISUAL_PARAM_COORDS_DESC;
	mapParams[MAPVISUAL_PARAM_COORDS].eType = ModelParameterTypeCoords;

	mapParams[MAPVISUAL_PARAM_DETAIL].strValue = MAPVISUAL_PARAM_DETAIL_DEFAULT;
	mapParams[MAPVISUAL_PARAM_DETAIL].strDesc = MAPVISUAL_PARAM_DETAIL_DESC;
	mapParams[MAPVISUAL_PARAM_DETAIL].eType = ModelParameterTypeInt;
	mapParams[MAPVISUAL_PARAM_DETAIL].strAuxData = QString("%1:%2").arg(MIN_DETAIL_LEVEL).arg(MAX_DETAIL_LEVEL);

	mapParams[MAPVISUAL_PARAM_CURRENTOBJ].strValue = MAPVISUAL_PARAM_CURRENTOBJ_DEFAULT;
	mapParams[MAPVISUAL_PARAM_CURRENTOBJ].strDesc = MAPVISUAL_PARAM_CURRENTOBJ_DESC;
	mapParams[MAPVISUAL_PARAM_CURRENTOBJ].eType = (ModelParameterType)(ModelParameterTypeModel | ModelParameterFixed);
	mapParams[MAPVISUAL_PARAM_CURRENTOBJ].strAuxData = CARMODEL_NAME;
}

int MapVisual::Cleanup()
{
	if (Visualizer::Cleanup())
		return 1;

	if (m_PaintSettings.pMemoryDC != NULL)
	{
		if (m_PaintSettings.pMemoryDC->isActive())
			m_PaintSettings.pMemoryDC->end();
		delete m_PaintSettings.pMemoryDC;
		m_PaintSettings.pMemoryDC = NULL;
	}

	if (m_PaintSettings.pOldEverythingBmp != NULL)
	{
		delete m_PaintSettings.pOldEverythingBmp;
		m_PaintSettings.pOldEverythingBmp = NULL;
	}

	if (m_PaintSettings.pOldMapBmp != NULL)
	{
		delete m_PaintSettings.pOldMapBmp;
		m_PaintSettings.pOldMapBmp = NULL;
	}

	return 0;
}

void MapVisual::SettingsChanged()
{
	Visualizer::SettingsChanged();

	m_UpdateMutex.lock();
	UpdateMapDrawingSettings(&m_DrawSettingsTemp);
	InvalidateRect(&m_DrawSettingsTemp);
	update();
	m_UpdateMutex.unlock();
}

void MapVisual::paintEvent(QPaintEvent * e)
{
	QPainter dc(m_pWidget);
	QRect bounds;

	m_DrawMutex.lock();
	CopyMapDrawingSettings(&m_PaintSettings, &m_DrawSettings);
	m_DrawMutex.unlock();

	bounds = e->rect().unite(m_PaintSettings.rUpdate);
	dc.setClipRect(bounds);

	if (m_PaintSettings.pMemoryDC == NULL)
		return; // memory DC not active

	m_PaintSettings.ptTopLeftClip = MapScreenToLongLat(&m_PaintSettings, bounds.topLeft());
	m_PaintSettings.ptBottomRightClip = MapScreenToLongLat(&m_PaintSettings, bounds.bottomRight());

	if (g_pMapDB->GetLastChange() > m_PaintSettings.tLastChange)
	{
		// if the data has changed, then we must redraw
		m_PaintSettings.bL1Redraw = true;
		m_PaintSettings.tLastChange = g_pMapDB->GetLastChange();
	}

	if ((m_PaintSettings.pOldEverythingBmp == NULL || m_PaintSettings.pOldMapBmp == NULL || m_PaintSettings.pOldEverythingBmp->width() != m_PaintSettings.iControlWidth || m_PaintSettings.pOldEverythingBmp->height() != m_PaintSettings.iControlHeight) && m_PaintSettings.iControlWidth > 0 && m_PaintSettings.iControlHeight > 0)
	{
		if (m_PaintSettings.pMemoryDC->isActive())
			m_PaintSettings.pMemoryDC->end();
		ReplaceOldEverythingBmp(&m_PaintSettings, new QPixmap(m_PaintSettings.iControlWidth, m_PaintSettings.iControlHeight));
		ReplaceOldMapBmp(&m_PaintSettings, new QPixmap(m_PaintSettings.iControlWidth, m_PaintSettings.iControlHeight));
		m_PaintSettings.pMemoryDC->begin(m_PaintSettings.pOldEverythingBmp, m_pWidget);
		m_PaintSettings.bL1Redraw = true;
	}
	if (m_PaintSettings.pMemoryDC->isActive())
	{
		if (m_PaintSettings.bL1Redraw) {
			g_pMapDB->DrawMap(&m_PaintSettings, bounds);
			QPainter temp;
			temp.begin(m_PaintSettings.pOldMapBmp, m_pWidget);
			temp.fillRect(bounds, m_PaintSettings.clrBackground);
			temp.drawPixmap(bounds.topLeft(), *m_PaintSettings.pOldEverythingBmp, bounds);
			temp.end();
			m_PaintSettings.bL2Redraw = true;
		}
		if (m_PaintSettings.bL2Redraw) {
			m_PaintSettings.pMemoryDC->drawPixmap(bounds.topLeft(), *m_PaintSettings.pOldMapBmp, bounds);
			// TODO: draw tracks here! (call MapDB::DrawRecordHighlights())
			if (m_PaintSettings.bShowMarkers)
				g_pMapObjects->RedrawObjects(&m_PaintSettings);
		}
		g_pMapDB->DrawBorder(&m_PaintSettings, m_pWidget->hasFocus());
		m_PaintSettings.bL1Redraw = false;
		m_PaintSettings.bL2Redraw = false;
		m_PaintSettings.rUpdate = QRect();
		dc.drawPixmap(bounds.topLeft(), *m_PaintSettings.pOldEverythingBmp, bounds);
	}
	m_UpdateMutex.lock();
	m_DrawSettingsTemp.bL1Redraw = m_PaintSettings.bL1Redraw;
	m_DrawSettingsTemp.bL2Redraw = m_PaintSettings.bL2Redraw;
	m_DrawSettingsTemp.rUpdate = m_PaintSettings.rUpdate;
	m_DrawSettingsTemp.tLastChange = m_PaintSettings.tLastChange;
	m_UpdateMutex.unlock();
	m_DrawMutex.lock();
	m_DrawSettings.bL1Redraw = m_PaintSettings.bL1Redraw;
	m_DrawSettings.bL2Redraw = m_PaintSettings.bL2Redraw;
	m_DrawSettings.rUpdate = m_PaintSettings.rUpdate;
	m_DrawSettings.tLastChange = m_PaintSettings.tLastChange;
	m_DrawMutex.unlock();
}

void MapVisual::resizeEvent(QResizeEvent * e)
{
	m_UpdateMutex.lock();
	m_DrawSettingsTemp.iControlWidth = e->size().width();
	m_DrawSettingsTemp.iControlHeight = e->size().height();
	m_DrawSettingsTemp.ptTopLeft = MapScreenToLongLat(&m_DrawSettingsTemp, QPoint(0, 0));
	m_DrawSettingsTemp.ptBottomRight = MapScreenToLongLat(&m_DrawSettingsTemp, QPoint(m_DrawSettingsTemp.iControlWidth, m_DrawSettingsTemp.iControlHeight));
	InvalidateRect(&m_DrawSettingsTemp);
	update();
	m_UpdateMutex.unlock();
}

void MapVisual::moveCenter(float fScreensRight, float fScreensDown)
{
	QPoint ptNew;

	m_UpdateMutex.lock();
	ptNew.setX((int)((fScreensRight + 0.5f) * m_DrawSettingsTemp.iControlWidth));
	ptNew.setY((int)((fScreensDown + 0.5f) * m_DrawSettingsTemp.iControlHeight));
	m_DrawSettingsTemp.ptCenter = MapScreenToLongLat(&m_DrawSettingsTemp, ptNew);
	m_DrawSettingsTemp.ptTopLeft = MapScreenToLongLat(&m_DrawSettingsTemp, QPoint(0, 0));
	m_DrawSettingsTemp.ptBottomRight = MapScreenToLongLat(&m_DrawSettingsTemp, QPoint(m_DrawSettingsTemp.iControlWidth, m_DrawSettingsTemp.iControlHeight));
	InvalidateRect(&m_DrawSettingsTemp);
	update();
	m_UpdateMutex.unlock();
}

void MapVisual::recenter(const Coords & newCenter)
{
	m_UpdateMutex.lock();
	m_DrawSettingsTemp.ptCenter = newCenter;
	m_DrawSettingsTemp.ptTopLeft = MapScreenToLongLat(&m_DrawSettingsTemp, QPoint(0, 0));
	m_DrawSettingsTemp.ptBottomRight = MapScreenToLongLat(&m_DrawSettingsTemp, QPoint(m_DrawSettingsTemp.iControlWidth, m_DrawSettingsTemp.iControlHeight));
	InvalidateRect(&m_DrawSettingsTemp);
	update();
	m_UpdateMutex.unlock();
}

void MapVisual::zoom(int iZoomOut)
{
	m_UpdateMutex.lock();
	m_DrawSettingsTemp.iDetailLevel += iZoomOut;
	if (m_DrawSettingsTemp.iDetailLevel < MIN_DETAIL_LEVEL)
		m_DrawSettingsTemp.iDetailLevel = MIN_DETAIL_LEVEL;
	else if (m_DrawSettingsTemp.iDetailLevel > MAX_DETAIL_LEVEL)
		m_DrawSettingsTemp.iDetailLevel = MAX_DETAIL_LEVEL;
	m_DrawSettingsTemp.ptTopLeft = MapScreenToLongLat(&m_DrawSettingsTemp, QPoint(0, 0));
	m_DrawSettingsTemp.ptBottomRight = MapScreenToLongLat(&m_DrawSettingsTemp, QPoint(m_DrawSettingsTemp.iControlWidth, m_DrawSettingsTemp.iControlHeight));
	InvalidateRect(&m_DrawSettingsTemp);
	update();
	m_UpdateMutex.unlock();
}

void MapVisual::rezoom(int iZoom)
{
	m_UpdateMutex.lock();
	m_DrawSettingsTemp.iDetailLevel = iZoom;
	if (m_DrawSettingsTemp.iDetailLevel < MIN_DETAIL_LEVEL)
		m_DrawSettingsTemp.iDetailLevel = MIN_DETAIL_LEVEL;
	else if (m_DrawSettingsTemp.iDetailLevel > MAX_DETAIL_LEVEL)
		m_DrawSettingsTemp.iDetailLevel = MAX_DETAIL_LEVEL;
	m_DrawSettingsTemp.ptTopLeft = MapScreenToLongLat(&m_DrawSettingsTemp, QPoint(0, 0));
	m_DrawSettingsTemp.ptBottomRight = MapScreenToLongLat(&m_DrawSettingsTemp, QPoint(m_DrawSettingsTemp.iControlWidth, m_DrawSettingsTemp.iControlHeight));
	InvalidateRect(&m_DrawSettingsTemp);
	update();
	m_UpdateMutex.unlock();
}

bool MapVisual::processKey(int iQtKey, Qt::ButtonState eButtonState)
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
		rezoom(m_iInitialZoom);
		return true;
	default:
		return false;
	}
}

void MapVisual::keyPressEvent(QKeyEvent * e)
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

void MapVisual::mouseReleaseEvent(QMouseEvent * e)
{
	Coords ptNewCenter;
	std::map<int, MapObject *> * pMapObjects;
	std::map<int, MapObject *>::iterator iterMapObject, iterClosest;
	int iClosest = 0;
	QRect rRect;

	m_mutexUpdate.lock();
	pMapObjects = g_pMapObjects->acquireLock();
	iterClosest = pMapObjects->end();
	ptNewCenter = MapScreenToLongLat(&m_DrawSettingsTemp, e->pos());

	for (iterMapObject = pMapObjects->begin(); iterMapObject != pMapObjects->end(); ++iterMapObject)
	{
		rRect = iterMapObject->second->RectVisible(&m_DrawSettingsTemp);
		if (rRect.contains(e->pos()))
		{
			int iTemp = (e->pos() - rRect.center()).manhattanLength();
			if (iterClosest == pMapObjects->end() || iTemp < iClosest) {
				iClosest = iTemp;
				iterClosest = iterMapObject;
			}
		}
	}
	if (iterClosest != pMapObjects->end()) {
		m_DrawSettingsTemp.iCurrentObject = iterClosest->first;
		if (g_pMainWindow != NULL)
			g_pMainWindow->statusBar()->message("Selected: " + iterClosest->second->GetDescription(), 2000);
	} else {
		m_DrawSettingsTemp.iCurrentObject = -1;
		if (g_pMainWindow != NULL)
			g_pMainWindow->statusBar()->message("Center: (" + DegreesToString(ptNewCenter.m_iLong, 6) + ", " + DegreesToString(ptNewCenter.m_iLat, 6) + ")", 2000);
	}
	g_pMapObjects->releaseLock();
	m_mutexUpdate.unlock();
	recenter(ptNewCenter);
	e->accept();
}

void MapVisual::dragEnterEvent(QDragEnterEvent * e)
{
	e->accept(m_pWidget != NULL && QTextDrag::canDecode(e));
}

void MapVisual::dragMoveEvent(QDragMoveEvent * e)
{
	if (m_pWidget != NULL)
		e->accept(m_pWidget->rect());
	else
		e->ignore();
}

void MapVisual::dropEvent(QDropEvent * e)
{
	QString strCommand;
	if (QTextDrag::decode(e, strCommand))
	{
		if (strCommand.left(7).compare("OBJECT:") == 0)
		{
			std::map<int, MapObject *> * pMapObjects;
			std::map<int, MapObject *>::iterator iterMapObject;
			QString strObjName = strCommand.mid(7).stripWhiteSpace();
			QRect rObject;

			m_UpdateMutex.lock();
			pMapObjects = g_pMapObjects->acquireLock();
			m_DrawSettingsTemp.iCurrentObject = -1;
			for (iterMapObject = pMapObjects->begin(); iterMapObject != pMapObjects->end(); ++iterMapObject) {
				if (iterMapObject->second->GetName().compare(strObjName) == 0)
				{
					m_DrawSettingsTemp.iCurrentObject = iterMapObject->first;
					rObject = iterMapObject->second->RectVisible(&m_DrawSettingsTemp);
					break;
				}
			}

			if (m_DrawSettingsTemp.iCurrentObject != -1)
			{
				Coords ptNewCenter = MapScreenToLongLat(&m_DrawSettingsTemp, rObject.center());
				g_pMapObjects->releaseLock();
				m_UpdateMutex.unlock();
				recenter(ptNewCenter);
			}
			else
			{
				g_pMapObjects->releaseLock();
				InvalidateRect(&m_DrawSettingsTemp, QRect(), false, true);
				update();
				m_UpdateMutex.unlock();
			}

			e->accept();
		}
		else
			e->ignore();
	} else
		e->ignore();
}

MapDrawingSettings * MapVisual::getSettings(bool bWait)
{
	if (bWait)
	{
		m_UpdateMutex.lock();
		return &m_DrawSettingsTemp;
	}
	else
		return m_UpdateMutex.tryLock() ? &m_DrawSettingsTemp : NULL;
}

void MapVisual::releaseSettings()
{
	m_UpdateMutex.unlock();
}

void MapVisual::update()
{
	m_DrawMutex.lock();
	CopyMapDrawingSettings(&m_DrawSettings, &m_DrawSettingsTemp);
	m_DrawMutex.unlock();

	if (m_pWidget && m_DrawSettings.rUpdate.isValid())
		m_pWidget->update(m_DrawSettings.rUpdate);
}

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

#ifndef _MAPVISUAL_H
#define _MAPVISUAL_H

#include "Visualizer.h"
#include "MapDB.h"
#include "MapObjects.h"

#define MAPVISUAL_NAME "MapVisual"

class MapVisual : public Visualizer
{
public:
	inline virtual QString GetModelType() const
	{
		return MAPVISUAL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(MAPVISUAL_NAME) == 0 || (bDescendSufficient && Visualizer::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	MapVisual(const QString & strModelName = QString::null);
	MapVisual(const MapVisual & copy);
	virtual ~MapVisual();

	virtual MapVisual & operator = (const MapVisual & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int ProcessEvent(SimEvent & event);
	virtual int PostRun();
	virtual int Save(std::map<QString, QString> & mapParams);
	virtual int Cleanup();

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual void SettingsChanged();

	inline virtual void focusInEvent(QFocusEvent * e __attribute__ ((unused)) ) { if (m_pWidget != NULL) m_pWidget->update(); }
	inline virtual void focusOutEvent(QFocusEvent * e __attribute__ ((unused)) ) { if (m_pWidget != NULL) m_pWidget->update(); }
	virtual void paintEvent(QPaintEvent * e);
	virtual void resizeEvent(QResizeEvent * e);
	virtual void keyPressEvent(QKeyEvent * e);
	virtual void mouseReleaseEvent(QMouseEvent * e);
	virtual void dragEnterEvent(QDragEnterEvent * e);
	virtual void dragMoveEvent(QDragMoveEvent * e);
	virtual void dropEvent(QDropEvent * e);

	MapDrawingSettings * getSettings(bool bWait = true);
	void releaseSettings();
	void update();

	void moveCenter(float fScreensRight, float fScreensDown);
	void recenter(const Coords & newCenter);
	void zoom(int iZoomOut);
	void rezoom(int iZoom);
	bool processKey(int iQtKey, Qt::ButtonState eButtonState = Qt::NoButton);

protected:
	MapDrawingSettings m_DrawSettingsTemp;
	QMutex m_UpdateMutex;

	MapDrawingSettings m_DrawSettings;
	QMutex m_DrawMutex;

	MapDrawingSettings m_PaintSettings;

	int m_iInitialZoom;
	Coords m_ptInitialPosition;
};

inline Model * MapVisualCreator(const QString & strModelName)
{
	return new MapVisual(strModelName);
}

#endif

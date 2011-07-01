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

#ifndef _VISUALIZER_H
#define _VISUALIZER_H

#include "Model.h"

#include <qwidget.h>
#include <qevent.h>

#define VISUALIZER_NAME "Visualizer"

#define EVENT_VISUALIZER_UPDATE 840184

#define EVENT_PRIORITY_UPDATE 1000

class Visualizer : public Model
{
public:
	inline virtual QString GetModelType() const
	{
		return VISUALIZER_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(VISUALIZER_NAME) == 0 || (bDescendSufficient && Model::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	Visualizer(const QString & strModelName = QString::null);
	Visualizer(const Visualizer & copy);
	virtual ~Visualizer();

	virtual Visualizer & operator = (const Visualizer & copy);

	inline virtual const QWidget * GetWidget() const
	{
		return m_pWidget;
	}
	inline virtual QWidget * GetWidget()
	{
		return m_pWidget;
	}
	inline virtual void SetWidget(QWidget * pWidget)
	{
		m_pWidget = pWidget;
	}
	inline virtual struct timeval GetDelay() const
	{
		return m_tDelay;
	}
	inline virtual void SetDelay(const struct timeval & tDelay)
	{
		m_tDelay = tDelay;
	}

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int ProcessEvent(SimEvent & event);
	virtual int PostRun();
	virtual int Save(std::map<QString, QString> & mapParams);
	virtual int Cleanup();

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	inline virtual void closeEvent(QCloseEvent * e __attribute__ ((unused)) ) {m_pWidget = NULL;}
	inline virtual void contextMenuEvent(QContextMenuEvent * e) {e->ignore();}
	inline virtual void dragEnterEvent(QDragEnterEvent * e) {e->ignore();}
	inline virtual void dragLeaveEvent(QDragLeaveEvent * e) {e = e;}
	inline virtual void dragMoveEvent(QDragMoveEvent * e) {e->ignore();}
	inline virtual void dropEvent(QDropEvent * e) {e->ignore();}
	inline virtual void enterEvent(QEvent * e __attribute__ ((unused)) ) {}
	inline virtual void focusInEvent(QFocusEvent * e __attribute__ ((unused)) ) {}
	inline virtual void focusOutEvent(QFocusEvent * e __attribute__ ((unused)) ) {}
	inline virtual void hideEvent(QHideEvent * e __attribute__ ((unused)) ) {}
	inline virtual void keyPressEvent(QKeyEvent * e) {e->ignore();}
	inline virtual void keyReleaseEvent(QKeyEvent * e) {e->ignore();}
	inline virtual void leaveEvent(QEvent * e __attribute__ ((unused)) ) {}
	inline virtual void mouseDoubleClickEvent(QMouseEvent * e) {e->ignore();}
	inline virtual void mouseMoveEvent(QMouseEvent * e) {e->ignore();}
	inline virtual void mousePressEvent(QMouseEvent * e) {e->ignore();}
	inline virtual void mouseReleaseEvent(QMouseEvent * e) {e->ignore();}
	inline virtual void moveEvent(QMoveEvent * e __attribute__ ((unused)) ) {}
	inline virtual void paintEvent(QPaintEvent * e __attribute__ ((unused)) ) {}
	inline virtual void resizeEvent(QResizeEvent * e __attribute__ ((unused)) ) {}
	inline virtual void showEvent(QShowEvent * e __attribute__ ((unused)) ) {}
	inline virtual void wheelEvent(QWheelEvent * e) {e->ignore();}

protected:
	virtual QWidget * CreateWidget();

	QWidget * m_pWidget;
	struct timeval m_tDelay;
};

#define VISUALIZER_PARAM_CAPTION "CAPTION"
#define VISUALIZER_PARAM_CAPTION_DEFAULT "GrooveNet Widget"
#define VISUALIZER_PARAM_CAPTION_DESC "CAPTION (string) -- The text to be displayed captioning this window."
#define VISUALIZER_PARAM_WIDTH "WIDTH"
#define VISUALIZER_PARAM_WIDTH_DEFAULT "400"
#define VISUALIZER_PARAM_WIDTH_DESC "WIDTH (pixels) -- The width of the window."
#define VISUALIZER_PARAM_HEIGHT "HEIGHT"
#define VISUALIZER_PARAM_HEIGHT_DEFAULT "300"
#define VISUALIZER_PARAM_HEIGHT_DESC "HEIGHT (pixels) -- The height of the window."

#endif

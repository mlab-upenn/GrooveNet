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

#ifndef _QCOLORPUSHBUTTON_H
#define _QCOLORPUSHBUTTON_H

#include <qpushbutton.h>
#include <qlineedit.h>

class QColorPushButton : public QPushButton
{
Q_OBJECT
public:
	QColorPushButton(QLineEdit * pBuddy = NULL, const QString & strText = "...", QWidget * parent = NULL, const char * name = 0);
	virtual ~QColorPushButton();

	QString m_strColor;

signals:
	virtual void colorChanged();

protected slots:
	virtual void slotChangeColor();

protected:
	QLineEdit * m_pBuddy;
};

#endif

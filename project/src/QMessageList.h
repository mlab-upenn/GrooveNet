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

#ifndef _QMESSAGELIST_H
#define _QMESSAGELIST_H

#include <qwidget.h>
#include <qmutex.h>
#include <qlayout.h>
#include <q3listbox.h>
#include <qlineedit.h>
#include <q3textedit.h>
//Added by qt3to4:
#include <QLabel>
#include <Q3PopupMenu>
#include <q3boxlayout.h>
#include <vector>

class QLabel;
class QLineEdit;
class QPushButton;
class Q3PopupMenu;
class Q3ListBox;

class QMessageList : public QWidget
{
Q_OBJECT
public:
	QMessageList(QWidget * parent = NULL, const char * name = 0, Qt::WFlags f = 0);
	QMessageList();
	~QMessageList();

	void addMessage(QString msg);
	void clear();
protected:
	QWidget * m_boxMessages;
	Q3ListBox * m_listMessages;
	Q3BoxLayout * layout;
};

extern QMessageList * m_pMessageList;

#endif

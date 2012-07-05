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
#ifndef _QCONFIGUREDIALOG_H
#define _QCONFIGUREDIALOG_H

#include <qdialog.h>
#include "Settings.h"

class QTable;
class QPushButton;
class QExpandableTableItem;

class QConfigureDialog : public QDialog
{
Q_OBJECT
public:
	QConfigureDialog(QWidget *parent = 0, const char *name = 0);

protected:
	void Setup();
	void UpdateTable();

	Settings m_Settings;
	QTable * m_pTableProperties;
	QPushButton * m_pDefault, * m_pOK, * m_pCancel, * m_pApply;
	std::map<QString, QExpandableTableItem *> m_mapGroups;
	std::map<QString, QTableItem *> m_mapChildren;

protected slots:
	virtual void slotOK();
	virtual void slotApply();
	virtual void slotDefault();
	virtual void slotSettingsChanged(int row, int col);
	virtual void slotTableDoubleClicked(int row, int col, int button, const QPoint & mousePos);
	virtual void slotTableClicked(int row, int col, int button, const QPoint & mousePos);
};

#endif

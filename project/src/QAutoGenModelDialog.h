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

#ifndef _QAUTOGENMODELDIALOG_H
#define _QAUTOGENMODELDIALOG_H

#include "QAutoGenDialog.h"

class QLabel;
class QTextEdit;

class QAutoGenModelDialog : public QDialog
{
Q_OBJECT
public:
	QAutoGenModelDialog(const QString & strModelName, const QString & strModelType, QWidget * parent = NULL, const char * name = 0, WFlags f = 0);
	virtual ~QAutoGenModelDialog();

	virtual void InitializeModelParams();

	std::vector<QString> * m_pVecModelTypes;
	std::map<QString, ModelParameter> * m_pModelParams;
	std::map<QString, AutoGenParameter> * m_pParams;
	std::map<QString, QString> * m_pAssocModelTypeMap;
	std::vector<std::pair<QString, QString> > * m_pAssocModelTypes;
	QLabel * m_labelModel;
	QTextEdit * m_txtPropertiesHelp;

protected slots:
	virtual void slotPropertiesValueChanged(int row, int col);
	virtual void slotPropertiesCurrentChanged(int row, int col);
	virtual void accept();

protected:
	bool AddTableItem(QTable * pTable, int row, const QString & strParam, AutoGenParameter & param, bool bUpdateTypes = false, bool bDisable = false);

	QPushButton * m_buttonProceed, * m_buttonCancel;
	QTable * m_tableProperties;
	QString m_strModelName, m_strModelType;
};

#endif

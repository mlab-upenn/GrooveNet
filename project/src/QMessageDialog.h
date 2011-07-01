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

#ifndef _QMESSAGEDIALOG_H
#define _QMESSAGEDIALOG_H

#include <arpa/inet.h>
#include <qdialog.h>
#include <vector>

#include "Message.h"

class QRadioButton;
class QButtonGroup;
class QPushButton;
class QComboBox;
class QSpinBox;
class QListBox;
class QLabel;
class QLineEdit;

class QMessageDialog;

typedef void (* QMessageDialogAcceptCallback)(QMessageDialog *);

class QMessageDialog : public QDialog
{
Q_OBJECT
public:
	QMessageDialog(in_addr_t ipCar = 0, QWidget * parent = NULL, const char * name = 0, WFlags f = 0);
	virtual ~QMessageDialog();

	void LoadMessages();
	void UpdateBoundingRegion();

	std::vector<QString> m_vecEmergencyMessages, m_vecWarningMessages;
	std::vector<in_addr_t> m_vecMsgSources;

	in_addr_t m_ipCar;
	QMessageDialogAcceptCallback m_pfnAcceptCallback;
	SafetyPacket::BoundingRegion m_sBoundingRegion;

	QComboBox * m_comboMsgSource;
	QRadioButton * m_buttonEmergency, * m_buttonWarning;
	QButtonGroup * m_groupMsgType;
	QComboBox * m_comboMsgText, * m_comboMsgRegionType;
	QSpinBox * m_spinMsgLifetime;
	QLabel * m_labelMsgRegionType, * m_labelDest, * m_labelTXTime;
	QLineEdit * m_txtDest, * m_txtTXTime;
	QPushButton * m_buttonMsgRegionConf, * m_buttonOK, * m_buttonCancel;

protected slots:
	virtual void slotMsgTypeChanged(int id);
	virtual void slotBoundingRegionTypeChanged(int index);
	virtual void slotBoundingRegionConfig();
	virtual void accept();
};

void DispatchMessage(QMessageDialog * pDialog);

#endif

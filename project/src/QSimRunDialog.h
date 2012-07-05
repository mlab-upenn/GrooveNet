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

#ifndef _QSIMRUNDIALOG_H
#define _QSIMRUNDIALOG_H

#include <qdialog.h>

#include "Global.h"
#include "Simulator.h"

#include <vector>

class QPushButton;
class QCheckBox;
class QButtonGroup;
class QRadioButton;
class QLineEdit;
class QSpinBox;
class QListBox;
class QTable;

class QSimRunDialog : public QDialog
{
Q_OBJECT
public:
	QSimRunDialog(QWidget * parent = 0, const char * name = 0, bool modal = false, WFlags f = 0);
	virtual ~QSimRunDialog();

	void GetLogFilePaths(std::vector<QString> & vecFilenames);

	struct timeval m_tIncrement, m_tDuration;
	unsigned int m_iTrials;
	std::vector<EventMessage> m_vecMessages;
	bool m_bProfile;

protected slots:
	virtual void slotSimulationType(int id);
	virtual void slotTrialsChanged(int value);
	virtual void slotDurationToggled(bool on);
	virtual void slotDurationChanged(const QString & strText);
	virtual void slotSimulationTime(int id);
	virtual void slotIncrementChanged(const QString & strText);
	virtual void slotMessageHighlighted(int index);
	virtual void slotAddMessage();
	virtual void slotEditMessage();
	virtual void slotRemoveMessage();
	virtual void slotProfileToggled(bool on);

protected:
	QPushButton * m_buttonAddMsg, * m_buttonEditMsg, * m_buttonRemoveMsg, * m_buttonRun, * m_buttonCancel;
	QButtonGroup * m_groupSimType, * m_groupTime;
	QRadioButton * m_buttonRunOnce, * m_buttonMonteCarlo, * m_buttonRealTime, * m_buttonFastTime;
	QSpinBox * m_spinTrials;
	QCheckBox * m_chkDuration;
	QLineEdit * m_txtIncrement, * m_txtDuration;
	QCheckBox * m_chkProfile;
	QListBox * m_listMessages;
	QTable * m_tableLogFiles;
	int m_iRunOnceID, m_iRunMonteCarloID, m_iRealTimeID, m_iFastTimeID;
};

#endif

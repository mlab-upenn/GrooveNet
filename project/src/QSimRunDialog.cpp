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

#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qtable.h>
#include <qcombobox.h>

#include "QSimRunDialog.h"
#include "StringHelp.h"
#include "Logger.h"
#include "QFileTableItem.h"
#include "QMessageDialog.h"

#include <qlayout.h>
#include <qsplitter.h>

#include "app16x16.xpm"

QSimRunDialog::QSimRunDialog(QWidget * parent, const char * name, bool modal, WFlags f)
: QDialog(parent, name, modal, f), m_tIncrement(timeval0), m_tDuration(timeval0), m_bProfile(false)
{
	unsigned int i;

	QSplitter * pListTableSplitter = new QSplitter(Qt::Vertical, this);
	QWidget * pDurationBox = new QWidget(this);
	QWidget * pMessagesBox = new QWidget(pListTableSplitter);
	QWidget * pMessagesButtonsBox = new QWidget(pMessagesBox);
	QWidget * pProfileBox = new QWidget(this);
	QWidget * pButtonBox = new QWidget(this);

	QVBoxLayout * pLayout = new QVBoxLayout(this, 8, 8);
	QHBoxLayout * pDurationBoxLayout = new QHBoxLayout(pDurationBox, 0, 8);
	QHBoxLayout * pMessagesBoxLayout = new QHBoxLayout(pMessagesBox, 0, 8);
	QVBoxLayout * pMessagesButtonsBoxLayout = new QVBoxLayout(pMessagesButtonsBox, 0, 8);
	QHBoxLayout * pProfileBoxLayout = new QHBoxLayout(pProfileBox, 0, 8);
	QHBoxLayout * pButtonBoxLayout = new QHBoxLayout(pButtonBox, 0, 8);

	setCaption("GrooveNet - Run Simulation...");
	setIcon(app16x16_xpm);

	m_groupSimType = new QButtonGroup(3, Qt::Horizontal, "Simulation Type", this);
	m_buttonRunOnce = new QRadioButton("Run Once", m_groupSimType);
	m_buttonMonteCarlo = new QRadioButton("Monte Carlo", m_groupSimType);
	m_spinTrials = new QSpinBox(1, 999999999, 1, m_groupSimType);
	m_spinTrials->setSuffix(" trials");
	m_groupTime = new QButtonGroup(3, Qt::Horizontal, "Simulation Time", this);
	m_buttonRealTime = new QRadioButton("Real Time", m_groupTime);
	m_buttonFastTime = new QRadioButton("Fixed Time Increments:", m_groupTime);
	m_txtIncrement = new QLineEdit("0.", m_groupTime);
	m_chkDuration = new QCheckBox("Duration:", pDurationBox);
	m_txtDuration = new QLineEdit("0.", pDurationBox);
	m_listMessages = new QListBox(pMessagesBox);
	m_buttonAddMsg = new QPushButton("Add...", pMessagesButtonsBox);
	m_buttonEditMsg = new QPushButton("Edit...", pMessagesButtonsBox);
	m_buttonRemoveMsg = new QPushButton("Remove", pMessagesButtonsBox);
	m_tableLogFiles = new QTable(LOGFILES, 2, pListTableSplitter);
	m_chkProfile = new QCheckBox("Profile", pProfileBox);
	m_tableLogFiles->verticalHeader()->hide();
	m_tableLogFiles->setLeftMargin(0);
	m_tableLogFiles->horizontalHeader()->setClickEnabled(false);
	m_tableLogFiles->horizontalHeader()->setMovingEnabled(false);
	m_tableLogFiles->horizontalHeader()->setLabel(0, "Log File Type");
	m_tableLogFiles->horizontalHeader()->setLabel(1, "Path");
	m_tableLogFiles->setSorting(false);
	m_tableLogFiles->setSelectionMode(QTable::SingleRow);
	m_tableLogFiles->setFocusStyle(QTable::FollowStyle);
	m_tableLogFiles->setColumnReadOnly(0, true);
	m_tableLogFiles->setColumnReadOnly(1, false);
	m_tableLogFiles->setColumnStretchable(1, true);
	for (i = 0; i < LOGFILES; i++)
	{
		m_tableLogFiles->setText(i, 0, g_strLogFileNames[i]);
		m_tableLogFiles->setItem(i, 1, new QFileTableItem(m_tableLogFiles));
	}
	m_tableLogFiles->adjustColumn(0);
	m_buttonRun = new QPushButton("&Run", pButtonBox, "run");
	m_buttonCancel = new QPushButton("&Cancel", pButtonBox, "cancel");
	m_iRunOnceID = m_groupSimType->id(m_buttonRunOnce);
	m_iRunMonteCarloID = m_groupSimType->id(m_buttonMonteCarlo);
	m_iRealTimeID = m_groupTime->id(m_buttonRealTime);
	m_iFastTimeID = m_groupTime->id(m_buttonFastTime);
	connect(m_groupSimType, SIGNAL(clicked(int)), this, SLOT(slotSimulationType(int)));
	connect(m_spinTrials, SIGNAL(valueChanged(int)), this, SLOT(slotTrialsChanged(int)));
	connect(m_chkDuration, SIGNAL(toggled(bool)), this, SLOT(slotDurationToggled(bool)));
	connect(m_txtDuration, SIGNAL(textChanged(const QString& )), this, SLOT(slotDurationChanged(const QString& )));
	connect(m_groupTime, SIGNAL(clicked(int)), this, SLOT(slotSimulationTime(int)));
	connect(m_txtIncrement, SIGNAL(textChanged(const QString& )), this, SLOT(slotIncrementChanged(const QString& )));
	connect(m_listMessages, SIGNAL(highlighted(int)), this, SLOT(slotMessageHighlighted(int)));
	connect(m_buttonAddMsg, SIGNAL(clicked()), this, SLOT(slotAddMessage()));
	connect(m_buttonEditMsg, SIGNAL(clicked()), this, SLOT(slotEditMessage()));
	connect(m_buttonRemoveMsg, SIGNAL(clicked()), this, SLOT(slotRemoveMessage()));
	connect(m_chkProfile, SIGNAL(toggled(bool)), this, SLOT(slotProfileToggled(bool)));
	connect(m_buttonRun, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
	m_buttonCancel->setAutoDefault(false);
	m_groupSimType->setButton(m_iRunOnceID);
	m_chkDuration->setChecked(false);
	m_chkProfile->setChecked(false);
	m_groupTime->setButton(m_iRealTimeID);
	slotSimulationType(m_groupSimType->selectedId());
	slotDurationToggled(false);
	slotSimulationTime(m_groupTime->selectedId());
	slotProfileToggled(false);
	m_listMessages->setCurrentItem(-1);
	slotMessageHighlighted(m_listMessages->currentItem());

	pLayout->addWidget(m_groupSimType);
	pLayout->addWidget(pDurationBox);
	pLayout->addWidget(m_groupTime);
	pLayout->addWidget(pListTableSplitter, 1);
	pLayout->addWidget(pProfileBox);
	pLayout->addWidget(pButtonBox);
	pDurationBoxLayout->addWidget(m_chkDuration, 0, Qt::AlignLeft);
	pDurationBoxLayout->addWidget(m_txtDuration, 1);
	pMessagesBoxLayout->addWidget(m_listMessages, 1);
	pMessagesBoxLayout->addWidget(pMessagesButtonsBox);
	pMessagesButtonsBoxLayout->addWidget(m_buttonAddMsg, 0, Qt::AlignCenter);
	pMessagesButtonsBoxLayout->addWidget(m_buttonEditMsg, 0, Qt::AlignCenter);
	pMessagesButtonsBoxLayout->addWidget(m_buttonRemoveMsg, 0, Qt::AlignCenter);
	pMessagesButtonsBoxLayout->addStretch(1);
	pProfileBoxLayout->addWidget(m_chkProfile, 0, Qt::AlignLeft);
	pProfileBoxLayout->addStretch(1);
	pButtonBoxLayout->addStretch(1);
	pButtonBoxLayout->addWidget(m_buttonRun);
	pButtonBoxLayout->addWidget(m_buttonCancel);
}

QSimRunDialog::~QSimRunDialog()
{
}

void QSimRunDialog::GetLogFilePaths(std::vector<QString> & vecFilenames)
{
	unsigned int i;
	vecFilenames.resize(LOGFILES);
	for (i = 0; i < LOGFILES; i++)
		vecFilenames[i] = m_tableLogFiles->text(i, 1);
}

void QSimRunDialog::slotSimulationType(int id)
{
	if (id == m_iRunOnceID)
		m_iTrials = 0;
	else if (id == m_iRunMonteCarloID)
	{
		m_iTrials = m_spinTrials->value();
		m_chkDuration->setChecked(true);
		slotDurationToggled(true);
	}
	m_spinTrials->setEnabled(id == m_iRunMonteCarloID);
	m_chkDuration->setEnabled(id == m_iRunOnceID);
}

void QSimRunDialog::slotTrialsChanged(int value)
{
	if (m_groupSimType->selectedId() == m_iRunMonteCarloID)
		m_iTrials = value;
}

void QSimRunDialog::slotDurationToggled(bool on)
{
	m_txtDuration->setEnabled(on);
	if (on)
		m_tDuration = MakeTime(ValidateNumber(StringToNumber(m_txtDuration->text()), 0., HUGE_VAL));
	else
		m_tDuration = timeval0;
}

void QSimRunDialog::slotDurationChanged(const QString & strText)
{
	if (m_chkDuration->isChecked())
		m_tDuration = MakeTime(ValidateNumber(StringToNumber(strText), 0., HUGE_VAL));
}

void QSimRunDialog::slotSimulationTime(int id)
{
	if (id == m_iRealTimeID)
		m_tIncrement = timeval0;
	else if (id == m_iFastTimeID)
		m_tIncrement = MakeTime(ValidateNumber(StringToNumber(m_txtIncrement->text()), 0., HUGE_VAL));
	m_txtIncrement->setEnabled(id == m_iFastTimeID);
}

void QSimRunDialog::slotIncrementChanged(const QString & strText)
{
	if (m_groupTime->selectedId() == m_iFastTimeID)
		m_tIncrement = MakeTime(ValidateNumber(StringToNumber(strText), 0., HUGE_VAL));
}

void QSimRunDialog::slotMessageHighlighted(int index)
{
	m_buttonEditMsg->setEnabled(index > -1 && index < (signed)m_listMessages->count());
	m_buttonRemoveMsg->setEnabled(index > -1 && index < (signed)m_listMessages->count());
}

void QSimRunDialog::slotAddMessage()
{
	QMessageDialog * pDialog = new QMessageDialog(0, this);
	pDialog->m_buttonOK->setText("Add");
	pDialog->setCaption("GrooveNet - Add Event Message...");
	if (pDialog->exec() == QDialog::Accepted)
	{
		EventMessage msg;
		//don't have m_eType so get rid of this
		//TODO: remove the emergency/warning buttons from gui -MH
		/*
		if (pDialog->m_groupMsgType->selectedId() == pDialog->m_groupMsgType->id(pDialog->m_buttonEmergency))
			msg.eType = Message::MessageTypeEmergency;
		else
			msg.eType = Message::MessageTypeWarning;
		*/
		msg.strMessage = pDialog->m_comboMsgText->currentText();
		msg.sBoundingRegion = pDialog->m_sBoundingRegion;
		msg.strDest = pDialog->m_txtDest->text();
		msg.tTransmit = MakeTime(ValidateNumber(StringToNumber(pDialog->m_txtTXTime->text()), 0., HUGE_VAL));
		msg.tLifetime = MakeTime(pDialog->m_spinMsgLifetime->value() * 1e-3f);
		msg.ipSource = pDialog->m_comboMsgSource->currentItem() > -1 ? pDialog->m_vecMsgSources[pDialog->m_comboMsgSource->currentItem()] : 0;
		m_vecMessages.push_back(msg);
		m_listMessages->insertItem(QString("%1 from %2: t=%3->%4, \"%5\"").arg("Safety").arg(IPAddressToString(msg.ipSource)).arg(ToDouble(msg.tTransmit)).arg(ToDouble(msg.tTransmit+msg.tLifetime)).arg(msg.strMessage));
		m_listMessages->setCurrentItem(m_listMessages->count() - 1);
		slotMessageHighlighted(m_listMessages->currentItem());
	}
}

void QSimRunDialog::slotEditMessage()
{
	int index = m_listMessages->currentItem();
	if (index < 0 || index >= (signed)m_listMessages->count())
		return;
	EventMessage & msg = m_vecMessages[index];
	QMessageDialog * pDialog = new QMessageDialog(0, this);
	unsigned int i;
	for (i = 0; i < pDialog->m_vecMsgSources.size(); i++)
	{
		if (pDialog->m_vecMsgSources[i] == msg.ipSource)
		{
			pDialog->m_comboMsgSource->setCurrentItem(i);
			break;
		}
	}
	//again, no more m_eType -MH
	/*
	pDialog->m_groupMsgType->setButton(pDialog->m_groupMsgType->id(msg.eType == Message::MessageTypeEmergency ? pDialog->m_buttonEmergency : pDialog->m_buttonWarning));
	*/
	pDialog->m_comboMsgText->setCurrentText(msg.strMessage);
	pDialog->m_sBoundingRegion = msg.sBoundingRegion;
	pDialog->UpdateBoundingRegion();
	pDialog->m_txtDest->setText(msg.strDest);
	pDialog->m_txtTXTime->setText(QString("%1").arg(ToDouble(msg.tTransmit)));
	pDialog->m_spinMsgLifetime->setValue((int)(ToDouble(msg.tLifetime) * 1e3));
	pDialog->m_buttonOK->setText("Change");
	setCaption("GrooveNet - Edit Event Message...");
	if (pDialog->exec() == QDialog::Accepted)
	{
		//no more m_eType -MH
		/*
		if (pDialog->m_groupMsgType->selectedId() == pDialog->m_groupMsgType->id(pDialog->m_buttonEmergency))
			msg.eType = Message::MessageTypeEmergency;
		else
			msg.eType = Message::MessageTypeWarning;
		*/
		msg.strMessage = pDialog->m_comboMsgText->currentText();
		msg.sBoundingRegion = pDialog->m_sBoundingRegion;
		msg.strDest = pDialog->m_txtDest->text();
		msg.tTransmit = MakeTime(ValidateNumber(StringToNumber(pDialog->m_txtTXTime->text()), 0., HUGE_VAL));
		msg.tLifetime = MakeTime(pDialog->m_spinMsgLifetime->value() * 1e-3f);
		msg.ipSource = pDialog->m_comboMsgSource->currentItem() > -1 ? pDialog->m_vecMsgSources[pDialog->m_comboMsgSource->currentItem()] : 0;
		m_listMessages->changeItem(QString("%1 from %2: t=%3->%4, \"%5\"").arg("Safety").arg(IPAddressToString(msg.ipSource)).arg(ToDouble(msg.tTransmit)).arg(ToDouble(msg.tTransmit+msg.tLifetime)).arg(msg.strMessage), index);
	}
}

void QSimRunDialog::slotRemoveMessage()
{
	int index = m_listMessages->currentItem();
	if (index > -1 && index < (signed)m_listMessages->count())
	{
		m_vecMessages.erase(m_vecMessages.begin() + index);
		m_listMessages->removeItem(index);
	}
}

void QSimRunDialog::slotProfileToggled(bool on)
{
	m_bProfile = on;
}

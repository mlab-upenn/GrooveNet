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

#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qlineedit.h>

#include "QMessageDialog.h"
#include "QBoundingRegionConfDialog.h"
#include "CarRegistry.h"
#include "InfrastructureNodeRegistry.h"
#include "StringHelp.h"
#include "Simulator.h"
#include "Logger.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qgroupbox.h>

#include "app16x16.xpm"

QMessageDialog::QMessageDialog(in_addr_t ipCar, QWidget * parent, const char * name, WFlags f)
: QDialog(parent, name, f), m_ipCar(ipCar), m_pfnAcceptCallback(NULL)
{
	std::map<in_addr_t, CarModel *> * pCarRegistry = g_pCarRegistry->acquireLock();
	std::map<in_addr_t, CarModel *>::iterator iterCar;
	std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterNode;
	QWidget * boxMsgSource = new QWidget(this);
	QLabel * labelMsgSource = new QLabel("Message Source:", boxMsgSource);
	QWidget * boxMsgText = new QWidget(this);
	QLabel * labelMsgText = new QLabel("Message:", boxMsgText);
	QGroupBox * boxBoundingRegion = new QGroupBox(1, Qt::Vertical, "Message Bounding Region", this);
	QWidget * boxMsgDest = new QWidget(this);
	QWidget * boxMsgLifetime = new QWidget(this);
	QLabel * labelMsgLifetime = new QLabel("Message lifetime:", boxMsgLifetime);
	QWidget * boxButtons = new QWidget(this);

	QVBoxLayout * pLayout = new QVBoxLayout(this, 8, 8);
	QHBoxLayout * pMsgSourceBoxLayout = new QHBoxLayout(boxMsgSource, 0, 8);
	QHBoxLayout * pMsgTextBoxLayout = new QHBoxLayout(boxMsgText, 0, 8);
	QHBoxLayout * pMsgDestBoxLayout = new QHBoxLayout(boxMsgDest, 0, 8);
	QHBoxLayout * pMsgLifetimeBoxLayout = new QHBoxLayout(boxMsgLifetime, 0, 8);
	QHBoxLayout * pButtonBoxLayout = new QHBoxLayout(boxButtons, 0, 8);

	m_sBoundingRegion.eRegionType = SafetyPacket::BoundingRegionTypeNone;

	m_comboMsgSource = new QComboBox(false, boxMsgSource);
	m_groupMsgType = new QButtonGroup(2, Qt::Horizontal, "Message Type", this);
	m_buttonEmergency = new QRadioButton("Emergency Message", m_groupMsgType);
	m_buttonWarning = new QRadioButton("Warning Message", m_groupMsgType);
	m_labelMsgRegionType = new QLabel("Bounding region type:", boxBoundingRegion);
	m_comboMsgRegionType = new QComboBox(false, boxBoundingRegion);
	m_buttonMsgRegionConf = new QPushButton("Configure...", boxBoundingRegion);
	m_comboMsgText = new QComboBox(true, boxMsgText);
	m_labelDest = new QLabel("Message destination address:", boxMsgDest);
	m_txtDest = new QLineEdit("", boxMsgDest);
	m_labelTXTime = new QLabel("Transmit time:", boxMsgLifetime);
	m_txtTXTime = new QLineEdit("0.", boxMsgLifetime);
	m_spinMsgLifetime = new QSpinBox(0, 999999999, 100, boxMsgLifetime);
	m_buttonOK = new QPushButton("&Send", boxButtons);
	m_buttonCancel = new QPushButton("&Cancel", boxButtons);

	connect(m_groupMsgType, SIGNAL(clicked(int)), this, SLOT(slotMsgTypeChanged(int)));
	connect(m_comboMsgRegionType, SIGNAL(activated(int)), this, SLOT(slotBoundingRegionTypeChanged(int)));
	connect(m_buttonMsgRegionConf, SIGNAL(clicked()), this, SLOT(slotBoundingRegionConfig()));
	connect(m_buttonOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

	// populate list of messages
	LoadMessages();

	if (m_ipCar != 0)
	{
		iterCar = pCarRegistry->find(m_ipCar);
		if (iterCar->second != NULL)
		{
			m_vecMsgSources.push_back(iterCar->first);
			m_comboMsgSource->insertItem(QString("%1 (:%2)").arg(IPAddressToString(iterCar->first)).arg(IPAddressToString(iterCar->second->GetOwnerIPAddress() == CARMODEL_IPOWNER_LOCAL ? ::GetIPAddress() : iterCar->second->GetOwnerIPAddress())));
		}
	}
	else
	{
		// populate list of vehicles
		for (iterCar = pCarRegistry->begin(); iterCar != pCarRegistry->end(); ++iterCar)
		{
			if (iterCar->second != NULL)
			{
				m_vecMsgSources.push_back(iterCar->first);
				m_comboMsgSource->insertItem(QString("%1 (:%2)").arg(IPAddressToString(iterCar->first)).arg(IPAddressToString(iterCar->second->GetOwnerIPAddress() == CARMODEL_IPOWNER_LOCAL ? ::GetIPAddress() : iterCar->second->GetOwnerIPAddress())));
			}
		}
	}
	g_pCarRegistry->releaseLock();

	m_comboMsgRegionType->insertItem("None");
	m_comboMsgRegionType->insertItem("Bounding Box (rectangular)");
	m_comboMsgRegionType->insertItem("Waypoint Bounding Region");
	m_comboMsgRegionType->insertItem("Circular Bounding Region");
	m_comboMsgRegionType->insertItem("Directional Bounding Region");
	m_comboMsgRegionType->setCurrentItem(0);
	slotBoundingRegionTypeChanged(0);

	m_groupMsgType->setButton(m_groupMsgType->id(m_buttonEmergency));
	slotMsgTypeChanged(m_groupMsgType->selectedId());
	m_spinMsgLifetime->setValue(60000);
	m_spinMsgLifetime->setSuffix(" ms");
	m_buttonOK->setAutoDefault(false);
	m_buttonCancel->setAutoDefault(false);

	pLayout->addWidget(boxMsgSource);
	pLayout->addWidget(m_groupMsgType);
	pLayout->addWidget(boxMsgText);
	pLayout->addWidget(boxBoundingRegion);
	pLayout->addWidget(boxMsgDest);
	pLayout->addWidget(boxMsgLifetime);
	pLayout->addWidget(boxButtons);
	pMsgSourceBoxLayout->addWidget(labelMsgSource, 0, Qt::AlignLeft);
	pMsgSourceBoxLayout->addWidget(m_comboMsgSource, 1);
	pMsgTextBoxLayout->addWidget(labelMsgText, 0, Qt::AlignLeft);
	pMsgTextBoxLayout->addWidget(m_comboMsgText, 1);
	pMsgDestBoxLayout->addWidget(m_labelDest, 0, Qt::AlignLeft);
	pMsgDestBoxLayout->addWidget(m_txtDest, 1);
	pMsgLifetimeBoxLayout->addWidget(m_labelTXTime, 0, Qt::AlignLeft);
	pMsgLifetimeBoxLayout->addWidget(m_txtTXTime, 1);
	pMsgLifetimeBoxLayout->addWidget(labelMsgLifetime, 0, Qt::AlignLeft);
	pMsgLifetimeBoxLayout->addWidget(m_spinMsgLifetime, 1);
	pButtonBoxLayout->addStretch(1);
	pButtonBoxLayout->addWidget(m_buttonOK, 0, Qt::AlignCenter);
	pButtonBoxLayout->addWidget(m_buttonCancel, 0, Qt::AlignCenter);

	setCaption("GrooveNet - Send Message...");
	setIcon(app16x16_xpm);
}

QMessageDialog::~QMessageDialog()
{
}

void QMessageDialog::LoadMessages()
{
	QFile file(GetDataPath("messages.txt"));
	QTextStream reader;
	QString strLine;
	std::vector<QString> * pMessages = NULL;

	if (!file.open(IO_ReadOnly | IO_Translate))
		return;

	m_vecEmergencyMessages.clear();
	m_vecWarningMessages.clear();

	reader.setDevice(&file);
	while (!(strLine = reader.readLine()).isNull())
	{
		if (strLine.isEmpty() || strLine[0] == '%')
			continue;

		if (strLine.compare("<Emergency>") == 0)
			pMessages = &m_vecEmergencyMessages;
		else if (strLine.compare("<Warning>") == 0)
			pMessages = &m_vecWarningMessages;
		else if (pMessages != NULL)
			pMessages->push_back(strLine.stripWhiteSpace());
	}
	reader.unsetDevice();
	file.close();
}

void QMessageDialog::UpdateBoundingRegion()
{
	m_comboMsgRegionType->setCurrentItem(m_sBoundingRegion.eRegionType);
	slotBoundingRegionTypeChanged(m_sBoundingRegion.eRegionType);
}

void QMessageDialog::slotMsgTypeChanged(int id)
{
	unsigned int i;

	m_comboMsgText->clear();
	if (id == m_groupMsgType->id(m_buttonEmergency)) // all
	{
		for (i = 0; i < m_vecEmergencyMessages.size(); i++)
			m_comboMsgText->insertItem(m_vecEmergencyMessages[i]);
	}
	else // selected
	{
		for (i = 0; i < m_vecWarningMessages.size(); i++)
			m_comboMsgText->insertItem(m_vecWarningMessages[i]);
	}
	m_comboMsgText->setCurrentItem(m_comboMsgText->count() > 0 ? 0 : -1);
}

void QMessageDialog::slotBoundingRegionTypeChanged(int index)
{
	m_buttonMsgRegionConf->setEnabled(index > 0);
	SafetyPacket::BoundingRegionType eOldType = m_sBoundingRegion.eRegionType, eNewType = index > 0 ? (SafetyPacket::BoundingRegionType)index : SafetyPacket::BoundingRegionTypeNone;

	if (eOldType != eNewType)
	{
		m_sBoundingRegion.eRegionType = eNewType;
		if (eNewType > 0)
		{
			SafetyPacket::BoundingRegion sBoundingRegion;
			sBoundingRegion.eRegionType = eNewType;
			sBoundingRegion.fParam = 0.;
			QBoundingRegionConfDialog * pDialog = new QBoundingRegionConfDialog(sBoundingRegion, this);
		
			if (pDialog->exec() == QDialog::Accepted)
				m_sBoundingRegion = pDialog->m_sBoundingRegion;
			else
			{
				m_sBoundingRegion.eRegionType = eOldType;
				m_comboMsgRegionType->setCurrentItem(eOldType);
				m_buttonMsgRegionConf->setEnabled(eOldType > 0);
			}
		
			delete pDialog;
		}
	}
}

void QMessageDialog::slotBoundingRegionConfig()
{
	QBoundingRegionConfDialog * pDialog = new QBoundingRegionConfDialog(m_sBoundingRegion, this);

	if (pDialog->exec() == QDialog::Accepted)
		m_sBoundingRegion = pDialog->m_sBoundingRegion;

	delete pDialog;
}

void QMessageDialog::accept()
{
	if (m_pfnAcceptCallback != NULL)
		(*m_pfnAcceptCallback)(this);

	QDialog::accept();
}

void DispatchMessage(QMessageDialog * pDialog)
{
	SafetyPacket msg;
	std::map<in_addr_t, CarModel *> * pCarRegistry = g_pCarRegistry->acquireLock();
	std::map<in_addr_t, CarModel *>::iterator iterCar = pCarRegistry->find(pDialog->m_vecMsgSources[pDialog->m_comboMsgSource->currentItem()]);
	CarModel * pCar = (iterCar == pCarRegistry->end() ? NULL : iterCar->second);

	if (pCar != NULL)
	{
		Event1Message msgEvent;
		Address sDest;
		pCar->CreateMessage(&msg);
		//no m_eType now -MH
		/*
		if (pDialog->m_groupMsgType->selectedId() == pDialog->m_groupMsgType->id(pDialog->m_buttonEmergency))
			msg.m_eType = SafetyPacket::MessageTypeEmergency;
		else
			msg.m_eType = SafetyPacket::MessageTypeWarning;
		*/
		msg.m_sBoundingRegion = pDialog->m_sBoundingRegion;
		msg.m_pData = (unsigned char *)strdup(pDialog->m_comboMsgText->currentText());
		msg.m_iDataLength = pDialog->m_comboMsgText->currentText().length() + 1;

		g_pSimulator->m_mutexEvent1Log.lock();
		msgEvent.ID = msg.m_ID.srcID;
		msgEvent.tMessage = g_pSimulator->m_tCurrent;
		msgEvent.tLifetime = msg.m_tLifetime = MakeTime(pDialog->m_spinMsgLifetime->value() * 1e-3f);
		msgEvent.ptOrigin = pCar->GetCurrentPosition();
		if (!pDialog->m_txtDest->text().isEmpty() && StringToAddress(pDialog->m_txtDest->text(), &sDest))
			msgEvent.ptDest = sDest.ptCoordinates;
		else
			msgEvent.ptDest.Set(0, 0);
		msgEvent.fDistance = 0.f;
		msgEvent.fOriginatorDistance = 0.f;
		msgEvent.iCars = 1;
		g_pSimulator->m_mapEvent1Log.insert(std::pair<PacketSequence, Event1Message>(msgEvent.ID, msgEvent));
		g_pSimulator->m_mutexEvent1Log.unlock();

		g_pSimulator->m_msgCurrentTrack = msg.m_ID.srcID;
		if (pCar->IsLoggingEnabled())
			g_pLogger->WriteMessage(LOGFILE_MESSAGES, &msg);
		pCar->TransmitPacket(&msg);
		if (pCar->m_pCommModel != NULL)
			pCar->m_pCommModel->AddMessageToRebroadcastQueue(msg);
	}
	g_pCarRegistry->releaseLock();
}

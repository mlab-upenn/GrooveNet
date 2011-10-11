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

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qlistbox.h>

#include <set>

#include "QNetworkManager.h"

#include "Network.h"
#include "StringHelp.h"

QNetworkManager::QNetworkManager(QWidget * parent, const char * name, Qt::WFlags f)
: QWidget(parent, name, f)
{
	QHBoxLayout * pLayout, * pClientsAddLayout;
	QVBoxLayout * pClientsLayout, * pNetworkLayout;
	QSizePolicy policy;

	std::map<QString, ServerCreator>::iterator iterServerCreator;
	std::map<QString, ClientCreator>::iterator iterClientCreator;

	m_pNetworkServerMenu = new QPopupMenu(this);
	connect(m_pNetworkServerMenu, SIGNAL(activated(int)), this, SLOT(slotNetworkServer(int)));
	for (iterServerCreator = g_mapServerCreators.begin(); iterServerCreator != g_mapServerCreators.end(); ++iterServerCreator)
		m_pNetworkServerMenu->insertItem(iterServerCreator->first);

	m_pNetworkClientMenu = new QPopupMenu(this);
	connect(m_pNetworkClientMenu, SIGNAL(activated(int)), this, SLOT(slotClientConnect(int)));
	for (iterClientCreator = g_mapClientCreators.begin(); iterClientCreator != g_mapClientCreators.end(); ++iterClientCreator)
		m_pNetworkClientMenu->insertItem(iterClientCreator->first);

	m_boxClients = new QWidget(this, "clients.box");
	m_boxClientsAdd = new QWidget(m_boxClients, "clients.add.box");
	m_lblClientsAddVehicle = new QLabel("Vehicle/Simulator:", m_boxClientsAdd);
	m_lblClientsAddVehicle->setEnabled(false);
	m_txtClientsAddVehicle = new QLineEdit(m_boxClientsAdd, "clients.add.vehicle");
	m_txtClientsAddVehicle->setEnabled(false);
	connect(m_txtClientsAddVehicle, SIGNAL(textChanged(const QString& )), this, SLOT(slotClientTextChanged(const QString& )));
	m_btnClientsAddConnect = new QPushButton("Connect", m_boxClientsAdd, "clients.add.connect");
	m_btnClientsAddConnect->setEnabled(false);
	m_btnClientsAddConnect->setPopup(m_pNetworkClientMenu);
	m_btnClientsAddDisconnect = new QPushButton("Disconnect", m_boxClientsAdd, "clients.add.disconnect");
	m_btnClientsAddDisconnect->setEnabled(false);
	connect(m_btnClientsAddDisconnect, SIGNAL(clicked()), this, SLOT(slotClientDisconnect()));
	m_listClients = new QListBox(m_boxClients, "clients.list");
	m_listClients->setEnabled(false);
	connect(m_listClients, SIGNAL(highlighted(int)), this, SLOT(slotClientListHighlighted(int)));
	m_boxNetwork = new QWidget(this, "network.box");
	m_btnNetworkInit = new QPushButton("Initialize", m_boxNetwork, "network.init");
	connect(m_btnNetworkInit, SIGNAL(clicked()), this, SLOT(slotNetworkInit()));
	m_btnNetworkClose = new QPushButton("Close", m_boxNetwork, "network.close");
	m_btnNetworkClose->setEnabled(false);
	connect(m_btnNetworkClose, SIGNAL(clicked()), this, SLOT(slotNetworkClose()));
	m_btnNetworkServer = new QPushButton("Start Server", m_boxNetwork, "network.server");
	m_btnNetworkServer->setEnabled(false);
	m_btnNetworkServer->setPopup(m_pNetworkServerMenu);
	connect(m_btnNetworkServer, SIGNAL(clicked()), this, SLOT(slotNetworkServer()));

	pLayout = new QHBoxLayout(this, 8, 8, "layout");
	pClientsLayout = new QVBoxLayout(m_boxClients, 0, 8, "clients.layout");
	pClientsAddLayout = new QHBoxLayout(m_boxClientsAdd, 0, 8, "clients.add.layout");
	pNetworkLayout = new QVBoxLayout(m_boxNetwork, 0, 8, "network.layout");

	pNetworkLayout->addWidget(m_btnNetworkInit, 0, Qt::AlignCenter);
	pNetworkLayout->addWidget(m_btnNetworkClose, 0, Qt::AlignCenter);
	pNetworkLayout->addWidget(m_btnNetworkServer, 0, Qt::AlignCenter);
	pNetworkLayout->addStretch(1);

	pClientsAddLayout->addWidget(m_lblClientsAddVehicle, 0, 0);
	pClientsAddLayout->addWidget(m_txtClientsAddVehicle, 1, 0);
	pClientsAddLayout->addWidget(m_btnClientsAddConnect, 0, 0);
	pClientsAddLayout->addWidget(m_btnClientsAddDisconnect, 0, Qt::AlignCenter);

	pClientsLayout->addWidget(m_boxClientsAdd, 0, 0);
	pClientsLayout->addWidget(m_listClients, 1, 0);

	pLayout->addWidget(m_boxClients, 1, 0);
	pLayout->addWidget(m_boxNetwork, 0, 0);
}

QNetworkManager::~QNetworkManager()
{
}

void QNetworkManager::SetNetworkInitialized(bool bInitialized)
{
	if (bInitialized)
	{
		m_btnNetworkInit->setText("Reinitialize");
		m_btnNetworkClose->setEnabled(true);
		m_btnNetworkServer->setEnabled(true);
		m_lblClientsAddVehicle->setEnabled(true);
		m_txtClientsAddVehicle->setEnabled(true);
		slotClientTextChanged(m_txtClientsAddVehicle->text());
		m_listClients->setEnabled(true);
	}
	else
	{
		m_btnNetworkInit->setText("Initialize");
		m_btnNetworkClose->setEnabled(false);
		m_btnNetworkServer->setEnabled(false);
		m_lblClientsAddVehicle->setEnabled(false);
		m_txtClientsAddVehicle->setEnabled(false);
		m_btnClientsAddConnect->setEnabled(false);
		m_btnClientsAddDisconnect->setEnabled(false);
		m_listClients->setEnabled(false);
	}
}

void QNetworkManager::SetServerRunning(bool bRunning)
{
	if (bRunning)
	{
		m_btnNetworkServer->setPopup(NULL);
		m_btnNetworkServer->setText("Stop Server");
	}
	else
	{
		m_btnNetworkServer->setText("Start Server");
		m_btnNetworkServer->setPopup(m_pNetworkServerMenu);
	}
}

void QNetworkManager::UpdateClientTable(in_addr_t ipCurrent)
{
	std::map<in_addr_t, Client *> * pConnections = GetConnections();
	std::map<in_addr_t, Client *>::iterator iterConnection;
	std::set<in_addr_t> setConnections;
	QString strText;
	int iCurrent;
	unsigned int i = 0;
	bool bReal;
	in_addr_t ipReal;

	m_mutexClients.lock();
	iCurrent = m_listClients->currentItem();
	// first, update clients already in table
	while (i < m_listClients->count())
	{
		strText = m_listClients->text(i);
		bReal = StringToIPAddress(strText, ipReal);
		if (bReal && (iterConnection = pConnections->find(ipReal)) != pConnections->end())
		{
			setConnections.insert(ipReal);
			if (ipCurrent == ipReal)
				iCurrent = i;
			i++;
		}
		else
		{
			m_listClients->removeItem(i);
			if (iCurrent == (signed)i)
				iCurrent = -1;
			else if (iCurrent > (signed)i)
				iCurrent--;
		}
	}

	// secondly, add clients not in table
	for (iterConnection = pConnections->begin(); iterConnection != pConnections->end(); ++iterConnection)
	{
		if (setConnections.find(iterConnection->first) == setConnections.end()) {
			if (iterConnection->first == ipCurrent)
				iCurrent = m_listClients->count();
			m_listClients->insertItem(IPAddressToString(iterConnection->first));
		}
	}
	ReleaseConnections();

	m_listClients->setCurrentItem(iCurrent);
	slotClientListHighlighted(m_listClients->currentItem());
	slotClientTextChanged(m_txtClientsAddVehicle->text());
	m_mutexClients.unlock();
}

void QNetworkManager::slotNetworkInit()
{
	InitNetwork();
}

void QNetworkManager::slotNetworkClose()
{
	CloseNetwork();
}

void QNetworkManager::slotNetworkServer()
{
	if (IsServerRunning())
		StopServer();
}

void QNetworkManager::slotNetworkServer(int id)
{
	StartServer(m_pNetworkServerMenu->text(id));
}

void QNetworkManager::slotClientTextChanged(const QString & strText)
{
	std::vector<in_addr_t> vecVehicles;
	bool bReal = ParseVehicleIDs(strText, vecVehicles);
	Client * pClient = NULL;
	unsigned int i;

	if (bReal)
	{
		for (i = 0; i < vecVehicles.size(); i++)
		{
			if ((pClient = GetClient(vecVehicles[i])) != NULL)
				break;
		}
	}

	if (bReal && pClient != NULL)
		m_btnClientsAddConnect->setText("Reconnect");
	else
		m_btnClientsAddConnect->setText("Connect");
	m_btnClientsAddConnect->setEnabled(bReal);
	m_btnClientsAddDisconnect->setEnabled(pClient != NULL);
}

void QNetworkManager::slotClientConnect(int id)
{
	std::vector<in_addr_t> vecVehicles;
	QString strText = m_txtClientsAddVehicle->text();
	bool bValid = ParseVehicleIDs(strText, vecVehicles);
	unsigned int i;

	if (bValid)
	{
		for (i = 0; i < vecVehicles.size(); i++)
			OpenConnection(vecVehicles[i], m_pNetworkClientMenu->text(id));
		if (!vecVehicles.empty())
			UpdateClientTable(vecVehicles.back());
	}
}

void QNetworkManager::slotClientDisconnect()
{
	std::vector<in_addr_t> vecVehicles;
	QString strText = m_txtClientsAddVehicle->text();
	bool bValid = ParseVehicleIDs(strText, vecVehicles);
	unsigned int i;

	if (bValid) {
		for (i = 0; i < vecVehicles.size(); i++)
			CloseConnection(vecVehicles[i]);
		if (!vecVehicles.empty())
			UpdateClientTable(vecVehicles.front());
	}
}

void QNetworkManager::slotClientListHighlighted(int index)
{
	if (index > -1)
		m_txtClientsAddVehicle->setText(m_listClients->text(index));
	slotClientTextChanged(m_txtClientsAddVehicle->text());
}

bool QNetworkManager::ParseVehicleIDs(const QString & strText, std::vector<in_addr_t> & vecVehicles)
{
	int iSep2;
	in_addr_t ipR1, ipR2, ipTemp, ipSubnet = GetIPSubnet(), ipAddress = GetIPAddress();
	bool bReal;

	iSep2 = strText.find('-');
	if (iSep2 > -1)
	{
		if (bReal = (StringToIPAddress(strText.left(iSep2), ipR1) && StringToIPAddress(strText.mid(iSep2+1), ipR2) && (ipR1 & ipSubnet) == (ipAddress & ipSubnet) && (ipR2 & ipSubnet) == (ipAddress & ipSubnet)))
		{
			if (ipR1 > ipR2)
			{
				ipTemp = ipR1;
				ipR1 = ipR2;
				ipR2 = ipTemp;
			}
			if (ipR2 - ipR1 > 999)
				ipR2 = ipR1 + 999;
			for (ipTemp = ipR1; ipTemp <= ipR2; ipTemp++)
				vecVehicles.push_back(ipTemp);
		}
	}
	else
	{
		if (bReal = StringToIPAddress(strText, ipR1) && (ipR1 & ipSubnet) == (ipAddress & ipSubnet))
			vecVehicles.push_back(ipR1);
	}
	return bReal;
}

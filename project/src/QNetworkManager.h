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

#ifndef _QNETWORKMANAGER_H
#define _QNETWORKMANAGER_H

#include <qwidget.h>
#include <qmutex.h>

#include <arpa/inet.h>

#include <vector>

class QLabel;
class QLineEdit;
class QPushButton;
class QPopupMenu;
class QListBox;

class QNetworkManager : public QWidget
{
Q_OBJECT
public:
	QNetworkManager(QWidget * parent = NULL, const char * name = 0, Qt::WFlags f = 0);
	virtual ~QNetworkManager();

	virtual void SetNetworkInitialized(bool bInitialized);
	virtual void SetServerRunning(bool bRunning);
	virtual void UpdateClientTable(in_addr_t ipCurrent = 0);

protected slots:
	virtual void slotNetworkInit();
	virtual void slotNetworkClose();
	virtual void slotNetworkServer();
	virtual void slotNetworkServer(int id);
	virtual void slotClientTextChanged(const QString & strText);
	virtual void slotClientConnect(int id);
	virtual void slotClientDisconnect();
	virtual void slotClientListHighlighted(int index);

protected:
	bool ParseVehicleIDs(const QString & strText, std::vector<in_addr_t> & vecVehicles);

	QWidget * m_boxClients;
	QWidget * m_boxClientsAdd;
	QLabel * m_lblClientsAddVehicle;
	QLineEdit * m_txtClientsAddVehicle;
	QPushButton * m_btnClientsAddConnect;
	QPushButton * m_btnClientsAddDisconnect;
	QListBox * m_listClients;
	QWidget * m_boxNetwork;
	QPushButton * m_btnNetworkInit;
	QPushButton * m_btnNetworkClose;
	QPushButton * m_btnNetworkServer;
	QPopupMenu * m_pNetworkServerMenu;
	QPopupMenu * m_pNetworkClientMenu;

	QMutex m_mutexClients;
};

#endif

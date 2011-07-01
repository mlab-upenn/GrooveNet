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

#ifndef _UDP_H
#define _UDP_H

#include "Network.h"

#include <qmutex.h>

#define UDPSERVER_NAME "UDP Server"

class UDPServer : public Server
{
public:
	UDPServer();
	virtual ~UDPServer();

	virtual bool Start();
	virtual bool IsRunning() const;
	virtual bool Stop();

protected:
	int m_iSocketFD;
	struct sockaddr_in m_sServer;
};

inline Server * UDPServerCreator()
{
	return new UDPServer();
}


#define UDPCLIENT_NAME "UDP Client"

class UDPClient : public Client
{
public:
	UDPClient();
	virtual ~UDPClient();

	virtual bool Start(in_addr_t ipTo);
	virtual bool IsRunning() const;
	virtual bool Select(struct timeval tWait = timeval0);
	virtual bool Write(const unsigned char * pBuffer, int iLength);
	virtual bool Stop();

protected:
	int m_iSocketFD;
	struct sockaddr_in m_sClient, m_sServer;
	QMutex m_mutexWrite;
};

inline Client * UDPClientCreator()
{
	return new UDPClient();
}

#endif

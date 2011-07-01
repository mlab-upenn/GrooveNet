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

#ifndef _TCP_H
#define _TCP_H

#include "Network.h"
#include "QMessageList.h"

#include <qmutex.h>

class TCPServer;

#ifndef MAX_DEADLOCK
#define MAX_DEADLOCK 300
#endif

class TCPListener : public QThread
{
public:
	TCPListener(TCPServer * pServer);
	~TCPListener();

	virtual void start(Priority priority = InheritPriority);
	virtual bool wait(unsigned long time = ULONG_MAX);
	bool Select(struct timeval tWait = timeval0);

protected:
	virtual void run();

	TCPServer * m_pServer;
	bool m_bCancelled;
};


#define TCPSERVER_NAME "TCP Server"

class TCPServer : public Server
{
public:
	TCPServer();
	virtual ~TCPServer();

	virtual bool Start();
	virtual bool IsRunning() const;
	virtual bool Read(const fd_set * pFDs, std::list<ReadBuffer> & listRead);
	virtual bool Stop();

protected:
	int m_iSocketFD;
	struct sockaddr_in m_sServer;
	TCPListener * m_pListener;
	std::map<int, struct sockaddr_in> m_mapConnections;

	friend class TCPListener;
};

inline Server * TCPServerCreator()
{
	return new TCPServer();
}


#define TCPCLIENT_NAME "TCP Client"

class TCPClient : public Client
{
public:
	TCPClient();
	virtual ~TCPClient();

	virtual bool Start(in_addr_t ipTo);
	virtual bool IsRunning() const;
	virtual bool Select(struct timeval tWait = timeval0);
	virtual bool Write(const unsigned char * pBuffer, int iLength);
	virtual bool Stop();

protected:
	int m_iSocketFD;
	struct sockaddr_in m_sClient, m_sServer;
	QMutex m_mutexWrite;

private:
	unsigned char * writeBuffer;
	unsigned int bufferLength;
	unsigned int maxBufferSize;
	time_t lastUpdateTime;
};

inline Client * TCPClientCreator()
{
	return new TCPClient();
}

extern QMessageList * m_pMessageList;

#endif

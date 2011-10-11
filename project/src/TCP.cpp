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

#include "TCP.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define TCPSERVER_PORT 40002
#define TCPCLIENT_PORT 0

#include "Logger.h"

TCPListener::TCPListener(TCPServer * pServer)
: QThread(), m_pServer(pServer), m_bCancelled(false)
{
}

TCPListener::~TCPListener()
{
}

void TCPListener::start(Priority priority)
{
	while (!wait(MAX_DEADLOCK))
		terminate();
	m_bCancelled = false;
	QThread::start(priority);
}

bool TCPListener::wait(unsigned long time)
{
	m_bCancelled = true;
	return QThread::wait(time);
}

bool TCPListener::Select(struct timeval tWait)
{
	fd_set rfds;

	if (m_pServer->m_iSocketFD == -1)
		return false;

	FD_ZERO(&rfds);
	FD_SET(m_pServer->m_iSocketFD, &rfds);

	return TEMP_FAILURE_RETRY(::select(FD_SETSIZE, &rfds, NULL, NULL, &tWait)) > 0 && FD_ISSET(m_pServer->m_iSocketFD, &rfds);
}

void TCPListener::run()
{
	std::pair<int, struct sockaddr_in> net;
	socklen_t netLength;
	const struct timeval tWait = {0, 100};

	while (!m_bCancelled) {
		netLength = sizeof(net.second);
		if (m_pServer->m_iSocketFD > -1 && (net.first = ::accept(m_pServer->m_iSocketFD, (struct sockaddr *)&net.second, &netLength)) > -1) {
			m_pServer->m_mutexConnections.lock();
			if (m_pServer->m_mapConnections.insert(net).second)
				m_pServer->m_vecConnections.push_back(net.first);
			m_pServer->m_mutexConnections.unlock();
		}
		//printf("TCP Run Loop\n");
		Select(tWait);
	}
}


TCPServer::TCPServer()
: Server(), m_iSocketFD(-1), m_pListener(NULL)
{
}

TCPServer::~TCPServer()
{
	if (m_pListener != NULL)
		delete m_pListener;
}

bool TCPServer::Start()
{
	std::map<int, sockaddr_in>::const_iterator iterConnection;
	if (!Stop())
		return false; // can't stop previous connection

	m_iSocketFD = TEMP_FAILURE_RETRY(::socket(AF_INET, SOCK_STREAM, 0));
	if (m_iSocketFD == -1)
		return false;

	//don't wait to reuse socket if crashes -MH
	int optval = 1;
	if(setsockopt(m_iSocketFD, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0) return false;

	//::fcntl(m_iSocketFD, F_SETFL, O_NONBLOCK | fcntl(m_iSocketFD, F_GETFL));
	//made it blocking now -MH
	::fcntl(m_iSocketFD, F_SETFL, fcntl(m_iSocketFD, F_GETFL));

	m_sServer.sin_family = AF_INET;
	m_sServer.sin_addr.s_addr = INADDR_ANY;
	m_sServer.sin_port = htons(TCPSERVER_PORT);

	if (TEMP_FAILURE_RETRY(::bind(m_iSocketFD, (struct sockaddr *)&m_sServer, sizeof(m_sServer))) == -1) {
		Stop();
		return false;
	}

	if (::listen(m_iSocketFD, SOMAXCONN) == -1)
		return false;

	if (m_pListener == NULL)
		m_pListener = new TCPListener(this);
	m_pListener->start();
	if (m_pListener->running())
		return Server::Start();
	else
		return false;
}

bool TCPServer::IsRunning() const
{
	return m_iSocketFD != -1;
}
/*
bool TCPServer::Read(const fd_set * pFDs, std::list<ReadBuffer> & listRead)
{
	ReadBuffer buffer;
	unsigned int i;

	if (m_mutexRead.tryLock())
	{
		for (i = 0; i < m_vecConnections.size(); i++)
		{
			if (FD_ISSET(m_vecConnections[i], pFDs))
			{
				buffer.iLength = TEMP_FAILURE_RETRY(::recv(m_vecConnections[i], m_pRead, BUFFER_LENGTH, 0));
				if (buffer.iLength > 0) {
//					g_pLogger->LogInfo(QString("Received packet of size %1\n").arg(buffer.iLength), WARNING_LEVEL_DEBUG);
					buffer.sFrom = m_mapConnections[m_vecConnections[i]];
					buffer.pData = (unsigned char *)::memcpy(::malloc(buffer.iLength), m_pRead, buffer.iLength);
					listRead.push_back(buffer);
				}
			}
		}
		m_mutexRead.unlock();
	}
	return !listRead.empty();
}
*/

bool TCPServer::Read(const fd_set * pFDs, std::list<ReadBuffer> & listRead)
{
	ReadBuffer buffer;
	unsigned int i;
	int val;
	unsigned int msgLength;

	//printf("reading..........");
	//fflush(stdout);

	if (m_mutexRead.tryLock())
	{
		//printf("1");
		//fflush(stdout);
		for (i = 0; i < m_vecConnections.size(); i++)
		{
			//printf("2");
			//fflush(stdout);
			if (FD_ISSET(m_vecConnections[i], pFDs))
			{
				//printf("3\n");
				//fflush(stdout);

				buffer.iLength = 0;
				while(buffer.iLength < sizeof(PacketType))
				{
					val = ::recv(m_vecConnections[i], m_pRead+buffer.iLength, sizeof(PacketType)-buffer.iLength, 0);
					if(val < 0)
					{
						printf("!TCP Error\n");
						return false;
					}
					buffer.iLength += val;
				}
				PacketType pType = *((PacketType*)m_pRead);
				switch(pType)
				{
				case ptGeneric:
					//printf("Reading Generic Packet\n");
					//fflush(stdout);
					while(buffer.iLength < PACKET_MINIMUM_LENGTH)
					{
						val = ::recv(m_vecConnections[i], m_pRead+buffer.iLength, PACKET_MINIMUM_LENGTH-buffer.iLength, 0);
						if(val < 0)
						{
							printf("!TCP Error\n");
							return false;
						}
						buffer.iLength += val;
					}
					//printf("read %d bytes, should be %d\n", buffer.iLength, PACKET_MINIMUM_LENGTH);
					//fflush(stdout);
					buffer.sFrom = m_mapConnections[m_vecConnections[i]];
					buffer.pData = (unsigned char *)::memcpy(::malloc(buffer.iLength), m_pRead, buffer.iLength);
					listRead.push_back(buffer);
					break;
				case ptSafety:
					//printf("Reading Safety Packet\n");
					//fflush(stdout);
					while(buffer.iLength < MESSAGE_MINIMUM_LENGTH)
					{
						val = ::recv(m_vecConnections[i], m_pRead+buffer.iLength, MESSAGE_MINIMUM_LENGTH-buffer.iLength, 0);
						if(val < 0)
						{
							printf("!TCP Error\n");
							return false;
						}
						buffer.iLength += val;
					}
					//printf("read %d bytes, should be %d\n", buffer.iLength, MESSAGE_MINIMUM_LENGTH);
					//fflush(stdout);
					msgLength = *((unsigned int*)(((char*)m_pRead+buffer.iLength)-sizeof(unsigned int)));
					//printf("message length = %d\n", msgLength);
					//fflush(stdout);
					while(buffer.iLength < msgLength+MESSAGE_MINIMUM_LENGTH)
					{
						val = ::recv(m_vecConnections[i], m_pRead+buffer.iLength, msgLength+MESSAGE_MINIMUM_LENGTH-buffer.iLength, 0);
						if(val < 0)
						{
							printf("!TCP Error\n");
							return false;
						}
						buffer.iLength += val;
					}
					//printf("read %d bytes, should be %d\n", buffer.iLength, MESSAGE_MINIMUM_LENGTH+msgLength);
					//fflush(stdout);
					//printf("message: %s\n", m_pRead+buffer.iLength-msgLength);
					buffer.sFrom = m_mapConnections[m_vecConnections[i]];
					buffer.pData = (unsigned char *)::memcpy(::malloc(buffer.iLength), m_pRead, buffer.iLength);
					listRead.push_back(buffer);
					//m_pMessageList->addMessage(QString("<SafetyPacket> %1").arg((const char *)m_pRead+buffer.iLength-msgLength));
					break;
				default:
					break;
				
				}
			}
		}
		m_mutexRead.unlock();
	}
	return !listRead.empty();
}

bool TCPServer::Stop()
{
	std::map<int, sockaddr_in>::const_iterator iterConnection;

	//printf("-1-\n");
	//fflush(stdout);

	//the wait thing was screwing it up, so i took it out -MH
//	if (m_pListener != NULL && m_pListener->running() && !m_pListener->wait())
//		m_pListener->terminate();
	if (m_pListener != NULL && m_pListener->running())
		m_pListener->terminate();

	//printf("-2-\n");
	//fflush(stdout);

	if (!Server::Stop())
		return false;

	//printf("-3-\n");
	//fflush(stdout);

	for (iterConnection = m_mapConnections.begin(); iterConnection != m_mapConnections.end(); ++iterConnection)
		TEMP_FAILURE_RETRY(::close(iterConnection->first));
	m_mapConnections.clear();
	m_vecConnections.clear();
	if (m_iSocketFD == -1)
		return true;

	//printf("-4-\n");
	//fflush(stdout);

	if (TEMP_FAILURE_RETRY(::close(m_iSocketFD)) == 0) {
		m_iSocketFD = -1;
		return true;
	} else
		return false;
}


TCPClient::TCPClient()
: Client(), m_iSocketFD(-1)
{
	writeBuffer = (unsigned char *)malloc(65535);
	maxBufferSize = 65535;
	bufferLength = 0;
	lastUpdateTime = time(NULL);
}

TCPClient::~TCPClient()
{
}

bool TCPClient::Start(in_addr_t ipTo)
{
	if (!Stop())
		return false; // can't stop existing connection

	m_iSocketFD = TEMP_FAILURE_RETRY(::socket(AF_INET, SOCK_STREAM, 0));
	if (m_iSocketFD == -1)
		return false;

	m_sClient.sin_family = AF_INET;
	m_sClient.sin_addr.s_addr = INADDR_ANY;
	m_sClient.sin_port = htons(TCPCLIENT_PORT);

	if (TEMP_FAILURE_RETRY(::bind(m_iSocketFD, (struct sockaddr *) &m_sClient, sizeof(m_sClient))) == -1) {
		Stop();
		return false;
	}

	m_sServer.sin_family = AF_INET;
	m_sServer.sin_addr.s_addr = htonl(ipTo);
	m_sServer.sin_port = htons(TCPSERVER_PORT);

	if (TEMP_FAILURE_RETRY(::connect(m_iSocketFD, (struct sockaddr *) &m_sServer, sizeof(m_sServer))) == -1) {
		Stop();
		return false;
	} else
		return true;
}

bool TCPClient::IsRunning() const
{
	return m_iSocketFD != -1;
}

bool TCPClient::Select(struct timeval tWait)
{
	fd_set wfds;

	if (m_iSocketFD == -1)
		return false;

	FD_ZERO(&wfds);
	FD_SET(m_iSocketFD, &wfds); // see if can write or if socket device is busy

	return TEMP_FAILURE_RETRY(::select(FD_SETSIZE, NULL, &wfds, NULL, &tWait)) > 0 && FD_ISSET(m_iSocketFD, &wfds);
}

/*
bool TCPClient::Write(const unsigned char * pBuffer, int iLength)
{
	bool bResult;

	if (m_iSocketFD == -1)
		return false;
	m_mutexWrite.lock();
//	g_pLogger->LogInfo(QString("Sent packet of size %1\n").arg(iLength), WARNING_LEVEL_DEBUG);
	bResult = TEMP_FAILURE_RETRY(::sendto(m_iSocketFD, (const char *)pBuffer, iLength, 0, (struct sockaddr *) &m_sServer, sizeof(m_sServer))) != -1;
	m_mutexWrite.unlock();
	return bResult;
}
*/


bool TCPClient::Write(const unsigned char * pBuffer, int iLength)
{
	bool bResult;

	if (m_iSocketFD == -1)
		return false;
	m_mutexWrite.lock();
	int bytesWritten = 0;
	int val = 0;
	while(bytesWritten < iLength)
	{
		val = ::sendto(m_iSocketFD, (const char *)pBuffer+bytesWritten, iLength-bytesWritten, 0, (struct sockaddr *) &m_sServer, sizeof(m_sServer));
		if(val < 0)
		{
			printf("*** TCP Error\n");
			return false;
		}
		bytesWritten += val;
	}
	m_mutexWrite.unlock();
	return true;
}

/*
bool TCPClient::Write(const unsigned char * pBuffer, int iLength)
{
	bool bResult;

	if (m_iSocketFD == -1)
		return false;
	m_mutexWrite.lock();
	int bytesWritten = 0;
	int val = 0;
	if(time(NULL) >= lastUpdateTime+1)
	{
		dbgprint("*");
		//send the buffer
		while(bytesWritten < bufferLength)
		{
			val = ::sendto(m_iSocketFD, (const char *)writeBuffer+bytesWritten, bufferLength-bytesWritten, 0, (struct sockaddr *) &m_sServer, sizeof(m_sServer));
			if(val < 0)
			{
				printf("*** TCP Error\n");
				return false;
			}
			bytesWritten += val;
		}

		lastUpdateTime = time(NULL);
		bufferLength = 0;
	}
	else
	{
		dbgprint("-");
		//write to the buffer
		if(bufferLength + iLength >= maxBufferSize)
		{
			dbgprint("Resized\n");
			writeBuffer = (unsigned char *)realloc(writeBuffer, maxBufferSize*2);
			maxBufferSize *= 2;
		}
		memcpy(writeBuffer+bufferLength, pBuffer, iLength);
		bufferLength += iLength;
	}

	m_mutexWrite.unlock();
	return true;
}
*/

bool TCPClient::Stop()
{
	if (m_iSocketFD == -1)
		return true;

	if (TEMP_FAILURE_RETRY(::close(m_iSocketFD)) == 0) {
		m_iSocketFD = -1;
		return true;
	} else
		return false;
}

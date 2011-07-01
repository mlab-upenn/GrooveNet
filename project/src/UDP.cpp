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

#include "UDP.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define UDPSERVER_PORT 40001
#define UDPCLIENT_PORT 0

UDPServer::UDPServer()
: Server(), m_iSocketFD(-1)
{
}

UDPServer::~UDPServer()
{
}

bool UDPServer::Start()
{
	if (!Stop())
		return false; // can't stop previous connection

	m_iSocketFD = TEMP_FAILURE_RETRY(::socket(AF_INET, SOCK_DGRAM, 0));
	if (m_iSocketFD == -1)
		return false;

	m_vecConnections.push_back(m_iSocketFD);
	::fcntl(m_iSocketFD, F_SETFL, O_NONBLOCK | fcntl(m_iSocketFD, F_GETFL));

	m_sServer.sin_family = AF_INET;
	m_sServer.sin_addr.s_addr = INADDR_ANY;
	m_sServer.sin_port = htons(UDPSERVER_PORT);

	if (TEMP_FAILURE_RETRY(::bind(m_iSocketFD, (struct sockaddr *)&m_sServer, sizeof(m_sServer))) == -1) {
		Stop();
		return false;
	} else
		return Server::Start();
}

bool UDPServer::IsRunning() const
{
	return m_iSocketFD != -1;
}

bool UDPServer::Stop()
{
	if (!Server::Stop())
		return false;

	m_vecConnections.clear();
	if (m_iSocketFD == -1)
		return true;

	if (TEMP_FAILURE_RETRY(::close(m_iSocketFD)) == 0) {
		m_iSocketFD = -1;
		return true;
	} else
		return false;
}


UDPClient::UDPClient()
: Client(), m_iSocketFD(-1)
{
}

UDPClient::~UDPClient()
{
}

bool UDPClient::Start(in_addr_t ipTo)
{
	if (!Stop())
		return false; // can't stop existing connection

	m_iSocketFD = TEMP_FAILURE_RETRY(::socket(AF_INET, SOCK_DGRAM, 0));
	if (m_iSocketFD == -1)
		return false;

	m_sClient.sin_family = AF_INET;
	m_sClient.sin_addr.s_addr = INADDR_ANY;
	m_sClient.sin_port = htons(UDPCLIENT_PORT);

	if (TEMP_FAILURE_RETRY(::bind(m_iSocketFD, (struct sockaddr *) &m_sClient, sizeof(m_sClient))) == -1) {
		Stop();
		return false;
	}

	m_sServer.sin_family = AF_INET;
	m_sServer.sin_addr.s_addr = htonl(ipTo);
	m_sServer.sin_port = htons(UDPSERVER_PORT);

	if (TEMP_FAILURE_RETRY(::connect(m_iSocketFD, (struct sockaddr *) &m_sServer, sizeof(m_sServer))) == -1) {
		Stop();
		return false;
	} else
		return true;
}

bool UDPClient::IsRunning() const
{
	return m_iSocketFD != -1;
}

bool UDPClient::Select(struct timeval tWait)
{
	fd_set wfds;

	if (m_iSocketFD == -1)
		return false;

	FD_ZERO(&wfds);
	FD_SET(m_iSocketFD, &wfds); // see if can write or if socket device is busy

	return TEMP_FAILURE_RETRY(::select(FD_SETSIZE, NULL, &wfds, NULL, &tWait)) > 0 && FD_ISSET(m_iSocketFD, &wfds);
}

bool UDPClient::Write(const unsigned char * pBuffer, int iLength)
{
	bool bResult;

	if (m_iSocketFD == -1)
		return false;
	m_mutexWrite.lock();
	bResult = TEMP_FAILURE_RETRY(::sendto(m_iSocketFD, (const char *)pBuffer, iLength, 0, (struct sockaddr *) &m_sServer, sizeof(m_sServer))) != -1;
	m_mutexWrite.unlock();
	return bResult;
}

bool UDPClient::Stop()
{
	if (m_iSocketFD == -1)
		return true;

	if (TEMP_FAILURE_RETRY(::close(m_iSocketFD)) == 0) {
		m_iSocketFD = -1;
		return true;
	} else
		return false;
}


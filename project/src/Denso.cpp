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

#include "Denso.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>

#include "WAVEapi.h"

static wrm_configuration_parameters_type g_sWAVEParams = {CHANNEL_174, FULL_TXPOWER, DATARATE_03MBPS_10MHz, 256, 256, SERVICE_MODE_PRIVATE, UNIT_MODE_RSU, ANTENNA_1, BANDWIDTH_10, 0, 0, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

bool InitializeDensoUnit(in_addr_t ipSrc, in_addr_t ipSubnet, const QString & strDeviceName, int iTXPower, int iTXRate)
{
	g_sWAVEParams.datarate = iTXRate + 1;
	if (iTXPower + MIN_TXPOWER <= MAX_TXPOWER)
		g_sWAVEParams.txpower = iTXPower + MIN_TXPOWER;
	else
		g_sWAVEParams.txpower = FULL_TXPOWER;

	// initialize communication
	if (init_WAVE_comm(ipSrc, ipSubnet, strDeviceName) != SUCCESS)
		return false;

	// configure
	set_WRM_configuration(&g_sWAVEParams);

	return get_Connection_status() == CONNECTED;
}

bool CloseDensoUnit()
{
	term_WAVE_comm();
	return true;
}


DensoServer::DensoServer()
: Server(), m_bRunning(false), m_pBuffer((unsigned char *)malloc(MAX_PAYLOAD_SIZE))
{
}

DensoServer::~DensoServer()
{
	free(m_pBuffer);
}

bool DensoServer::Start()
{
	if (!Stop())
		return false; // can't stop previous connection

	m_bRunning = Server::Start();
	return m_bRunning;
}

bool DensoServer::IsRunning() const
{
	return m_bRunning;
}

bool DensoServer::Select(fd_set * pFDs, struct timeval tWait)
{
	return get_Connection_status() == CONNECTED && check_WAVE_rx_pkt() == SUCCESS;
}

bool DensoServer::Read(const fd_set * pFDs, std::list<ReadBuffer> & listRead)
{
	ReadBuffer buffer;
	uint16_t iLen = 0;
	rx_pkt_configuration_parameters_type packetInfo;

	if (get_Connection_status() == CONNECTED && m_mutexRead.tryLock())
	{
		if (receive_WAVE_rx_pkt(&packetInfo, &buffer.sFrom.sin_addr, m_pBuffer, &iLen) == SUCCESS && (buffer.iLength = iLen) > 0) {
			m_iRSSI = packetInfo.rssi + 95;
			buffer.pData = (unsigned char *)::memcpy(::malloc(buffer.iLength), m_pBuffer, buffer.iLength);
			listRead.push_back(buffer);
		}
		m_mutexRead.unlock();
	}
	return !listRead.empty();
}

bool DensoServer::Stop()
{
	if (!Server::Stop())
		return false;

	m_bRunning = false;
	return true;
}


DensoClient::DensoClient()
: Client(), m_ipDest(0), m_bRunning(false)
{
}

DensoClient::~DensoClient()
{
}

bool DensoClient::Start(in_addr_t ipTo)
{
	if (!Stop())
		return false; // can't stop existing connection

	m_ipDest = htonl(ipTo);
	m_bRunning = true;
	return m_bRunning;
}

bool DensoClient::IsRunning() const
{
	return m_bRunning;
}

bool DensoClient::Select(struct timeval tWait)
{
	return true;
}

bool DensoClient::Write(const unsigned char * pBuffer, int iLength)
{
	bool bResult;

	m_mutexWrite.lock();
	bResult = (send_WAVE_tx_pkt(&g_sWAVEParams, m_ipDest, pBuffer, iLength) == SUCCESS);
	m_mutexWrite.unlock();
	return bResult;
}

bool DensoClient::Stop()
{
	m_bRunning = false;
	return true;
}


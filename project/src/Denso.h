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

#ifndef _DENSO_H
#define _DENSO_H

#include "Network.h"

#include <qmutex.h>

bool InitializeDensoUnit(in_addr_t ipSrc, in_addr_t ipSubnet, const QString & strDeviceName, int iTXPower, int iTXRate);
bool CloseDensoUnit();

#define DENSOSERVER_NAME "Denso Server"

class DensoServer : public Server
{
public:
	DensoServer();
	virtual ~DensoServer();

	virtual bool Start();
	virtual bool IsRunning() const;
	inline virtual bool FillFDs(fd_set * pFDs) const
	{
		FD_ZERO(pFDs);
		return true;
	}
	virtual bool Select(fd_set * pFDs, struct timeval tWait = timeval0);
	virtual bool Read(const fd_set * pFDs, std::list<ReadBuffer> & listRead);
	virtual bool Stop();

protected:
	inline virtual char rssi()
	{
		return (char)m_iRSSI;
	}

	bool m_bRunning;
	struct sockaddr_in m_sServer;
	unsigned char * m_pBuffer;
	int m_iRSSI;
};

inline Server * DensoServerCreator()
{
	return new DensoServer();
}


#define DENSOCLIENT_NAME "Denso Client"

class DensoClient : public Client
{
public:
	DensoClient();
	virtual ~DensoClient();

	virtual bool Start(in_addr_t ipTo);
	virtual bool IsRunning() const;
	virtual bool Select(struct timeval tWait = timeval0);
	virtual bool Write(const unsigned char * pBuffer, int iLength);
	virtual bool Stop();

protected:
	in_addr_t m_ipDest;
	bool m_bRunning;
	QMutex m_mutexWrite;
};

inline Client * DensoClientCreator()
{
	return new DensoClient();
}

#endif

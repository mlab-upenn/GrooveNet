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

#ifndef _NETWORK_H
#define _NETWORK_H

#include "Coords.h"
#include "Global.h"

#include <arpa/inet.h>

#include <qthread.h>

#include <vector>
#include <map>

#define NETWORK_TIMEOUT_SECS 5
#define NETWORK_TIMEOUT_USECS 0

#include "Message.h"

typedef struct ReadBufferStruct
{
	unsigned char * pData;
	int iLength;
	struct sockaddr_in sFrom;
} ReadBuffer;

#define BUFFER_LENGTH 65536

class Server : public QThread
{
public:
	Server();
	virtual ~Server();

	virtual bool Start();
	virtual bool IsRunning() const = 0;
	virtual bool FillFDs(fd_set * pFDs) const;
	virtual bool Select(fd_set * pFDs, struct timeval tWait = timeval0);
	virtual bool Read(const fd_set * pFDs, std::list<ReadBuffer> & listRead);
	virtual bool Stop();

	inline std::map<in_addr_t, std::vector<Packet *> > * acquireLock(bool bWait = true)
	{
		if (bWait) {
			m_mutexBuffers.lock();
			return &m_mapPackets;
		} else
			return m_mutexBuffers.tryLock() ? &m_mapPackets : NULL;
	}
	inline void releaseLock()
	{
		m_mutexBuffers.unlock();
	}

protected:
	virtual void run();
	inline virtual char rssi()
	{
		return PACKET_RSSI_UNAVAILABLE;
	}
	inline virtual char snr()
	{
		return PACKET_SNR_UNAVAILABLE;
	}

	bool m_bCancelled;
	std::map<in_addr_t, Buffer> m_mapBuffers;
	std::map<in_addr_t, std::vector<Packet *> > m_mapPackets;
	std::vector<int> m_vecConnections;
	QMutex m_mutexBuffers, m_mutexRead, m_mutexConnections;
	unsigned char m_pRead[BUFFER_LENGTH];
};

typedef Server * (* ServerCreator)();

class Client
{
public:
	inline Client() {}
	inline virtual ~Client() {}

	virtual bool Start(in_addr_t ipTo) = 0;
	virtual bool IsRunning() const = 0;
	virtual bool Select(struct timeval tWait = timeval0) = 0;
	virtual bool Write(const unsigned char * pBuffer, int iLength) = 0;
	virtual bool Stop() = 0;
};

typedef Client * (* ClientCreator)();

class HybridServer : QThread
{
public:
	HybridServer();
	~HybridServer();

	bool Start();
	bool Stop();
	bool isRunning();
	bool addClient(int connfd);
	void addPacket(Packet * packet);

	virtual void run();
private:
	int listenFD;
	bool running;
	QMutex connMutex;
	QMutex packetsMutex;
	std::vector<int> connFDs;
	HybridPacket sendPacket;
	HybridPacket recvPacket;
};

class HybridListener : QThread
{
public:
	HybridListener();
	~HybridListener();

	bool Start();
	bool Stop();
	bool isRunning();

	virtual void run();
private:
	int listenFD;
};

class HybridClient : QThread
{
public:
	HybridClient();
	~HybridClient();

	bool Start();
	bool Stop();
	bool isRunning();
	void addPacket(Packet * packet);

	virtual void run();
private:
	QMutex packetsMutex;
	HybridPacket sendPacket;
	HybridPacket recvPacket;
	in_addr_t serverAddr;
};

in_addr_t GetIPAddress(); // host byte order
in_addr_t GetIPSubnet(); // host byte order
unsigned int GetTXRate();

bool InitNetworking();
bool InitNetwork();
bool IsNetworkRunning();
void CloseNetwork();

bool StartServer(const QString & strServer);
bool StartHybridServer();
bool StartHybridClient();
void AddHybridPacket(Packet * packet);
bool IsServerRunning();
Server * GetServer();
void StopServer();

bool OpenConnection(in_addr_t ipReal, const QString & strClient);
Client * GetClient(in_addr_t ipReal);
std::map<in_addr_t, Client *> * GetConnections();
void ReleaseConnections();
void SendPacketToAll(const Packet * packet);
void CloseConnection(in_addr_t ipReal);
void CloseAllConnections();

extern std::map<QString, ServerCreator> g_mapServerCreators;
extern std::map<QString, ClientCreator> g_mapClientCreators;

#endif

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

#include "Network.h"

#include "Global.h"
#include "Settings.h"
#include "Logger.h"
#include "StringHelp.h"
#include "UDP.h"
#include "TCP.h"
#include "Denso.h"
#include "Simulator.h"
#include "MainWindow.h"
#include "CarRegistry.h"
#include "InfrastructureNodeRegistry.h"

#include <qstring.h>
#include <qmutex.h>
#include <signal.h>
#include <stdlib.h>
#include <map>

static in_addr_t g_ipAddress = 0;
static in_addr_t g_ipSubnet = 0;
static QString g_strDeviceName = "";
static QString g_strDeviceType = "";
static int g_iTXPower = 0;
static int g_iTXRate = 0;
static int lastUpdateTime = 0;

std::map<QString, ServerCreator> g_mapServerCreators;
std::map<QString, ClientCreator> g_mapClientCreators;
std::map<in_addr_t, Packet *> mapSendPackets;

static bool g_bNetworkEnabled = false;
static void (* g_pOldSigPipeHandler) (int) = SIG_DFL;
Server * g_pServer = NULL;
HybridServer * g_pHybridServer = NULL;
HybridListener * g_pHybridListener = NULL;
HybridClient * g_pHybridClient = NULL;
static std::map<in_addr_t, Client *> g_mapClients;
static QMutex g_mutexClients(true);

in_addr_t GetIPAddress()
{
	return g_ipAddress;
}

in_addr_t GetIPSubnet()
{
	return g_ipSubnet;
}

unsigned int GetTXRate()
{
	// TODO: vary depending on device type
	// right now, assume Denso kit (DSRC)
	switch (g_iTXRate)
	{
	case 0:
		return 375000;
	case 1:
		return 562500;
	case 2:
		return 750000;
	case 3:
		return 1500000;
	case 4:
		return 2250000;
	case 5:
		return 3000000;
	case 6:
		return 3375000;
	default:
		return 0;
	}
}

bool InitNetworking()
{
	g_mapServerCreators[UDPSERVER_NAME] = UDPServerCreator;
	g_mapServerCreators[TCPSERVER_NAME] = TCPServerCreator;
	//g_mapServerCreators[DENSOSERVER_NAME] = DensoServerCreator;
	g_mapClientCreators[UDPCLIENT_NAME] = UDPClientCreator;
	g_mapClientCreators[TCPCLIENT_NAME] = TCPClientCreator;
	//g_mapClientCreators[DENSOCLIENT_NAME] = DensoClientCreator;
	return true;
}

bool InitNetwork()
{
	QString strNetInitFile, strNetInitArgs, strValue;
	bool bContinue = true;
	int iErrorCode;

	// make sure network isn't running
	CloseNetwork();

	if (g_bNetworkEnabled)
		return false;

	g_pLogger->LogInfo("Opening Network Interface...");

	g_strDeviceName = g_pSettings->GetParam(PARAMKEY_NETWORK_DEVNAME, g_pSettings->m_sSettings[SETTINGS_NETWORK_DEVICENAME_NUM].GetValue().strValue, true);
	g_strDeviceType = g_pSettings->m_sSettings[SETTINGS_NETWORK_DEVICETYPE_NUM].GetValue().strValue;
	if (g_pSettings->GetParam(PARAMKEY_NETWORK_IP, "", false).isEmpty() || !StringToIPAddress(g_pSettings->GetParam(PARAMKEY_NETWORK_IP, "", false), g_ipAddress))
	{
		if (!StringToIPAddress(g_pSettings->m_sSettings[SETTINGS_NETWORK_IPADDRESS_NUM].GetValue().strValue, g_ipAddress))
			g_ipAddress = 0;
	}
	if (g_pSettings->GetParam(PARAMKEY_NETWORK_SUBNET, "", false).isEmpty() || !StringToIPAddress(g_pSettings->GetParam(PARAMKEY_NETWORK_SUBNET, "", false), g_ipSubnet))
		StringToIPAddress(g_pSettings->m_sSettings[SETTINGS_NETWORK_IPSUBNET_NUM].GetValue().strValue, g_ipSubnet);
	g_iTXPower = g_pSettings->m_sSettings[SETTINGS_NETWORK_TXPOWER_NUM].GetValue().iValue;
	g_iTXRate = g_pSettings->m_sSettings[SETTINGS_NETWORK_TXRATE_NUM].GetValue().iValue;

	strNetInitFile = g_pSettings->m_sSettings[SETTINGS_NETWORK_INITFILE_NUM].GetValue().strValue;
	if (!strNetInitFile.isEmpty())
	{
		strNetInitArgs = g_pSettings->m_sSettings[SETTINGS_NETWORK_INITARGS_NUM].GetValue().strValue;

		strNetInitArgs.replace("%dn", g_strDeviceName);
		strNetInitArgs.replace("%dt", g_strDeviceType);
		strNetInitArgs.replace("%ip", IPAddressToString(g_ipAddress));
		strNetInitArgs.replace("%sub", IPAddressToString(g_ipSubnet));
		strNetInitArgs.replace("%txp", QString::number(g_iTXPower));
		strNetInitArgs.replace("%txr", QString::number(g_iTXRate));

		if (!strNetInitArgs.isEmpty())
			strNetInitFile += " " + strNetInitArgs;

		while (bContinue)
		{
			iErrorCode = system(strNetInitFile);
			if (iErrorCode == 0) {
				g_bNetworkEnabled = true;
				bContinue = false;
			} else
				bContinue = g_pLogger->PromptLogError(PACKAGE_TITLE, QString("Could not open network interface (code %1)").arg(iErrorCode), false);
		}
	}
	else
		g_bNetworkEnabled = true;

	if (g_bNetworkEnabled)
	{
		/*if (g_strDeviceType.compare("denso") == 0)
			g_bNetworkEnabled = InitializeDensoUnit(g_ipAddress, g_ipSubnet, g_strDeviceName, g_iTXPower, g_iTXRate);
		*/
	}

	if (g_bNetworkEnabled)
		g_pLogger->LogInfo("Successful\n");
	else {
		g_pLogger->LogInfo("Failed\n");
		return false;
	}

	if (g_pMainWindow != NULL)
	{
		g_pMainWindow->m_pNetInit->setMenuText("Re&initialize");
		g_pMainWindow->m_pNetInit->setToolTip("Reinitialize network devices and connections");
		g_pMainWindow->m_pNetClose->setEnabled(true);
		g_pMainWindow->m_pNetMenu->setItemEnabled(g_pMainWindow->m_iNetServerMenuID, true);
		g_pMainWindow->m_pNetworkManager->SetNetworkInitialized(true);
	}

	g_pOldSigPipeHandler = signal(SIGPIPE, SIG_IGN);

	// start server
	if (g_pSettings->m_sSettings[SETTINGS_NETWORK_AUTOSERVER_NUM].GetValue().bValue)
	{
		if (g_strDeviceType.compare("denso") == 0)
			return StartServer(UDPSERVER_NAME);
		else
			return StartServer(UDPSERVER_NAME);
	}
	else
		return true;
}

bool IsNetworkRunning()
{
	return g_bNetworkEnabled;
}

void CloseNetwork()
{
	if (IsNetworkRunning())
	{
		QString strNetCloseFile, strNetCloseArgs;
		bool bContinue = true;
		int iErrorCode;

		// stop clients, server
		CloseAllConnections();
		if (g_pMainWindow != NULL)
			g_pMainWindow->m_pNetworkManager->UpdateClientTable();
		if (!g_mapClients.empty())
			return;

		StopServer();
		if (g_pServer != NULL)
			return;

		g_pLogger->LogInfo("Closing Network Interface...");
		signal(SIGPIPE, g_pOldSigPipeHandler);

		/*if (g_strDeviceType.compare("denso") == 0)
			g_bNetworkEnabled = !CloseDensoUnit();
		*/
		strNetCloseFile = g_pSettings->m_sSettings[SETTINGS_NETWORK_CLOSEFILE_NUM].GetValue().strValue;
		if (!g_bNetworkEnabled && !strNetCloseFile.isEmpty())
		{
			strNetCloseArgs = g_pSettings->m_sSettings[SETTINGS_NETWORK_CLOSEARGS_NUM].GetValue().strValue;

			strNetCloseArgs.replace("%dn", g_strDeviceName);
			strNetCloseArgs.replace("%dt", g_strDeviceType);
			strNetCloseArgs.replace("%ip", IPAddressToString(g_ipAddress));
			strNetCloseArgs.replace("%sub", IPAddressToString(g_ipSubnet));
			strNetCloseArgs.replace("%txp", QString::number(g_iTXPower));
			strNetCloseArgs.replace("%txr", QString::number(g_iTXRate));

			if (!strNetCloseArgs.isEmpty())
				strNetCloseFile += " " + strNetCloseArgs;

			while (bContinue)
			{
				iErrorCode = system(strNetCloseFile);
				if (iErrorCode == 0) {
					g_bNetworkEnabled = false;
					bContinue = false;
				} else
					bContinue = g_pLogger->PromptLogError(PACKAGE_TITLE, QString("Could not close network interface (code %1)").arg(iErrorCode), false);
			}
		}
		else
			g_bNetworkEnabled = false;

		if (g_bNetworkEnabled) {
			g_pLogger->LogInfo("Failed\n");
			return;
		} else
			g_pLogger->LogInfo("Successful\n");

		if (g_pMainWindow != NULL)
		{
			g_pMainWindow->m_pNetInit->setMenuText("&Initialize");
			g_pMainWindow->m_pNetInit->setToolTip("Initialize network devices and connections");
			g_pMainWindow->m_pNetClose->setEnabled(false);
			g_pMainWindow->m_pNetMenu->setItemEnabled(g_pMainWindow->m_iNetServerMenuID, false);
			g_pMainWindow->m_pNetworkManager->SetNetworkInitialized(false);
		}
	}
}

bool StartHybridClient()
{
	g_pLogger->LogInfo(QString("Starting Hybrid Client..."));

	if(g_pHybridClient!= NULL)
	{
		g_pHybridClient->Stop();
		delete g_pHybridClient;
		g_pHybridClient = NULL;
	}
	g_pHybridClient = new HybridClient;
	
	if(g_pHybridClient != NULL && g_pHybridClient->Start())
	{
		g_pLogger->LogInfo("Successful\n");
		return true;
	} else {
		g_pLogger->LogInfo("Failed\n");
		if (g_pHybridClient != NULL) {
			delete g_pHybridClient;
			g_pHybridClient = NULL;
		}
		return false;
	}
}

bool StartHybridServer()
{
	g_pLogger->LogInfo(QString("Starting Hybrid Server..."));

	if(g_pHybridServer != NULL)
	{
		g_pHybridServer->Stop();
		delete g_pHybridServer;
		g_pHybridServer = NULL;
	}
	g_pHybridServer = new HybridServer;
	g_pHybridListener = new HybridListener;
	if(g_pHybridServer != NULL && g_pHybridServer->Start())
	{
		if(g_pHybridListener != NULL && g_pHybridListener->Start())
		{
			g_pLogger->LogInfo("Successful\n");
			return true;
		} else {
			g_pLogger->LogInfo("Failed\n");
			if (g_pHybridServer != NULL) {
				delete g_pHybridServer;
				g_pHybridServer = NULL;
			}
			if (g_pHybridListener != NULL) {
				delete g_pHybridListener;
				g_pHybridListener = NULL;
			}
			return false;
		}
	} else {
		g_pLogger->LogInfo("Failed\n");
		if (g_pHybridServer != NULL) {
			delete g_pHybridServer;
			g_pHybridServer = NULL;
		}
		return false;
	}
}

bool StartServer(const QString & strServer)
{
	std::map<QString, ServerCreator>::iterator iterCreator;

	g_pLogger->LogInfo(QString("Starting %1...").arg(strServer));

	iterCreator = g_mapServerCreators.find(strServer);

	if (g_pServer != NULL) {
		g_pServer->Stop();
		delete g_pServer;
		g_pServer = NULL;
	}
	if (iterCreator != g_mapServerCreators.end())
		g_pServer = (*iterCreator->second)();
	if (g_pServer != NULL && g_pServer->Start())
	{
		if (g_pMainWindow != NULL)
		{
			g_pMainWindow->m_pNetMenu->removeItem(g_pMainWindow->m_iNetServerMenuID);
			g_pMainWindow->m_iNetServerMenuID = -1;
			g_pMainWindow->m_pNetServer->addTo(g_pMainWindow->m_pNetMenu);
			g_pMainWindow->m_pNetworkManager->SetServerRunning(true);
		}
		g_pLogger->LogInfo("Successful\n");
		return true;
	} else {
		g_pLogger->LogInfo("Failed\n");
		if (g_pServer != NULL) {
			delete g_pServer;
			g_pServer = NULL;
		}
		return false;
	}
}

bool IsServerRunning()
{
	return g_pServer != NULL && g_pServer->IsRunning();
}

bool IsHybridServerRunning()
{
	return g_pHybridServer != NULL && g_pHybridServer->isRunning();
}

Server * GetServer()
{
	return g_pServer;
}

void StopServer()
{
	if (g_pServer != NULL) {
		g_pLogger->LogInfo("Stopping Server...");

		g_pServer->Stop();
		delete g_pServer;
		g_pServer = NULL;

		if (g_pMainWindow != NULL)
		{
			g_pMainWindow->m_pNetServer->removeFrom(g_pMainWindow->m_pNetMenu);
			g_pMainWindow->m_iNetServerMenuID = g_pMainWindow->m_pNetMenu->insertItem("&Start Server", g_pMainWindow->m_pNetServerMenu);
			g_pMainWindow->m_pNetworkManager->SetServerRunning(false);
		}
		g_pLogger->LogInfo("Successful\n");
	}
}

bool OpenConnection(in_addr_t ipReal, const QString & strClient)
{
	std::map<in_addr_t, Client *>::iterator iterClient, iterOldClient;
	std::map<QString, ClientCreator>::iterator iterCreator;
	bool bSuccess = false;
	Client * pClient;

	g_mutexClients.lock();

	g_pLogger->LogInfo(QString("Starting %1 to host %2...").arg(strClient).arg(IPAddressToString(ipReal)));

	iterCreator = g_mapClientCreators.find(strClient);
	iterClient = g_mapClients.find(ipReal);

	if (iterClient != g_mapClients.end())
	{
		if (iterClient->second != NULL)
		{
			iterClient->second->Stop();
			delete iterClient->second;
		}
		g_mapClients.erase(iterClient);
	}
	if (iterCreator != g_mapClientCreators.end())
		pClient = (*iterCreator->second)();
	else
		pClient = NULL;

	if (pClient != NULL && pClient->Start(ipReal))
		bSuccess = g_mapClients.insert(std::pair<in_addr_t, Client *>(ipReal, pClient)).second;
	else if (pClient != NULL)
		delete pClient;

	if (bSuccess)
		g_pLogger->LogInfo("Successful\n");
	else
		g_pLogger->LogInfo("Failed\n");

	g_mutexClients.unlock();

	return bSuccess;
}

Client * GetClient(in_addr_t ipReal)
{
	std::map<in_addr_t, Client *>::iterator iterConnection;
	Client * pClient = NULL;

	g_mutexClients.lock();
	iterConnection = g_mapClients.find(ipReal);
	if (iterConnection != g_mapClients.end())
		pClient = iterConnection->second;
	g_mutexClients.unlock();
	return pClient;
}

std::map<in_addr_t, Client *> * GetConnections()
{
	g_mutexClients.lock();
	return &g_mapClients;
}

void ReleaseConnections()
{
	g_mutexClients.unlock();
}

/*
void SendPacketToAll(const Packet * packet)
{
	std::map<in_addr_t, Client *>::iterator iterConnection;
	unsigned char * pBuffer;
	int iLength = 0;

	g_mutexClients.lock();
	pBuffer = packet->ToBytes(iLength);
	if (pBuffer == NULL)
	{
		g_mutexClients.unlock();
		return;
	}

	for (iterConnection = g_mapClients.begin(); iterConnection != g_mapClients.end(); ++iterConnection)
	{
		if (iterConnection->first == packet->m_ID.srcID.ipCar)
			continue;

		// change header for each client
		*((in_addr_t *)(pBuffer + PACKET_IPRX_OFFSET)) = iterConnection->first;

		if (iterConnection->second != NULL && iterConnection->second->Select())
			iterConnection->second->Write(pBuffer, iLength);
	}
	g_mutexClients.unlock();

	free(pBuffer);
}
*/

void SendPacketToAll(const Packet * packet)
{
	std::map<in_addr_t, Client *>::iterator iterConnection;
	std::map<in_addr_t, Packet *>::iterator iterMapSendPackets;
	unsigned char * pBuffer;
	int iLength = 0;

	//TODO: ignore calls if not connected to any clients

	if(time(NULL) >= lastUpdateTime+1) //if it's time to send the packets
	{
		g_mutexClients.lock();
		//loop over the map
		for(iterMapSendPackets = mapSendPackets.begin(); iterMapSendPackets != mapSendPackets.end(); iterMapSendPackets++)
		{
			dbgprint("*");
			pBuffer = iterMapSendPackets->second->ToBytes(iLength);
			if (pBuffer == NULL)
			{
				g_mutexClients.unlock();
				return;
			}
		
			for (iterConnection = g_mapClients.begin(); iterConnection != g_mapClients.end(); ++iterConnection)
			{
				if (iterConnection->first == iterMapSendPackets->second->m_ID.srcID.ipCar)
					continue;
		
				// change header for each client
				*((in_addr_t *)(pBuffer + PACKET_IPRX_OFFSET)) = iterConnection->first;
		
				if (iterConnection->second != NULL && iterConnection->second->Select())
					iterConnection->second->Write(pBuffer, iLength);
			}
			free(pBuffer);
			DestroyPacket(iterMapSendPackets->second);
		}
		g_mutexClients.unlock();
		lastUpdateTime = time(NULL);
		mapSendPackets.clear();
	}
	else //not time to send yet
	{
		dbgprint("-");
		//add packets to map. be sure to clone
		mapSendPackets[packet->m_ipTX] = packet->clone();
	}
}



void CloseConnection(in_addr_t ipReal)
{
	std::map<in_addr_t, Client *>::iterator iterConnection;

	g_mutexClients.lock();
	g_pLogger->LogInfo(QString("Stopping client to host %1...").arg(IPAddressToString(ipReal)));
	iterConnection = g_mapClients.find(ipReal);
	if (iterConnection != g_mapClients.end())
	{
		if (iterConnection->second != NULL)
		{
			iterConnection->second->Stop();
			delete iterConnection->second;
			iterConnection->second = NULL;
		}
		g_mapClients.erase(iterConnection);
	}
	g_pLogger->LogInfo("Successful\n");
	g_mutexClients.unlock();
}

void CloseAllConnections()
{
	std::map<in_addr_t, Client *>::iterator iterConnection;

	g_mutexClients.lock();
	if (!g_mapClients.empty())
	{
		g_pLogger->LogInfo("Stopping all clients...");
		for (iterConnection = g_mapClients.begin(); iterConnection != g_mapClients.end(); ++iterConnection)
		{
			if (iterConnection->second != NULL)
			{
				iterConnection->second->Stop();
				delete iterConnection->second;
			}
		}
		g_mapClients.clear();
		g_pLogger->LogInfo("Successful\n");
	}
	g_mutexClients.unlock();
}


Server::Server()
: QThread(), m_bCancelled(false)
{
}

Server::~Server()
{
	std::map<in_addr_t, std::vector<Packet *> >::iterator iterPackets;
	unsigned int i;
	for (iterPackets = m_mapPackets.begin(); iterPackets != m_mapPackets.end(); ++iterPackets)
	{
		for (i = 0; i < iterPackets->second.size(); i++)
			DestroyPacket(iterPackets->second[i]);
	}
}

bool Server::Start()
{
	m_bCancelled = false;
	QThread::start();
	return true;
}

#define MAX_DEADLOCK 300

bool Server::Stop()
{
	m_bCancelled = true;
	while (!QThread::wait(MAX_DEADLOCK))
		QThread::terminate();

	m_mutexBuffers.lock();
	m_mapBuffers.clear();
	m_mutexBuffers.unlock();

	return true;
}

bool Server::FillFDs(fd_set * pFDs) const
{
	unsigned int i;

	FD_ZERO(pFDs);
	for (i = 0; i < m_vecConnections.size(); i++)
		FD_SET(m_vecConnections[i], pFDs);
	return !m_vecConnections.empty();
}

bool Server::Select(fd_set * pFDs, struct timeval tWait)
{
	return TEMP_FAILURE_RETRY(::select(FD_SETSIZE, pFDs, NULL, NULL, &tWait)) > 0;
}

bool Server::Read(const fd_set * pFDs, std::list<ReadBuffer> & listRead)
{
	ReadBuffer buffer;
	int iLen;
	unsigned int i;

	if (m_mutexRead.tryLock())
	{
		for (i = 0; i < m_vecConnections.size(); i++)
		{
			if (FD_ISSET(m_vecConnections[i], pFDs))
			{
				iLen = sizeof(buffer.sFrom);
				buffer.iLength = TEMP_FAILURE_RETRY(::recvfrom(m_vecConnections[i], m_pRead, BUFFER_LENGTH, 0, (struct sockaddr *) &buffer.sFrom, (socklen_t *) &iLen));
				if (buffer.iLength > 0) {
					buffer.pData = (unsigned char *)::memcpy(::malloc(buffer.iLength), m_pRead, buffer.iLength);
					listRead.push_back(buffer);
				}
			}
		}
		m_mutexRead.unlock();
	}
	return !listRead.empty();
}


#define SERVER_WAIT_SECS 0
#define SERVER_WAIT_USECS 100000

void Server::run()
{
	Packet * pPacket = NULL, * pNewPacket = NULL;
	unsigned char * pNewMessage;
	int iNewMessageLength = 0;
	in_addr_t ipTX;
	std::map<in_addr_t, in_addr_t> mapNewCars;
	struct timeval tWait = MakeTime(SERVER_WAIT_SECS, SERVER_WAIT_USECS);
	std::map<in_addr_t, std::vector<Packet *> >::iterator iterPackets;
	std::map<in_addr_t, Buffer>::iterator iterBuffer;
	std::map<in_addr_t, CarModel *> * pCarRegistry;
	std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry;
	std::map<in_addr_t, CarModel *>::iterator iterCarReceiver;
	std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterNode;
	struct timeval tRX;
	char iRSSI, iSNR;
	fd_set rfds;
	std::list<ReadBuffer> listRead;
	std::list<ReadBuffer>::iterator iterRead;

	while (!m_bCancelled)
	{
		// get any waiting messages
		m_mutexConnections.lock();
		if (FillFDs(&rfds) && Select(&rfds, tWait))
		{
			Read(&rfds, listRead);
			for (iterRead = listRead.begin(); iterRead != listRead.end(); ++iterRead)
			{
				ipTX = ntohl(iterRead->sFrom.sin_addr.s_addr);
				// if new message is not null, add it to the current buffer
				if (iterRead->pData != NULL)
					m_mapBuffers[ipTX] += Buffer(iterRead->pData, iterRead->iLength);
			}
			listRead.clear();
		}
		m_mutexConnections.unlock();

		// process non-empty buffers into messages
		for (iterBuffer = m_mapBuffers.begin(); iterBuffer != m_mapBuffers.end(); ++iterBuffer)
		{
			if (iterBuffer->second.m_iLength > 0)
			{
				//printf("processing buffer..........\n");
				//fflush(stdout);
				unsigned char * pNew = NULL;

				pCarRegistry = g_pCarRegistry->acquireLock();
				pNodeRegistry = g_pInfrastructureNodeRegistry->acquireLock();
				m_mutexBuffers.lock();
				pNewMessage = iterBuffer->second.m_pData;
				iNewMessageLength = (signed)iterBuffer->second.m_iLength;
				tRX = GetCurrentTime();
				iRSSI = rssi();
				iSNR = snr();
				while (iNewMessageLength >= (signed)PACKET_MINIMUM_LENGTH && (pPacket = CreatePacket(*(const PacketType *)pNewMessage)) != NULL) {
					//printf("got packet...\n");
					if (!pPacket->FromBytes(pNewMessage, iNewMessageLength))
					{
						//printf("failed...\n");
						DestroyPacket(pPacket);
						iNewMessageLength = 0;
						break;
					}

					//if(((SafetyPacket*)pPacket)->m_ePacketType == ptSafety)
					//	printf("data: %s\n", ((SafetyPacket*)pPacket)->m_pData);
					//if(((Packet*)pPacket)->m_ePacketType == ptGeneric)
					//	printf("long: %ld\n", ((Packet*)pPacket)->m_ptTXPosition.m_iLong);
					// add it to the proper message buffer
					pPacket->m_tRX = tRX;
					pPacket->m_iRSSI = iRSSI;
					pPacket->m_iSNR = iSNR;

					pNewPacket = pPacket->clone();
					for (iterCarReceiver = pCarRegistry->begin(); iterCarReceiver != pCarRegistry->end(); ++iterCarReceiver)
					{
						if (iterCarReceiver->second != NULL && iterCarReceiver->second->GetOwnerIPAddress() == CARMODEL_IPOWNER_LOCAL && iterCarReceiver->first != pPacket->m_ipTX) {
							pNewPacket->m_ipRX = iterCarReceiver->first;
							iterCarReceiver->second->ReceivePacket(pNewPacket);
						}
					}

					for (iterNode = pNodeRegistry->begin(); iterNode != pNodeRegistry->end(); ++iterNode)
					{
						if (iterNode->second != NULL && iterNode->first != pPacket->m_ipTX)
						{
							pNewPacket->m_ipRX = iterNode->first;
							iterNode->second->ReceivePacket(pNewPacket);
						}
					}
					DestroyPacket(pNewPacket);

					iterCarReceiver = pCarRegistry->find(pPacket->m_ID.srcID.ipCar);
					iterNode = pNodeRegistry->find(pPacket->m_ID.srcID.ipCar);

					if ((iterCarReceiver == pCarRegistry->end() || iterCarReceiver->second == NULL || iterCarReceiver->second->GetOwnerIPAddress() != CARMODEL_IPOWNER_LOCAL) && (iterNode == pNodeRegistry->end() || iterNode->second == NULL))
					{
						//printf("processing..................................\n");
						mapNewCars[pPacket->m_ID.srcID.ipCar] = iterBuffer->first;
						iterPackets = m_mapPackets.find(pPacket->m_ID.srcID.ipCar);
						if (iterPackets == m_mapPackets.end())
							iterPackets = m_mapPackets.insert(std::pair<in_addr_t, std::vector<Packet *> >(pPacket->m_ID.srcID.ipCar, std::vector<Packet *>())).first;
						iterPackets->second.push_back(pPacket);
						push_heap(iterPackets->second.begin(), iterPackets->second.end(), ComparePacketPtrs);
					}
					else
						DestroyPacket(pPacket);
				}
				m_mutexBuffers.unlock();
				g_pInfrastructureNodeRegistry->releaseLock();
				g_pCarRegistry->releaseLock();

				if (iNewMessageLength > 0)
					pNew = (unsigned char *)memcpy(malloc(iNewMessageLength), pNewMessage, iNewMessageLength);
				iterBuffer->second = Buffer(pNew, iNewMessageLength);
			}
		}
		//add new network car to list
		if (!mapNewCars.empty()) {
			g_pSimulator->m_ModelMgr.AddNetworkCars(mapNewCars);
			mapNewCars.clear();
		}
	//sleep(1);
	}
}

HybridServer::HybridServer():running(false)
{
}

HybridServer::~HybridServer()
{
}

void HybridServer::addPacket(Packet * packet)
{
	packetsMutex.lock();
	if(connFDs.size() > 0)
		sendPacket.addPacket(packet);
	packetsMutex.unlock();
}

bool HybridServer::Start()
{
	running = true;
	QThread::start();
	return true;
}

bool HybridServer::Stop()
{
	//TODO: implement this
	return false;
}

bool HybridServer::addClient(int connfd)
{
	connMutex.lock();
	connFDs.push_back(connfd);
	connMutex.unlock();
	return true;
}

void HybridServer::run()
{
	while(running)
	{
		connMutex.lock();
		std::vector<int>::iterator iter;
		int packetSize;
		int bytesTransferred;
		int val;
		unsigned char * buf = (unsigned char *)malloc(65535);

		for(iter = connFDs.begin(); iter != connFDs.end(); iter++)
		{
			dbgprint("Hybrid Server Loop\n");
			bytesTransferred = 0;
			while(bytesTransferred < 4)
			{
				val = ::recv(*iter, buf, 4, 0);
				if(val < 0)
				{
					g_pLogger->LogInfo(QString("Hybrid Network Error"));
					//TODO: handle recv errors correctly
					return;
				}
				bytesTransferred += val;
			}
			memcpy(&packetSize, buf, 4);
			dbgprint("Got Hybrid Packet... size = %d\n", packetSize);
			while(bytesTransferred < packetSize)
			{
				val = ::recv(*iter, buf+bytesTransferred, packetSize-bytesTransferred, 0);
				if(val < 0)
				{
					g_pLogger->LogInfo(QString("Hybrid Network Error"));
					//TODO: handle recv errors correctly
					return;
				}
				bytesTransferred += val;
			}
			recvPacket.fromBytes(buf);

			//--------------------------------------------------
			std::map<in_addr_t, in_addr_t> mapNewCars;
			std::vector<Packet*>::iterator iterPackets;
			std::vector<SafetyPacket*>::iterator iterSafetyPackets;
			Packet * pNewPacket;
			std::map<in_addr_t, CarModel *> * pCarRegistry;
			std::map<in_addr_t, CarModel *>::iterator iterCarReceiver;
			pCarRegistry = g_pCarRegistry->acquireLock();
			std::map<in_addr_t, std::vector<Packet *> > m_mapPackets;

			for(iterPackets = recvPacket.genericPackets.begin(); iterPackets != recvPacket.genericPackets.end(); iterPackets++)
			{
				pNewPacket = (*iterPackets)->clone();

				for (iterCarReceiver = pCarRegistry->begin(); iterCarReceiver != pCarRegistry->end(); ++iterCarReceiver)
				{
					if (iterCarReceiver->second != NULL && iterCarReceiver->second->GetOwnerIPAddress() == CARMODEL_IPOWNER_LOCAL && iterCarReceiver->first != (*iterPackets)->m_ipTX) {
						pNewPacket->m_ipRX = iterCarReceiver->first;
						iterCarReceiver->second->ReceivePacket(pNewPacket);
					}
				}

				mapNewCars[(*iterPackets)->m_ID.srcID.ipCar] = (*iterPackets)->m_ID.srcID.ipCar;
			}

			//ignore safety packets for now
			/*for(iterSafetyPackets = recvPacket.safetyPackets.begin(); iterSafetyPackets != recvPacket.safetyPackets.end(); iterSafetyPackets++)
			{
				pNewPacket = (Packet*)((*iterSafetyPackets)->clone());

				for (iterCarReceiver = pCarRegistry->begin(); iterCarReceiver != pCarRegistry->end(); ++iterCarReceiver)
				{
					if (iterCarReceiver->second != NULL && iterCarReceiver->second->GetOwnerIPAddress() == CARMODEL_IPOWNER_LOCAL && iterCarReceiver->first != (*iterPackets)->m_ipTX) {
						pNewPacket->m_ipRX = iterCarReceiver->first;
						iterCarReceiver->second->ReceivePacket(pNewPacket);
					}
				}
			}*/
			g_pCarRegistry->releaseLock();

			if (!mapNewCars.empty()) {
				g_pSimulator->m_ModelMgr.AddNetworkCars(mapNewCars);
				mapNewCars.clear();
			}
			//--------------------------------------------------

			recvPacket.clear();
			free(buf);

			packetsMutex.lock();
			dbgprint("--copied--\n");
			HybridPacket * mySendPacket = sendPacket.clone();
			sendPacket.clear();
			packetsMutex.unlock();

			buf = mySendPacket->toBytes();
			bytesTransferred = 0;
			memcpy(&packetSize, buf, 4);
			while(bytesTransferred < packetSize)
			{
				val = ::send(*iter, buf+bytesTransferred, packetSize-bytesTransferred, 0);
				if(val < 0)
				{
					g_pLogger->LogInfo(QString("Hybrid Network Error"));
					//TODO: handle send errors correctly
					return;
				}
				bytesTransferred += val;
				dbgprint("sent %d\n", val);
			}
			mySendPacket->clear();
			free(buf);
			delete mySendPacket;
		}
		connMutex.unlock();
		sleep(1);
	}
}

bool HybridServer::isRunning()
{
	return running;
}


HybridListener::HybridListener()
{
}

HybridListener::~HybridListener()
{
}

bool HybridListener::Start()
{
	QThread::start();
	return true;
}

void HybridListener::run()
{
	int listenfd, optval = 1;
	struct sockaddr_in serveraddr;

	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		g_pLogger->LogInfo(QString("Hybrid Network Error: Couldn't Create Socket"));
		return;
	}

	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
			(const void *)&optval, sizeof(int)) < 0)
	{
		g_pLogger->LogInfo(QString("Hybrid Network Error: Couldn't Set Socket Options"));
		return;
	}

	bzero((char*)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)40005);
	if(bind(listenfd, (sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
	{
		g_pLogger->LogInfo(QString("Hybrid Network Error: Couldn't Bind To Socket"));
		return;
	}

	if(listen(listenfd, 1024) < 0)
	{
		g_pLogger->LogInfo(QString("Hybrid Network Error: Couldn't Listen On Socket"));
		return;
	}

	int connfd, clientlen;
	struct sockaddr_in clientaddr;
	clientlen = sizeof(clientaddr);
	while(true)
	{
		if(g_pHybridServer != NULL && g_pHybridServer->isRunning())
		{
			dbgprint("Waiting to accept hybrid connection\n");
			connfd = accept(listenfd, (sockaddr*)&clientaddr, (socklen_t*)&clientlen);
			g_pLogger->LogInfo(QString("Accepted Hybrid Connection\n"));
			g_pHybridServer->addClient(connfd);
		}
	}
}


HybridClient::HybridClient()
{
}

HybridClient::~HybridClient()
{
}

void HybridClient::addPacket(Packet * packet)
{
	packetsMutex.lock();
	sendPacket.addPacket(packet);
	packetsMutex.unlock();
}

bool HybridClient::Start()
{
	QThread::start();
	return true;
}

bool HybridClient::Stop()
{
	//TODO: implement this
	return false;
}

void HybridClient::run()
{
	int clientfd;
	struct sockaddr_in serveraddr;

	if((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		g_pLogger->LogInfo(QString("Hybrid Network Error: Couldn't Create Socket"));
		return;
	}

	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr =
		htonl(inet_addr(g_pSettings->m_sSettings[SETTINGS_HYBRIDNETWORK_IP_NUM].GetValue().strValue));
	serveraddr.sin_port = htons(40005);

	if(connect(clientfd, (sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
	{
		g_pLogger->LogInfo(QString("Hybrid Network Error: Couldn't Connect"));
		return;
	}

	while(true)
	{
		int packetSize;
		int bytesTransferred;
		int val;
		unsigned char * sendBuf;
		unsigned char * buf = new unsigned char[65535];

		packetsMutex.lock();
		dbgprint("--copied--\n");
		HybridPacket * mySendPacket = sendPacket.clone();
		sendPacket.clear();
		packetsMutex.unlock();

		if(mySendPacket->numPackets() > 0)
		{
			dbgprint("Hybrid Client Loop: ");
			sendBuf = mySendPacket->toBytes();
			bytesTransferred = 0;
			memcpy(&packetSize, sendBuf, 4);
			dbgprint("Sending %d Bytes, size in buf = %d\n", packetSize, int(*sendBuf));
			while(bytesTransferred < packetSize)
			{
				val = ::send(clientfd, sendBuf+bytesTransferred, packetSize-bytesTransferred, 0);
				if(val < 0)
				{
					g_pLogger->LogInfo(QString("Hybrid Network Error"));
					//TODO: handle send errors correctly
					return;
				}
				bytesTransferred += val;
				dbgprint("sent %d\n", val);
			}
			free(sendBuf);
			mySendPacket->clear();

			bytesTransferred = 0;
			while(bytesTransferred < 4)
			{
				val = ::recv(clientfd, buf, 4, 0);
				if(val < 0)
				{
					g_pLogger->LogInfo(QString("Hybrid Network Error"));
					//TODO: handle recv errors correctly
					return;
				}
				bytesTransferred += val;
			}
			memcpy(&packetSize, buf, 4);
	
			while(bytesTransferred < packetSize)
			{
				val = ::recv(clientfd, buf+bytesTransferred, packetSize-bytesTransferred, 0);
				if(val < 0)
				{
					g_pLogger->LogInfo(QString("Hybrid Network Error"));
					//TODO: handle recv errors correctly
					return;
				}
				bytesTransferred += val;
			}
			recvPacket.fromBytes(buf);
			//do stuff here to get the packets into the simulation
			recvPacket.clear();
		}
		delete mySendPacket;
		delete buf;
		sleep(1);
	}
}

void AddHybridPacket(Packet * packet)
{
	if(g_pHybridClient != NULL)
	{
		g_pHybridClient->addPacket(packet);
		//printf("added packet to hybrid client\n");
	}
	else if(g_pHybridServer != NULL)
	{
		g_pHybridServer->addPacket(packet);
		//printf("added packet to hybrid server\n");
	}
}

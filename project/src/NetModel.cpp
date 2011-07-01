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

#include "NetModel.h"
#include "Network.h"

unsigned char mode = 0;

NetModel::NetModel(const QString & strModelName)
: CarModel(strModelName)
{
}

NetModel::NetModel(const NetModel & copy)
: CarModel(copy)
{
}

NetModel::~NetModel()
{
}

NetModel & NetModel::operator = (const NetModel & copy)
{
	CarModel::operator = (copy);

	return *this;
}

QString NetModel::GetCarListColumnText(CarListColumn eColumn) const
{
	switch (eColumn)
	{
	case CarListColumnType:
		return "Network";
	default:
		return CarModel::GetCarListColumnText(eColumn);
	}
}

bool NetModel::IsActive() const
{
	return m_tTimestamp + MakeTime(NETWORK_TIMEOUT_SECS, NETWORK_TIMEOUT_USECS) >= GetLastEventTime();
}

int NetModel::Init(const std::map<QString, QString> & mapParams)
{
	if (CarModel::Init(mapParams))
		return 1;

	return 0;
}

int NetModel::PreRun()
{
	if (CarModel::PreRun())
		return 1;

	m_tTimestamp = timeval0;

	return 0;
}

int NetModel::ProcessEvent(SimEvent & event)
{
	if (CarModel::ProcessEvent(event))
		return 1;

	switch (event.GetEventID())
	{
	case EVENT_CARMODEL_UPDATE:
	{
		//printf("here------------------1");
		//fflush(stdout);
		bool bUpdated = false;
		std::map<in_addr_t, std::vector<Packet *> > * pMapBuffers;

		pMapBuffers = GetServer() == NULL ? NULL : GetServer()->acquireLock();

		if (pMapBuffers != NULL)
		{
			//printf("2");
			//fflush(stdout);
			std::map<in_addr_t, std::vector<Packet *> >::iterator iterPackets = pMapBuffers->find(m_ipCar);
			if (iterPackets != pMapBuffers->end())
			{
				//printf("3");
				//fflush(stdout);
				// got messages, ordered from oldest to most recent, in priority queue
				while (!iterPackets->second.empty())
				{
					switch (iterPackets->second.front()->m_ePacketType)
					{
					case ptSafety:
					{
						SafetyPacket * pMsg = (SafetyPacket *)iterPackets->second.front();
						if (pMsg->m_tTime > m_tTimestamp)
						{
							m_ptPosition = pMsg->m_ptPosition;
							m_iSpeed = pMsg->m_iSpeed;
							m_iHeading = pMsg->m_iHeading;
							m_tTimestamp = pMsg->m_tTime;
							m_iCurrentRecord = pMsg->m_iRecord;
							g_pMapDB->GetAbsoluteRecord(m_iCurrentRecord, pMsg->m_iCountyCode);
							m_bForwards = pMsg->m_cDirection == MESSAGE_DIRECTION_FORWARDS;
							m_iCRShapePoint = pMsg->m_iShapePoint;
							m_fCRProgress = pMsg->m_fProgress;
							bUpdated = true;
						}
						//wtf was this for, looks pretty useless to me
						//so I commented it out since we don't have m_eType
						//anymore anyway -MH
						/*
						switch (pMsg->m_eType)
						{
						case Message::MessageTypeExtended:
							break;
						case Message::MessageTypeControl:
							break;
						case Message::MessageTypeEmergency:
							break;
						case Message::MessageTypeWarning:
							break;
						default:
							break;
						}
						break;
						*/
						break;
					}
					//!!! here is where we update stuff -MH
					case ptGeneric:
					{
						Packet * pMsg = (Packet *)iterPackets->second.front();
						//if (pMsg->m_tTime > m_tTimestamp)
						{
							m_ptPosition = pMsg->m_ptTXPosition;
							m_iHeading = pMsg->m_iHeading;
							bUpdated = true;
						}
						break;
					}
					default:
						break;
					}
					
					pop_heap(iterPackets->second.begin(), iterPackets->second.end(), ComparePacketPtrs);
					iterPackets->second.pop_back();
				}
			}
		}
		GetServer()->releaseLock();
	}
	default:
		return 0;
	}
}

/*
int NetModel::Iteration(struct timeval tCurrent)
{
	std::map<in_addr_t, std::vector<Message> > * pMapBuffers = GetServer()->acquireLock();
	bool bUpdated = false;

	if (pMapBuffers != NULL)
	{
		std::map<in_addr_t, std::vector<Message> >::iterator iterMessages = pMapBuffers->find(m_ipCar);
		if (iterMessages != pMapBuffers->end())
		{
			// got messages, ordered from oldest to most recent, in priority queue
			while (!iterMessages->second.empty())
			{
				if (iterMessages->second.front().m_tTime > m_tTimestamp)
				{
					m_ptPosition = iterMessages->second.front().m_ptPosition;
					m_iSpeed = iterMessages->second.front().m_iSpeed;
					m_iHeading = iterMessages->second.front().m_iHeading;
					m_tTimestamp = iterMessages->second.front().m_tTime;
					m_iCurrentRecord = iterMessages->second.front().m_iRecord;
					g_pMapDB->GetAbsoluteRecord(m_iCurrentRecord, iterMessages->second.front().m_iCountyCode);
					m_bForwards = iterMessages->second.front().m_cDirection == MESSAGE_DIRECTION_FORWARDS;
					m_iCRShapePoint = iterMessages->second.front().m_iShapePoint;
					m_fCRProgress = iterMessages->second.front().m_fProgress;
					bUpdated = true;
				}
				switch (iterMessages->second.front().m_eType)
				{
				case Message::MessageTypeExtended:
					break;
				case Message::MessageTypeControl:
					break;
				case Message::MessageTypeEmergency:
					break;
				case Message::MessageTypeWarning:
					break;
				default:
					break;
				}

				pop_heap(iterMessages->second.begin(), iterMessages->second.end(), CompareMessages);
				iterMessages->second.pop_back();
			}
		}
		GetServer()->releaseLock();
	}

	if (bUpdated)
		return CarModel::Iteration(tCurrent) ? 1 : 0;
	else
		return 2;
}
*/
int NetModel::PostRun()
{
	if (CarModel::PostRun())
		return 1;

	return 0;
}

int NetModel::Save(std::map<QString, QString> & mapParams)
{
	if (CarModel::Save(mapParams))
		return 1;

	return 0;
}

void NetModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	CarModel::GetParams(mapParams);
}

int NetModel::Cleanup()
{
	if (CarModel::Cleanup())
		return 1;

	return 0;
}

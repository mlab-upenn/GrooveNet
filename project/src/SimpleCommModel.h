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

#ifndef _SIMPLECOMMMODEL_H
#define _SIMPLECOMMMODEL_H

#define SIMPLECOMMMODEL_NAME "SimpleCommModel"

#include "CarModel.h"

class SimpleCommModel : public CarCommModel
{
public:
	typedef struct RebroadcastMessageStruct
	{
		SafetyPacket msg;
		struct timeval tIntervalLow, tIntervalHigh;
	} RebroadcastMessage;

	inline virtual QString GetModelType() const
	{
		return SIMPLECOMMMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(SIMPLECOMMMODEL_NAME) == 0 || (bDescendSufficient && CarCommModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	SimpleCommModel(const QString & strModelName = QString::null);
	SimpleCommModel(const SimpleCommModel & copy);
	virtual ~SimpleCommModel();

	virtual SimpleCommModel & operator = (const SimpleCommModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int ProcessEvent(SimEvent & event);
	virtual int Save(std::map<QString, QString> & mapParams);

	virtual bool DoUpdate(struct timeval tCurrent);

	virtual bool TransmitMessage(Packet * msg);
	virtual void AddMessageToRebroadcastQueue(const SafetyPacket & msg);

	inline virtual bool IsGateway() const
	{
		return m_bGateway;
	}

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

protected:
//	std::vector<RebroadcastMessage> m_vecRebroadcast;
	bool m_bRebroadcast, m_bGateway, m_bJitter;
	struct timeval m_tRebroadcastInterval;
};

inline void DestroyRebroadcastMessage(void * ptr)
{
	delete (SimpleCommModel::RebroadcastMessage *)ptr;
}

inline Model * SimpleCommModelCreator(const QString & strModelName)
{
	return new SimpleCommModel(strModelName);
}

#endif

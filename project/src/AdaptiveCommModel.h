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

#ifndef _ADAPTIVECOMMMODEL_H
#define _ADAPTIVECOMMMODEL_H

#define ADAPTIVECOMMMODEL_NAME "AdaptiveCommModel"

#include "SimpleCommModel.h"

class AdaptiveCommModel : public SimpleCommModel
{
public:
	inline virtual QString GetModelType() const
	{
		return ADAPTIVECOMMMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(ADAPTIVECOMMMODEL_NAME) == 0 || (bDescendSufficient && SimpleCommModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	AdaptiveCommModel(const QString & strModelName = QString::null);
	AdaptiveCommModel(const AdaptiveCommModel & copy);
	virtual ~AdaptiveCommModel();

	virtual AdaptiveCommModel & operator = (const AdaptiveCommModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int ProcessEvent(SimEvent & event);
	virtual int Save(std::map<QString, QString> & mapParams);

	virtual bool DoUpdate(struct timeval tCurrent);

	virtual void AddMessageToRebroadcastQueue(const SafetyPacket & msg);
	virtual struct timeval GetRbxInterval(const SafetyPacket & msg, bool bFirst) const;
	virtual bool DoRebroadcast(const SafetyPacket & msg, struct timeval tRelevant) const;

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

protected:
	struct timeval m_tFirstRbxJitter, m_tMaxRbxInterval, m_tBackoffWindow;
	float m_fRbxThresholdHigh, m_fRbxThresholdLow;
	bool m_bFastRbx, m_bAdaptive, m_bDistBased, m_bLocBased;
};

inline Model * AdaptiveCommModelCreator(const QString & strModelName)
{
	return new AdaptiveCommModel(strModelName);
}

#endif

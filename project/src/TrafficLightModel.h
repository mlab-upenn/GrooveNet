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

#ifndef _TRAFFICLIGHTMODEL_H
#define _TRAFFICLIGHTMODEL_H

#include "Model.h"

#define TRAFFICLIGHTMODEL_NAME "TrafficLightModel"

#define EVENT_TRAFFICLIGHTMODEL_SIGNAL 412401

class TrafficLightModel : public Model
{
public:
	inline virtual QString GetModelType() const
	{
		return TRAFFICLIGHTMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(TRAFFICLIGHTMODEL_NAME) == 0 || (bDescendSufficient && Model::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	TrafficLightModel(const QString & strModelName = QString::null);
	TrafficLightModel(const TrafficLightModel & copy);
	virtual ~TrafficLightModel();

	virtual TrafficLightModel & operator = (const TrafficLightModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int ProcessEvent(SimEvent & event);
	virtual int Save(std::map<QString, QString> & mapParams);
	virtual int Cleanup();
	
	static void GetParams(std::map<QString, ModelParameter> & mapParams);

protected:
	struct timeval m_tGreenLightTime;
	unsigned int m_iVertexCounter;
};

inline Model * TrafficLightModelCreator(const QString & strModelName)
{
	return new TrafficLightModel(strModelName);
}

#endif

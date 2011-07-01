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

#ifndef _RANDOMWAYPOINTMODEL_H
#define _RANDOMWAYPOINTMODEL_H

#include "SimUnconstrainedModel.h"

#include <vector>

#include "MapDB.h"

#define RANDOMWAYPOINTMODEL_NAME "RandomWaypointModel"

class RandomWaypointModel : public SimUnconstrainedMobilityModel
{
public:
	inline virtual QString GetModelType() const
	{
		return RANDOMWAYPOINTMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(RANDOMWAYPOINTMODEL_NAME) == 0 || (bDescendSufficient && SimUnconstrainedMobilityModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	RandomWaypointModel(const QString & strModelName = QString::null);
	RandomWaypointModel(const RandomWaypointModel & copy);
	virtual ~RandomWaypointModel();

	virtual RandomWaypointModel & operator = (const RandomWaypointModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int Save(std::map<QString, QString> & mapParams);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual bool GetInitialConditions(Coords & ptPosition, short & iSpeed, short & iHeading);
	virtual bool DoIteration(float fElapsed, Coords & ptPosition, short & iSpeed, short & iHeading);
	virtual bool SetProgress(float & fTime, Coords & ptPosition, short iSpeed, short iHeading);
	virtual short ChooseSpeed(short iSpeed) const;
	virtual short ChooseDirection(short iHeading) const;

protected:
	Coords m_ptStart;
	short m_iLowSpeed, m_iHighSpeed;
	struct timeval m_tInterval;
	float m_fProgress;
};

inline Model * RandomWaypointModelCreator(const QString & strModelName)
{
	return new RandomWaypointModel(strModelName);
}

#endif

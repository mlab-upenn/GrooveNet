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

#ifndef _STREETSPEEDMODEL_H
#define _STREETSPEEDMODEL_H

#include "SimModel.h"

#define STREETSPEEDMODEL_NAME "StreetSpeedModel"

class StreetSpeedModel : public SimMobilityModel
{
public:
	inline virtual QString GetModelType() const
	{
		return STREETSPEEDMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(STREETSPEEDMODEL_NAME) == 0 || (bDescendSufficient && SimMobilityModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	StreetSpeedModel(const QString & strModelName = QString::null);
	StreetSpeedModel(const StreetSpeedModel & copy);
	virtual ~StreetSpeedModel();

	virtual StreetSpeedModel & operator = (const StreetSpeedModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int Save(std::map<QString, QString> & mapParams);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual bool GetInitialConditions(unsigned int & iRecord, Coords & ptPosition, short & iSpeed, short & iHeading, unsigned char & iLane);
	virtual bool DoIteration(float fElapsed, unsigned int & iRecord, Coords & ptPosition, short & iSpeed, short & iHeading, unsigned char & iLane);
	virtual short ChooseSpeed(unsigned int iRecord) const;

protected:
	SimTripModel * m_pTripModel;
};

inline Model * StreetSpeedModelCreator(const QString & strModelName)
{
	return new StreetSpeedModel(strModelName);
}

#endif

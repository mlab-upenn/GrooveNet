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

#ifndef _FIXEDMOBILITYMODEL_H
#define _FIXEDMOBILITYMODEL_H

#include "SimModel.h"

#define FIXEDMOBILITYMODEL_NAME "FixedMobilityModel"

class FixedMobilityModel : public SimMobilityModel
{
public:
	inline virtual QString GetModelType() const
	{
		return FIXEDMOBILITYMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(FIXEDMOBILITYMODEL_NAME) == 0 || (bDescendSufficient && SimMobilityModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	FixedMobilityModel(const QString & strModelName = QString::null);
	FixedMobilityModel(const FixedMobilityModel & copy);
	virtual ~FixedMobilityModel();

	virtual FixedMobilityModel & operator = (const FixedMobilityModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int Save(std::map<QString, QString> & mapParams);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual bool GetInitialConditions(unsigned int & iRecord, Coords & ptPosition, short & iSpeed, short & iHeading, unsigned char & iLane);
	virtual bool DoIteration(float fElapsed, unsigned int & iRecord, Coords & ptPosition, short & iSpeed, short & iHeading, unsigned char & iLane);
	inline virtual short ChooseSpeed(unsigned int iRecord __attribute__((unused)) ) const
	{
		return 0; // fixed = doesn't move
	}

protected:
	SimTripModel * m_pTripModel;
};

inline Model * FixedMobilityModelCreator(const QString & strModelName)
{
	return new FixedMobilityModel(strModelName);
}

#endif

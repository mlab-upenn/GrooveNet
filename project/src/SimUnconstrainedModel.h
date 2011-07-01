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

#ifndef _SIMUNCONSTRAINEDMODEL_H
#define _SIMUNCONSTRAINEDMODEL_H

#include "CarModel.h"

#define SIMUNCONSTRAINEDMOBILITYMODEL_NAME "SimUnconstrainedMobilityModel"

class SimUnconstrainedMobilityModel : public Model
{
public:
	inline virtual QString GetModelType() const
	{
		return SIMUNCONSTRAINEDMOBILITYMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(SIMUNCONSTRAINEDMOBILITYMODEL_NAME) == 0 || (bDescendSufficient && Model::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	SimUnconstrainedMobilityModel(const QString & strModelName = QString::null);
	SimUnconstrainedMobilityModel(const SimUnconstrainedMobilityModel & copy);
	virtual ~SimUnconstrainedMobilityModel();

	virtual SimUnconstrainedMobilityModel & operator = (const SimUnconstrainedMobilityModel & copy);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual bool GetInitialConditions(Coords & ptPosition, short & iSpeed, short & iHeading) = 0;
	virtual bool DoIteration(float fElapsed, Coords & ptPosition, short & iSpeed, short & iHeading) = 0;
	virtual short ChooseSpeed(short iSpeed) const = 0;
	virtual short ChooseDirection(short iHeading) const = 0;

	inline void SetCar(in_addr_t ipCar)
	{
		m_ipCar = ipCar;
	}
	inline in_addr_t GetCar() const
	{
		return m_ipCar;
	}

protected:
	in_addr_t m_ipCar;
};

#define SIMUNCONSTRAINEDMODEL_NAME "SimUnconstrainedModel"

class SimUnconstrainedModel : public CarModel
{
public:
	inline virtual QString GetModelType() const
	{
		return SIMUNCONSTRAINEDMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(SIMUNCONSTRAINEDMODEL_NAME) == 0 || (bDescendSufficient && CarModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	SimUnconstrainedModel(const QString & strModelName = QString::null);
	SimUnconstrainedModel(const SimUnconstrainedModel & copy);
	virtual ~SimUnconstrainedModel();

	virtual SimUnconstrainedModel & operator = (const SimUnconstrainedModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int ProcessEvent(SimEvent & event);
	virtual int PostRun();
	virtual int Save(std::map<QString, QString> & mapParams);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual QString GetCarListColumnText(CarListColumn eColumn) const;
	inline virtual bool IsActive() const
	{
		return m_bActive;
	}

protected:
	SimUnconstrainedMobilityModel * m_pMobilityModel;

	bool m_bActive;
	struct timeval m_tStartTime;
};

inline Model * SimUnconstrainedModelCreator(const QString & strModelName)
{
	return new SimUnconstrainedModel(strModelName);
}

#endif

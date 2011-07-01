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

#ifndef _SIMMODEL_H
#define _SIMMODEL_H

#include "CarModel.h"

#define SIMMOBILITYMODEL_NAME "SimMobilityModel"

class SimMobilityModel : public Model
{
public:
	inline virtual QString GetModelType() const
	{
		return SIMMOBILITYMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(SIMMOBILITYMODEL_NAME) == 0 || (bDescendSufficient && Model::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	SimMobilityModel(const QString & strModelName = QString::null);
	SimMobilityModel(const SimMobilityModel & copy);
	virtual ~SimMobilityModel();

	virtual SimMobilityModel & operator = (const SimMobilityModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int Save(std::map<QString, QString> & mapParams);
	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual bool GetInitialConditions(unsigned int & iRecord, Coords & ptPosition, short & iSpeed, short & iHeading, unsigned char & iLane) = 0;
	virtual bool DoIteration(float fElapsed, unsigned int & iRecord, Coords & ptPosition, short & iSpeed, short & iHeading, unsigned char & iLane) = 0;
	virtual void AssignLane(unsigned int iOldRecord, unsigned int iRecord, unsigned char & iLane);
	virtual void SwitchLanes(unsigned int iRecord, unsigned short iShapePoint, float fProgress, short iSpeed, bool bForwards, unsigned char & iLane);
	virtual short ChooseSpeed(unsigned int iRecord) const = 0;

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
	bool m_bMultilane;
};

#define SIMTRIPMODEL_NAME "SimTripModel"

class SimTripModel : public Model
{
public:
	inline virtual QString GetModelType() const
	{
		return SIMTRIPMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(SIMTRIPMODEL_NAME) == 0 || (bDescendSufficient && Model::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	SimTripModel(const QString & strModelName = QString::null);
	SimTripModel(const SimTripModel & copy);
	virtual ~SimTripModel();

	virtual SimTripModel & operator = (const SimTripModel & copy);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual unsigned int GetCurrentRecord() const = 0;
	virtual bool IsGoingForwards() const = 0;
	virtual unsigned short GetCRShapePoint() const = 0;
	virtual float GetCRProgress() const = 0;
	virtual Coords GetCurrentPosition() const = 0;
	// current direction reported in centidegrees, clockwise from true North
	virtual short GetCurrentDirection() const = 0;

	virtual bool SetProgress(float & fTime, short iSpeed, unsigned int & iNextRecord) = 0;

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

#define SIMMODEL_NAME "SimModel"

class SimModel : public CarModel
{
public:
	inline virtual QString GetModelType() const
	{
		return SIMMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(SIMMODEL_NAME) == 0 || (bDescendSufficient && CarModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	SimModel(const QString & strModelName = QString::null);
	SimModel(const SimModel & copy);
	virtual ~SimModel();

	virtual SimModel & operator = (const SimModel & copy);

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
	SimMobilityModel * m_pMobilityModel;
	SimTripModel * m_pTripModel;

	bool m_bActive;
	struct timeval m_tStartTime;
};

inline Model * SimModelCreator(const QString & strModelName)
{
	return new SimModel(strModelName);
}

#endif

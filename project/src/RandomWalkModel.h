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

#ifndef _RANDOMWALKMODEL_H
#define _RANDOMWALKMODEL_H

#include "SimModel.h"

#include <vector>

#include "MapDB.h"

#define RANDOMWALKMODEL_NAME "RandomWalkModel"

class RandomWalkModel : public SimTripModel
{
public:
	inline virtual QString GetModelType() const
	{
		return RANDOMWALKMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(RANDOMWALKMODEL_NAME) == 0 || (bDescendSufficient && SimTripModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	RandomWalkModel(const QString & strModelName = QString::null);
	RandomWalkModel(const RandomWalkModel & copy);
	virtual ~RandomWalkModel();

	virtual RandomWalkModel & operator = (const RandomWalkModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int Save(std::map<QString, QString> & mapParams);
	virtual int Cleanup();

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual unsigned int GetCurrentRecord() const;
	virtual bool IsGoingForwards() const;
	virtual unsigned short GetCRShapePoint() const;
	virtual float GetCRProgress() const;
	virtual Coords GetCurrentPosition() const;
	virtual short GetCurrentDirection() const;

	virtual bool SetProgress(float & fTime, short iSpeed, unsigned int & iNextRecord);

protected:
	virtual unsigned int GetNextRecord(unsigned int iVertex, unsigned int iPrevRecord);
	virtual unsigned int ChooseRandomRecord(const std::vector<unsigned int> & vecRecords, unsigned int iPrevRecord);

	unsigned int m_iCurrentRecord;
	unsigned int m_iCRShapePoint;
	float m_fCRProgress;
	bool m_bForwards;
	Address m_sStartAddress;
	QString m_strStartAddress;
};

inline Model * RandomWalkModelCreator(const QString & strModelName)
{
	return new RandomWalkModel(strModelName);
}

#endif

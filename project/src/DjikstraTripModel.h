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

#ifndef _DJIKSTRATRIPMODEL_H
#define _DJIKSTRATRIPMODEL_H

#include "RandomWalkModel.h"

#include <list>

#define DJIKSTRATRIPMODEL_NAME "DjikstraTripModel"

class DjikstraTripModel : public RandomWalkModel
{
public:
	inline virtual QString GetModelType() const
	{
		return DJIKSTRATRIPMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(DJIKSTRATRIPMODEL_NAME) == 0 || (bDescendSufficient && RandomWalkModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	DjikstraTripModel(const QString & strModelName = QString::null);
	DjikstraTripModel(const DjikstraTripModel & copy);
	virtual ~DjikstraTripModel();

	virtual DjikstraTripModel & operator = (const DjikstraTripModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int Save(std::map<QString, QString> & mapParams);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual bool SetProgress(float & fTime, short iSpeed, unsigned int & iNextRecord);

protected:
	virtual unsigned int GetNextRecord(unsigned int iVertex, unsigned int iPrevRecord);

	unsigned int m_iEndShapePoint;
	float m_fEndProgress;
	std::list<unsigned int> m_listPathRecords;
	std::list<Address> m_listWaypoints;
	Address m_sFinishAddress;
	QString m_strFinishAddress, m_strWaypoints;
};

inline Model * DjikstraTripModelCreator(const QString & strModelName)
{
	return new DjikstraTripModel(strModelName);
}

#endif

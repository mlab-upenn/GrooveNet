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

#ifndef _SIGHTSEEINGMODEL_H
#define _SIGHTSEEINGMODEL_H

#include "RandomWalkModel.h"

#include <list>

#define SIGHTSEEINGMODEL_NAME "SightseeingModel"

class SightseeingModel : public RandomWalkModel
{
public:
	inline virtual QString GetModelType() const
	{
		return SIGHTSEEINGMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(SIGHTSEEINGMODEL_NAME) == 0 || (bDescendSufficient && RandomWalkModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	SightseeingModel(const QString & strModelName = QString::null);
	SightseeingModel(const SightseeingModel & copy);
	virtual ~SightseeingModel();

	virtual SightseeingModel & operator = (const SightseeingModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int Save(std::map<QString, QString> & mapParams);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual bool SetProgress(float & fTime, short iSpeed, unsigned int & iNextRecord);

protected:
	virtual unsigned int GetNextRecord(unsigned int iVertex, unsigned int iPrevRecord);

	unsigned int m_iStartShapePoint;
	float m_fStartProgress;
	float m_fMaxDistance;
	std::list<unsigned int> m_listPathRecords;
};

inline Model * SightseeingModelCreator(const QString & strModelName)
{
	return new SightseeingModel(strModelName);
}

#endif

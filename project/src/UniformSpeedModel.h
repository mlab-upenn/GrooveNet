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

#ifndef _UNIFORMSPEEDMODEL_H
#define _UNIFORMSPEEDMODEL_H

#include "StreetSpeedModel.h"

#define UNIFORMSPEEDMODEL_NAME "UniformSpeedModel"

class UniformSpeedModel : public StreetSpeedModel
{
public:
	inline virtual QString GetModelType() const
	{
		return UNIFORMSPEEDMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(UNIFORMSPEEDMODEL_NAME) == 0 || (bDescendSufficient && StreetSpeedModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	UniformSpeedModel(const QString & strModelName = QString::null);
	UniformSpeedModel(const UniformSpeedModel & copy);
	virtual ~UniformSpeedModel();

	virtual UniformSpeedModel & operator = (const UniformSpeedModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int Save(std::map<QString, QString> & mapParams);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual short ChooseSpeed(unsigned int iRecord) const;

protected:
	typedef struct UniformSpeedRangeStruct {
		union {
			short iAbsolute; // absolute difference from mean speed
			float fRelative; // fraction of mean speed
		};
		enum {
			UniformSpeedRangeAbsolute = 0,
			UniformSpeedRangeRelative = 1
		} eType;
	} UniformSpeedRange;

	virtual short ChooseRandomSpeed(short iSpeed) const;

	UniformSpeedRange m_sLow, m_sHigh;
};

inline Model * UniformSpeedModelCreator(const QString & strModelName)
{
	return new UniformSpeedModel(strModelName);
}

#endif

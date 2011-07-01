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

#ifndef _MODEL_H
#define _MODEL_H

#include "Global.h"
#include "SimBase.h"

#include <qstring.h>
#include <qmutex.h>
#include <stdlib.h>
#include <map>

#define NULLMODEL_NAME "NullModel"

#define MODEL_NAME "Model"

typedef enum ModelParameterTypeEnum
{
	ModelParameterTypeText = 0,
	ModelParameterTypeInt,
	ModelParameterTypeFloat,
	ModelParameterTypeBool,
	ModelParameterTypeYesNo,
	ModelParameterTypeIP,
	ModelParameterTypeFile,
	ModelParameterTypeModel,
	ModelParameterTypeModels,
	ModelParameterTypeAddress,
	ModelParameterTypeAddresses,
	ModelParameterTypeGPS,
	ModelParameterTypeCoords,

	ModelParameterRandomize = 0,
	ModelParameterFixed = 16
} ModelParameterType;

typedef struct ModelParameterStruct
{
	QString strValue;
	QString strAuxData; // auxillary data for the parameter type
	QString strDesc;
	ModelParameterType eType;
} ModelParameter;



class Model{
public:
	inline virtual QString GetModelType() const
	{
		return MODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient __attribute__ ((unused)) = true) const
	{
		return strModelType.compare(MODEL_NAME) == 0;
	}

	Model(const QString & strModelName = QString::null);
	Model(const Model & copy);
	virtual ~Model();

	virtual Model & operator = (const Model & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	inline virtual int PreRun()
	{
		m_tLastEvent = timeval0;
		return 0;
	}
	inline virtual int ProcessEvent(SimEvent & event)
	{
		m_tLastEvent = event.GetTimestamp();
		return 0;
	}
/*	inline virtual int Iteration(struct timeval tCurrent)
	{
		m_tLastIteration = tCurrent;
		return 0;
	}
	inline virtual int PostIteration()
	{
		return 0;
	}
*/	inline virtual int PostRun()
	{
		return 0;
	}
	virtual int Save(std::map<QString, QString> & mapParams);
	inline virtual int Cleanup()
	{
		return 0;
	}

	inline virtual void SettingsChanged() {}
/*
	inline virtual struct timeval GetDelay() const
	{
		return m_tDelay;
	}
	inline virtual void SetDelay(const struct timeval & tDelay)
	{
		m_tDelay = tDelay;
	}
*/
	inline virtual const QString & GetModelName() const
	{
		return m_strModelName;
	}
	inline virtual struct timeval GetLastEventTime() const
	{
		return m_tLastEvent;
	}

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	QMutex m_mutexUpdate;

protected:
	// initialized parameters
//	struct timeval m_tDelay;

	// attributes
	QString m_strModelName;
	struct timeval m_tLastEvent;
};

typedef Model * (*ModelCreator) (const QString &);

inline const QString & GetParam(const std::map<QString, QString> & mapParams, const QString & strKey, const QString & strDefault = QString::null)
{
	std::map<QString, QString>::const_iterator iterParam = mapParams.find(strKey);
	if (iterParam == mapParams.end())
		return strDefault;
	else
		return iterParam->second;
}

#endif

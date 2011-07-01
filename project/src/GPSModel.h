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

#ifndef _GPSMODEL_H
#define _GPSMODEL_H

#include "CarModel.h"

#include <qthread.h>

#define GPSMODEL_NAME "GPSModel"

typedef struct GPSDataStruct
{
	struct timeval tTimestamp;
	Coords ptPosition; // TIGER coordinates
	short iSpeed; // in mph
	short iHeading; // in hundredths of a degree
	// TODO: add other fields later
} GPSData;

#ifndef MAX_DEADLOCK
#define MAX_DEADLOCK 30000
#endif

class GPSProcessor : public QThread
{
public:
	inline virtual QString GetGPSProcessorType() const
	{
		return "";
	}

	GPSProcessor();
	virtual ~GPSProcessor();

	inline GPSData * acquireLock(bool bWait = true)
	{
		if (bWait) {
			m_mutexData.lock();
			return &m_sGPSData;
		} else
			return m_mutexData.tryLock() ? &m_sGPSData : NULL;
	}
	inline void releaseLock()
	{
		m_mutexData.unlock();
	}

	virtual bool Init(const std::map<QString, QString> & mapParams) = 0;
	virtual void Save(std::map<QString, QString> & mapParams) = 0;
	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual void start(Priority priority = InheritPriority);
	virtual bool wait(unsigned long time = ULONG_MAX);

protected:
	GPSData m_sGPSData;
	QMutex m_mutexData;
	bool m_bCancelled;
};

#define GPSMODEL_NAME "GPSModel"

class GPSModel : public CarModel
{
public:
	inline virtual QString GetModelType() const
	{
		return GPSMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(GPSMODEL_NAME) == 0 || (bDescendSufficient && CarModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	GPSModel(const QString & strModelName = QString::null);
	GPSModel(const GPSModel & copy);
	virtual ~GPSModel();

	virtual GPSModel & operator = (const GPSModel & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int PreRun();
	virtual int ProcessEvent(SimEvent & event);
	virtual int PostRun();
	virtual int Save(std::map<QString, QString> & mapParams);
	virtual int Cleanup();

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual QString GetCarListColumnText(CarListColumn eColumn) const;
	virtual bool IsActive() const;

protected:
	GPSProcessor * m_pGPS;
};

inline Model * GPSModelCreator(const QString & strModelName)
{
	return (Model *)new GPSModel(strModelName);
}

#endif

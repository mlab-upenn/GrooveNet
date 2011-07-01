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

#ifndef _CARREGISTRY_H
#define _CARREGISTRY_H

#include "CarModel.h"

class CarRegistry
{
public:
	CarRegistry();
	~CarRegistry();

	inline std::map<in_addr_t, CarModel *> * acquireLock(bool bWait = true)
	{
		if (bWait) {
			m_mutexRegistry.lock();
			return &m_mapRegistry;
		} else
			return m_mutexRegistry.tryLock() ? &m_mapRegistry : NULL;
	}
	inline std::map<in_addr_t, CarModel *> * getRegistry()
	{
		return &m_mapRegistry;
	}
	inline void releaseLock()
	{
		m_mutexRegistry.unlock();
	}

	inline void addCar(CarModel * pCar)
	{
		m_mutexRegistry.lock();
		m_mapRegistry.insert(std::pair<in_addr_t, CarModel *>(pCar->GetIPAddress(), pCar));
		m_mutexRegistry.unlock();
	}
	inline bool removeCar(in_addr_t ipCar)
	{
		bool bRemoved;
		m_mutexRegistry.lock();
		bRemoved = m_mapRegistry.erase(ipCar) > 0;
		m_mutexRegistry.unlock();
		return bRemoved;
	}

	bool GetCarsOnRecord(unsigned int iRecord, std::vector<CarModel *> & vecCars);
	bool GetCarsOnRecord(unsigned int iRecord, bool bForwards, std::vector<CarModel *> & vecCars);
	bool GetCarsOnRecords(const std::set<unsigned int> & setRecords, std::vector<CarModel *> & vecCars);
	bool GetCarsOnRecords(const std::set<unsigned int> & setRecords, bool bForwards, std::vector<CarModel *> & vecCars);
	bool GetCarsInRange(const CarModel * pCar, std::vector<CarModel *> & vecCars);
	bool GetCommunicatingCarsInRange(const CarModel * pCar, std::vector<CarModel *> & vecCars);
	bool GetLocalCars(std::vector<CarModel *> & vecCars);
	bool GetNetworkCars(std::vector<CarModel *> & vecCars);

protected:
	std::map<in_addr_t, CarModel *> m_mapRegistry;
	QMutex m_mutexRegistry;

private:
	inline CarRegistry(const CarRegistry & copy __attribute__ ((unused)) ) {}
	inline CarRegistry & operator = (const CarRegistry & copy __attribute__ ((unused)) ) {return *this;}
};

extern CarRegistry * g_pCarRegistry;

#endif

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

#include "CarRegistry.h"

CarRegistry::CarRegistry()
: m_mutexRegistry(true)
{
}

CarRegistry::~CarRegistry()
{
}

bool CarRegistry::GetCarsOnRecord(unsigned int iRecord, std::vector<CarModel *> & vecCars)
{
	std::map<in_addr_t, CarModel *>::iterator iterCar;
	bool bFound = false;

	for (iterCar = m_mapRegistry.begin(); iterCar != m_mapRegistry.end(); ++iterCar)
	{
		if (iterCar->second != NULL && iterCar->second->GetCurrentRecord() == iRecord)
		{
			vecCars.push_back(iterCar->second);
			bFound = true;
		}
	}

	return bFound;
}

bool CarRegistry::GetCarsOnRecord(unsigned int iRecord, bool bForwards, std::vector<CarModel *> & vecCars)
{
	std::map<in_addr_t, CarModel *>::iterator iterCar;
	bool bFound = false;

	for (iterCar = m_mapRegistry.begin(); iterCar != m_mapRegistry.end(); ++iterCar)
	{
		if (iterCar->second != NULL && iterCar->second->GetCurrentRecord() == iRecord && iterCar->second->IsGoingForwards() == bForwards)
		{
			vecCars.push_back(iterCar->second);
			bFound = true;
		}
	}

	return bFound;
}

bool CarRegistry::GetCarsOnRecords(const std::set<unsigned int> & setRecords, std::vector<CarModel *> & vecCars)
{
	std::map<in_addr_t, CarModel *>::iterator iterCar;
	bool bFound = false;

	for (iterCar = m_mapRegistry.begin(); iterCar != m_mapRegistry.end(); ++iterCar)
	{
		if (iterCar->second != NULL && setRecords.find(iterCar->second->GetCurrentRecord()) != setRecords.end())
		{
			vecCars.push_back(iterCar->second);
			bFound = true;
		}
	}

	return bFound;
}

bool CarRegistry::GetCarsOnRecords(const std::set<unsigned int> & setRecords, bool bForwards, std::vector<CarModel *> & vecCars)
{
	std::map<in_addr_t, CarModel *>::iterator iterCar;
	bool bFound = false;

	for (iterCar = m_mapRegistry.begin(); iterCar != m_mapRegistry.end(); ++iterCar)
	{
		if (iterCar->second != NULL && setRecords.find(iterCar->second->GetCurrentRecord()) != setRecords.end() && iterCar->second->IsGoingForwards() == bForwards)
		{
			vecCars.push_back(iterCar->second);
			bFound = true;
		}
	}

	return bFound;
}

bool CarRegistry::GetCarsInRange(const CarModel * pCar, std::vector<CarModel *> & vecCars)
{
	std::map<in_addr_t, CarModel *>::iterator iterCar;
	bool bFound = false;

	for (iterCar = m_mapRegistry.begin(); iterCar != m_mapRegistry.end(); ++iterCar)
	{
		if (iterCar->second != NULL && iterCar->first != pCar->GetIPAddress() && pCar->m_pPhysModel != NULL && pCar->m_pPhysModel->IsCarInRange(pCar->GetCurrentPosition(), iterCar->second->GetCurrentPosition()))
		{
			vecCars.push_back(iterCar->second);
			bFound = true;
		}
	}

	return bFound;
}

bool CarRegistry::GetCommunicatingCarsInRange(const CarModel * pCar, std::vector<CarModel *> & vecCars)
{
	std::map<in_addr_t, CarModel *>::iterator iterCar;
	bool bFound = false;

	for (iterCar = m_mapRegistry.begin(); iterCar != m_mapRegistry.end(); ++iterCar)
	{
		if (iterCar->second != NULL && iterCar->first != pCar->GetIPAddress() && iterCar->second->IsActive() && iterCar->second->m_pCommModel != NULL && pCar->m_pPhysModel != NULL && pCar->m_pPhysModel->IsCarInRange(pCar->GetCurrentPosition(), iterCar->second->GetCurrentPosition()))
		{
			vecCars.push_back(iterCar->second);
			bFound = true;
		}
	}

	return bFound;
}

bool CarRegistry::GetLocalCars(std::vector<CarModel *> & vecCars)
{
	std::map<in_addr_t, CarModel *>::iterator iterCar;
	bool bFound = false;

	for (iterCar = m_mapRegistry.begin(); iterCar != m_mapRegistry.end(); ++iterCar)
	{
		if (iterCar->second != NULL && iterCar->second->GetOwnerIPAddress() == CARMODEL_IPOWNER_LOCAL)
		{
			vecCars.push_back(iterCar->second);
			bFound = true;
		}
	}

	return bFound;
}

bool CarRegistry::GetNetworkCars(std::vector<CarModel *> & vecCars)
{
	std::map<in_addr_t, CarModel *>::iterator iterCar;
	bool bFound = false;

	for (iterCar = m_mapRegistry.begin(); iterCar != m_mapRegistry.end(); ++iterCar)
	{
		if (iterCar->second != NULL && iterCar->second->GetOwnerIPAddress() != CARMODEL_IPOWNER_LOCAL)
		{
			vecCars.push_back(iterCar->second);
			bFound = true;
		}
	}

	return bFound;
}

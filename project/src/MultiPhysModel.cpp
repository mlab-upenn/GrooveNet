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

#include "MultiPhysModel.h"
#include "CarRegistry.h"
#include "InfrastructureNodeRegistry.h"
#include "StringHelp.h"
#include "Simulator.h"

#define MULTIPHYSMODEL_PARAM_V2VMODEL "V2V"
#define MULTIPHYSMODEL_PARAM_V2VMODEL_DEFAULT "NULL"
#define MULTIPHYSMODEL_PARAM_V2VMODEL_DESC "V2V (model) -- The physical layer model to emulate for packets from another vehicle."
#define MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL "INFRASTRUCTURE"
#define MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL_DEFAULT "NULL"
#define MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL_DESC "INFRASTRUCTURE (model) -- The physical layer model to emulate for packets from an infrastructure node."

MultiPhysModel::MultiPhysModel(const QString & strModelName)
: CarPhysModel(strModelName)
{
}

MultiPhysModel::MultiPhysModel(const MultiPhysModel & copy)
: CarPhysModel(copy), m_mapPhysModels(copy.m_mapPhysModels), m_mapPhysModelNames(copy.m_mapPhysModelNames)
{
}

MultiPhysModel::~MultiPhysModel()
{
}

MultiPhysModel & MultiPhysModel::operator = (const MultiPhysModel & copy)
{
	CarPhysModel::operator =(copy);

	m_mapPhysModels = copy.m_mapPhysModels;
	m_mapPhysModelNames = copy.m_mapPhysModelNames;
	return *this;
}

int MultiPhysModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	Model * pModel;

	if (CarPhysModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, MULTIPHYSMODEL_PARAM_V2VMODEL, MULTIPHYSMODEL_PARAM_V2VMODEL_DEFAULT);
	m_mapPhysModelNames[MULTIPHYSMODEL_PARAM_V2VMODEL] = strValue;
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 2;
	m_mapPhysModels.insert(std::pair<QString, CarPhysModel *>(MULTIPHYSMODEL_PARAM_V2VMODEL, (CarPhysModel *)pModel));

	strValue = GetParam(mapParams, MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL, MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL_DEFAULT);
	m_mapPhysModelNames[MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL] = strValue;
	if (!g_pSimulator->m_ModelMgr.GetModel(strValue, pModel))
		return 3;
	m_mapPhysModels.insert(std::pair<QString, CarPhysModel *>(MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL, (CarPhysModel *)pModel));

	return 0;
}

int MultiPhysModel::Save(std::map<QString, QString> & mapParams)
{
	std::map<QString, QString>::iterator iterPhysModelName;

	if (CarPhysModel::Save(mapParams))
		return 1;

	for (iterPhysModelName = m_mapPhysModelNames.begin(); iterPhysModelName != m_mapPhysModelNames.end(); ++iterPhysModelName)
		mapParams[iterPhysModelName->first] = iterPhysModelName->second;
	return 0;
}

void MultiPhysModel::SetCar(in_addr_t ipCar)
{
	std::map<QString, CarPhysModel *>::const_iterator iterPhysModel;
	CarPhysModel::SetCar(ipCar);
	for (iterPhysModel = m_mapPhysModels.begin(); iterPhysModel != m_mapPhysModels.end(); ++iterPhysModel)
	{
		if (iterPhysModel->second != NULL)
			iterPhysModel->second->SetCar(ipCar);
	}
}

bool MultiPhysModel::DoUpdate(struct timeval tCurrent)
{
	std::map<QString, CarPhysModel *>::const_iterator iterPhysModel;
	if (!CarPhysModel::DoUpdate(tCurrent))
		return false;

	for (iterPhysModel = m_mapPhysModels.begin(); iterPhysModel != m_mapPhysModels.end(); ++iterPhysModel)
	{
		if (iterPhysModel->second != NULL)
			iterPhysModel->second->DoUpdate(tCurrent);
	}
	return true;
}

bool MultiPhysModel::BeginProcessPacket(Packet * packet)
{
	CarPhysModel * pModel = GetRelevantModel(packet);
	// pass on to the relevant physical model
	return pModel != NULL && pModel->BeginProcessPacket(packet);
}

bool MultiPhysModel::EndProcessPacket(Packet * packet)
{
	CarPhysModel * pModel = GetRelevantModel(packet);
	// pass on to the relevant physical model
	return pModel != NULL && pModel->EndProcessPacket(packet);
}

bool MultiPhysModel::ReceivePacket(Packet * packet)
{
	CarPhysModel * pModel = GetRelevantModel(packet);
	// pass on to the relevant physical model
	return pModel != NULL && pModel->ReceivePacket(packet);
}

float MultiPhysModel::GetRXRange(const Packet * packet) const
{
	CarPhysModel * pModel = GetRelevantModel(packet);
	// pass on to the relevant physical model
	return pModel == NULL ? 0.f : pModel->GetRXRange(packet);
}

bool MultiPhysModel::IsCarInRange(const Coords & ptCar, const Coords & ptPosition) const
{
	std::map<QString, CarPhysModel *>::const_iterator iterModel = m_mapPhysModels.find(MULTIPHYSMODEL_PARAM_V2VMODEL);
	return iterModel != m_mapPhysModels.end() && iterModel->second != NULL && iterModel->second->IsCarInRange(ptCar, ptPosition);
}

unsigned int MultiPhysModel::GetCollisionCount() const
{
	std::set<CarPhysModel *> setModels;
	std::set<CarPhysModel *>::iterator iterModel;
	std::map<QString, CarPhysModel *>::const_iterator iterPhysModel;
	unsigned int iCount = 0;
	for (iterPhysModel = m_mapPhysModels.begin(); iterPhysModel != m_mapPhysModels.end(); ++iterPhysModel)
	{
		if (iterPhysModel->second != NULL)
			setModels.insert(iterPhysModel->second);
	}
	for (iterModel = setModels.begin(); iterModel != setModels.end(); ++iterModel)
		iCount += (*iterModel)->GetCollisionCount();

	return iCount;
}

unsigned int MultiPhysModel::GetMessageCount() const
{
	std::set<CarPhysModel *> setModels;
	std::set<CarPhysModel *>::iterator iterModel;
	std::map<QString, CarPhysModel *>::const_iterator iterPhysModel;
	unsigned int iCount = 0;
	for (iterPhysModel = m_mapPhysModels.begin(); iterPhysModel != m_mapPhysModels.end(); ++iterPhysModel)
	{
		if (iterPhysModel->second != NULL)
			setModels.insert(iterPhysModel->second);
	}
	for (iterModel = setModels.begin(); iterModel != setModels.end(); ++iterModel)
		iCount += (*iterModel)->GetMessageCount();

	return iCount;
}

void MultiPhysModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	CarPhysModel::GetParams(mapParams);

	mapParams[MULTIPHYSMODEL_PARAM_V2VMODEL].strValue = MULTIPHYSMODEL_PARAM_V2VMODEL_DEFAULT;
	mapParams[MULTIPHYSMODEL_PARAM_V2VMODEL].strDesc = MULTIPHYSMODEL_PARAM_V2VMODEL_DESC;
	mapParams[MULTIPHYSMODEL_PARAM_V2VMODEL].eType = ModelParameterTypeModel;
	mapParams[MULTIPHYSMODEL_PARAM_V2VMODEL].strAuxData = CARPHYSMODEL_NAME;

	mapParams[MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL].strValue = MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL_DEFAULT;
	mapParams[MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL].strDesc = MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL_DESC;
	mapParams[MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL].eType = ModelParameterTypeModel;
	mapParams[MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL].strAuxData = CARPHYSMODEL_NAME;
}

CarPhysModel * MultiPhysModel::GetRelevantModel(const Packet * packet)
{
	// get relevant model depending on source of message
	std::map<in_addr_t, CarModel *> * pCarRegistry = g_pCarRegistry->acquireLock();
	std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry = g_pInfrastructureNodeRegistry->acquireLock();
	std::map<in_addr_t, CarModel *>::iterator iterTXCar = pCarRegistry->find(packet->m_ipTX);
	std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterTXNode = pNodeRegistry->find(packet->m_ipTX);
	std::map<QString, CarPhysModel *>::iterator iterModel;
	CarPhysModel * pModel = NULL;
	if (iterTXNode != pNodeRegistry->end() && iterTXNode->second != NULL)
	{
		iterModel = m_mapPhysModels.find(MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL);
		if (iterModel != m_mapPhysModels.end())
			pModel = iterModel->second;
	}
	else
	{
		iterModel = m_mapPhysModels.find(MULTIPHYSMODEL_PARAM_V2VMODEL);
		if (iterModel != m_mapPhysModels.end())
			pModel = iterModel->second;
	}
	g_pInfrastructureNodeRegistry->releaseLock();
	g_pCarRegistry->releaseLock();
	return pModel;
}

CarPhysModel * MultiPhysModel::GetRelevantModel(const Packet * packet) const
{
	// get relevant model depending on source of message
	std::map<in_addr_t, CarModel *> * pCarRegistry = g_pCarRegistry->acquireLock();
	std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry = g_pInfrastructureNodeRegistry->acquireLock();
	std::map<in_addr_t, CarModel *>::iterator iterTXCar = pCarRegistry->find(packet->m_ipTX);
	std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterTXNode = pNodeRegistry->find(packet->m_ipTX);
	std::map<QString, CarPhysModel *>::const_iterator iterModel;
	CarPhysModel * pModel = NULL;
	if (iterTXNode != pNodeRegistry->end() && iterTXNode->second != NULL)
	{
		iterModel = m_mapPhysModels.find(MULTIPHYSMODEL_PARAM_INFRASTRUCTUREMODEL);
		if (iterModel != m_mapPhysModels.end())
			pModel = iterModel->second;
	}
	else
	{
		iterModel = m_mapPhysModels.find(MULTIPHYSMODEL_PARAM_V2VMODEL);
		if (iterModel != m_mapPhysModels.end())
			pModel = iterModel->second;
	}
	g_pInfrastructureNodeRegistry->releaseLock();
	g_pCarRegistry->releaseLock();
	return pModel;
}

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

#include "ModelMgr.h"
#include "CarRegistry.h"
#include "Simulator.h"
#include "Logger.h"
#include "StringHelp.h"
#include "SimpleLinkModel.h"
#include "SimplePhysModel.h"
#include "CollisionPhysModel.h"
#include "MultiPhysModel.h"
#include "SimpleCommModel.h"
#include "AdaptiveCommModel.h"
#include "GrooveCommModel.h"
#include "GPSModel.h"
#include "NetModel.h"
#include "SimModel.h"
#include "FixedMobilityModel.h"
#include "StreetSpeedModel.h"
#include "UniformSpeedModel.h"
#include "CarFollowingModel.h"
#include "RandomWalkModel.h"
#include "DjikstraTripModel.h"
#include "SightseeingModel.h"
#include "SimUnconstrainedModel.h"
#include "RandomWaypointModel.h"
#include "InfrastructureNodeModel.h"
#include "TrafficLightModel.h"
#include "MapVisual.h"
#include "CarListVisual.h"

#include <set>

ModelMgr::ModelMgr()
: m_pModelTreeNodes(NULL), m_nModelTreeNodes(0), m_modelsMutex(true)
{
	/* add code to register your model type(s) here, as described below:
	 * RegisterModel(<model creation function>, <model type identifier>);
	 */
	RegisterModel(NULL, NULLMODEL_NAME);
	RegisterModel(SimpleLinkModelCreator, SIMPLELINKMODEL_NAME);
	RegisterModel(SimplePhysModelCreator, SIMPLEPHYSMODEL_NAME);
	RegisterModel(CollisionPhysModelCreator, COLLISIONPHYSMODEL_NAME);
	RegisterModel(MultiPhysModelCreator, MULTIPHYSMODEL_NAME);
	RegisterModel(SimpleCommModelCreator, SIMPLECOMMMODEL_NAME);
	RegisterModel(AdaptiveCommModelCreator, ADAPTIVECOMMMODEL_NAME);
	RegisterModel(GrooveCommModelCreator, GROOVECOMMMODEL_NAME);
	RegisterModel(GPSModelCreator, GPSMODEL_NAME);
	RegisterModel(NetModelCreator, NETMODEL_NAME);
	RegisterModel(SimModelCreator, SIMMODEL_NAME);
	RegisterModel(FixedMobilityModelCreator, FIXEDMOBILITYMODEL_NAME);
	RegisterModel(StreetSpeedModelCreator, STREETSPEEDMODEL_NAME);
	RegisterModel(UniformSpeedModelCreator, UNIFORMSPEEDMODEL_NAME);
	RegisterModel(CarFollowingModelCreator, CARFOLLOWINGMODEL_NAME);
	RegisterModel(RandomWalkModelCreator, RANDOMWALKMODEL_NAME);
	RegisterModel(DjikstraTripModelCreator, DJIKSTRATRIPMODEL_NAME);
	RegisterModel(SightseeingModelCreator, SIGHTSEEINGMODEL_NAME);
	RegisterModel(SimUnconstrainedModelCreator, SIMUNCONSTRAINEDMODEL_NAME);
	RegisterModel(RandomWaypointModelCreator, RANDOMWAYPOINTMODEL_NAME);
	RegisterModel(InfrastructureNodeModelCreator, INFRASTRUCTURENODEMODEL_NAME);
	RegisterModel(TrafficLightModelCreator, TRAFFICLIGHTMODEL_NAME);
	RegisterModel(MapVisualCreator, MAPVISUAL_NAME);
	RegisterModel(CarListVisualCreator, CARLISTVISUAL_NAME);

	// add null model to model registry
	AddModel("NULL", NULLMODEL_NAME, "", std::map<QString, QString>());
}

ModelMgr::~ModelMgr()
{
	DestroyAllModels();
}

int ModelMgr::RegisterModel(ModelCreator pfnModelCreator, const QString & strModelName)
{
	if (IsRegisteredModel(strModelName))
	{
		errno = EEXIST;
		return 0;
	}

	return m_mapModelRegistry.insert(std::pair<QString, ModelCreator>(strModelName, pfnModelCreator)).second;
}

int ModelMgr::IsRegisteredModel(const QString & strModelName) const
{
	return m_mapModelRegistry.find(strModelName) != m_mapModelRegistry.end();
}

bool ModelMgr::IsModelTypeOf(const QString & strModelType, const QString & strTargetType) const
{
	std::map<QString, ModelCreator>::const_iterator iterModelCreator = m_mapModelRegistry.find(strModelType);
	bool bRet = false;
	if (iterModelCreator != m_mapModelRegistry.end())
	{
		if (iterModelCreator->second != NULL)
		{
			Model * pModel = (*iterModelCreator->second)("");
			bRet = pModel != NULL && pModel->IsModelTypeOf(strTargetType);
			if (pModel != NULL)
				delete pModel;
		}
		else
			bRet = true; // NULL model is a type of everybody
	}
	return bRet;
}

int ModelMgr::UnregisterModel(const QString & strModelName)
{
	std::map<QString, ModelCreator>::iterator iterModel = m_mapModelRegistry.find(strModelName);
	if (iterModel == m_mapModelRegistry.end())
	{
		errno = ENOENT;
		return 0;
	}
	else
	{
		m_mapModelRegistry.erase(iterModel);
		return 1;
	}
}

int ModelMgr::GetModelCreator(const QString & strModelName, ModelCreator & pfnModelCreator) const
{
	std::map<QString, ModelCreator>::const_iterator iterModel = m_mapModelRegistry.find(strModelName);
	if (iterModel == m_mapModelRegistry.end())
	{
		pfnModelCreator = NULL;
		return 0;
	}
	else
	{
		pfnModelCreator = iterModel->second;
		return 1;
	}
}

typedef struct ModelGraphNodeStruct
{
	QString strName;
	unsigned int iDependencies;
	std::set<struct ModelGraphNodeStruct *> setEdges;
	ModelTreeNode * pTreeNode;
} ModelGraphNode;

static bool ModelGraphNodeCompare(ModelGraphNode * first, ModelGraphNode * second)
{
	return second->iDependencies < first->iDependencies;
}

int ModelMgr::BuildModelTree(const std::vector<std::pair<QString, QString> > & vecDependencies)
{
	std::vector<ModelGraphNode *> vecModelNodes;
	ModelGraphNode * pModels, * pModel;
	ModelTreeNode * pNewModelTreeNodes;
	std::map<QString, ModelGraphNode *> mapModels;
	std::map<QString, ModelGraphNode *>::iterator iterModel;
	std::set<ModelGraphNode *>::iterator iterEdge;
	std::list<ModelTreeNode *>::iterator iterListNode;
	QString strModel;
	unsigned int i;
	int delimPtr, start;

	/* initialize nodes */
	vecModelNodes.resize(m_nModelTreeNodes + vecDependencies.size());
	pModels = new ModelGraphNode[vecModelNodes.size()];
	pNewModelTreeNodes = new ModelTreeNode[vecModelNodes.size()];
	for (i = 0; i < m_nModelTreeNodes; i++)
	{
		mapModels.insert(std::pair<QString, ModelGraphNode *>(m_pModelTreeNodes[i].strModelName, vecModelNodes[i] = pModels + i));
		vecModelNodes[i]->pTreeNode = pNewModelTreeNodes + i;
		pNewModelTreeNodes[i].iBits = MODELTREEBITS_FATAL;
		pNewModelTreeNodes[i].strModelName = vecModelNodes[i]->strName = m_pModelTreeNodes[i].strModelName;
		pNewModelTreeNodes[i].pModel = m_pModelTreeNodes[i].pModel;
		vecModelNodes[i]->iDependencies = 0;
	}
	for (i = 0; i < vecDependencies.size(); i++)
	{
		mapModels.insert(std::pair<QString, ModelGraphNode *>(vecDependencies[i].first, vecModelNodes[m_nModelTreeNodes + i] = pModels + m_nModelTreeNodes + i));
		vecModelNodes[m_nModelTreeNodes + i]->pTreeNode = pNewModelTreeNodes + m_nModelTreeNodes + i;
		pNewModelTreeNodes[m_nModelTreeNodes + i].iBits = MODELTREEBITS_FATAL;
		pNewModelTreeNodes[m_nModelTreeNodes + i].strModelName = vecModelNodes[m_nModelTreeNodes + i]->strName = vecDependencies[i].first;
		pNewModelTreeNodes[m_nModelTreeNodes + i].pModel = m_mapModels[vecDependencies[i].first];
		vecModelNodes[m_nModelTreeNodes + i]->iDependencies = 0;
	}

	/* add old edges */
	for (i = 0; i < m_nModelTreeNodes; i++)
	{
		for (iterListNode = m_pModelTreeNodes[i].listDepends.begin(); iterListNode != m_pModelTreeNodes[i].listDepends.end(); ++iterListNode)
		{
			if ((iterModel = mapModels.find((*iterListNode)->strModelName)) != mapModels.end()) /* model exists */
			{
				if (iterModel->second->setEdges.insert(vecModelNodes[i]).second)
					vecModelNodes[i]->iDependencies++;
			}
			else
			{
				delete[] pModels;
				delete[] pNewModelTreeNodes;
				return 0;
			}
		}
	}
	/* add new edges */
	for (i = 0; i < vecDependencies.size(); i++)
	{
		start = 0;
		while ((delimPtr = vecDependencies[i].second.find(';', start)) > -1)
		{
			if (!(strModel = vecDependencies[i].second.mid(start, delimPtr - start).simplifyWhiteSpace()).isEmpty())
			{
				if ((iterModel = mapModels.find(strModel)) != mapModels.end()) /* model exists */
				{
					if (iterModel->second->setEdges.insert(vecModelNodes[m_nModelTreeNodes + i]).second)
						vecModelNodes[m_nModelTreeNodes + i]->iDependencies++;
				}
				else
				{
					delete[] pModels;
					delete[] pNewModelTreeNodes;
					return 0;
				}
			}
			start = delimPtr + 1;
		}
		/* take care of last model name (if there are any) */
		if (!(strModel = vecDependencies[i].second.mid(start).simplifyWhiteSpace()).isEmpty())
		{
			if ((iterModel = mapModels.find(strModel)) != mapModels.end()) /* model exists */
			{
				if (iterModel->second->setEdges.insert(vecModelNodes[m_nModelTreeNodes + i]).second)
					vecModelNodes[m_nModelTreeNodes + i]->iDependencies++;
			}
			else
			{
				delete[] pModels;
				delete[] pNewModelTreeNodes;
				return 0;
			}
		}
	}

	/* sort priority queue by # of edges */
	make_heap(vecModelNodes.begin(), vecModelNodes.end(), ModelGraphNodeCompare);

	/* dequeue in order of minimum edges */
	while (!vecModelNodes.empty())
	{
		if (vecModelNodes[0]->iDependencies)
		{
			/* error: cyclic redundancy found! */
			delete[] pModels;
			delete[] pNewModelTreeNodes;
			return 0;
		}
		else
		{
			pModel = vecModelNodes[0];
			pModel->pTreeNode->iBits = MODELTREEBITS_CLEAN;
			pop_heap(vecModelNodes.begin(), vecModelNodes.end(), ModelGraphNodeCompare);
			vecModelNodes.pop_back();

			for (iterEdge = pModel->setEdges.begin(); iterEdge != pModel->setEdges.end(); ++iterEdge)
			{
				(*iterEdge)->pTreeNode->listDepends.push_back(pModel->pTreeNode);
				(*iterEdge)->iDependencies--;
			}
			make_heap(vecModelNodes.begin(), vecModelNodes.end(), ModelGraphNodeCompare);
		}
	}

	delete[] pModels;
	if (m_pModelTreeNodes)
		delete[] m_pModelTreeNodes;
	m_nModelTreeNodes += vecDependencies.size();
	m_pModelTreeNodes = pNewModelTreeNodes;
	return 1;
}

int ModelMgr::RemoveModelFromTree(const QString & strModelName)
{
	/* look for model in array */
	ModelTreeNode * pRemove, * pEnd = m_pModelTreeNodes + m_nModelTreeNodes;
	unsigned int i;
	std::list<ModelTreeNode *>::iterator iterListNode;

	for (pRemove = m_pModelTreeNodes; pRemove < pEnd; pRemove++)
	{
		if (!pRemove->strModelName.compare(strModelName))
			break;
	}
	if (pRemove == pEnd)
		return -1; /* doesn't exist */

	for (i = 0; i < m_nModelTreeNodes; i++)
	{
		if (m_pModelTreeNodes + i == pRemove)
			continue;
		for (iterListNode = m_pModelTreeNodes[i].listDepends.begin(); iterListNode != m_pModelTreeNodes[i].listDepends.end(); ++iterListNode)
		{
			if (*iterListNode == pRemove)
				return 0; /* dependency found! */
		}
	}

	/* no dependencies found - just swap out with last item and update indexes */
	m_nModelTreeNodes--;
	*pRemove = m_pModelTreeNodes[m_nModelTreeNodes];
	for (i = 0; i < m_nModelTreeNodes; i++)
	{
		for (iterListNode = m_pModelTreeNodes[i].listDepends.begin(); iterListNode != m_pModelTreeNodes[i].listDepends.end(); ++iterListNode)
		{
			if (*iterListNode == m_pModelTreeNodes + m_nModelTreeNodes)
				*iterListNode = pRemove; /* update index */
		}
	}
	return 1; /* success */
}

int ModelMgr::AddModel(const QString & strModelName, const QString & strModelType, const QString & strDepends, const std::map<QString, QString> & mapParams)
{
	int result = 0;
	std::vector<std::pair<QString, QString> > vecDepends;

	if (AddModel(strModelName, strModelType, mapParams))
	{
		vecDepends.push_back(std::pair<QString, QString>(strModelName, strDepends));
		result =  BuildModelTree(vecDepends);
	}
	return result;
}

int ModelMgr::AddModel(const QString & strModelName, const QString & strModelType, const std::map<QString, QString> & mapParams)
{
	std::map<QString, Model *>::iterator iterModel;
	ModelCreator pfnModelCreator;
	Model * pModel;

	if (m_mapModels.find(strModelName) == m_mapModels.end())
	{
		/* model doesn't exist yet */
		if (GetModelCreator(strModelType, pfnModelCreator))
		{
			if (pfnModelCreator)
			{
				pModel = (*pfnModelCreator)(strModelName);
				if (pModel && pModel->Init(mapParams) == 0)
				{
					if (g_pSimulator->running())
						pModel->PreRun();
					iterModel = m_mapModels.insert(std::pair<QString, Model *>(strModelName, pModel)).first;
					return 1;
				}
				else
					errno = EINVAL;
			}
			else
			{
				m_mapModels.insert(std::pair<QString, Model *>(strModelName, NULL));
				return 1;
			}
		}
		else
			errno = EINVAL;
	}
	else
		errno = EEXIST;
	return 0;
}

int ModelMgr::GetModel(const QString & strModelName, Model * & pModel)
{
	std::map<QString, Model *>::iterator iterModel;
	int result = 0;
	m_modelsMutex.lock();
	iterModel = m_mapModels.find(strModelName);
	if (iterModel == m_mapModels.end())
		pModel = NULL;
	else
	{
		pModel = iterModel->second;
		result = 1;
	}
	m_modelsMutex.unlock();
	return result;
}

unsigned int ModelMgr::GetAllModels(ModelTreeNode * & pModelNodes)
{
	m_modelsMutex.lock();
	pModelNodes = m_pModelTreeNodes;
	return m_nModelTreeNodes;
}

void ModelMgr::ReleaseAllModels()
{
	m_modelsMutex.unlock();
}

int ModelMgr::RemoveModel(const QString & strModelName, Model * & pModel)
{
	std::map<QString, Model *>::iterator iterModel;
	int result = 0;
	m_modelsMutex.lock();
	iterModel = m_mapModels.find(strModelName);
	if (iterModel == m_mapModels.end())
		pModel = NULL;
	else
	{
		if (RemoveModelFromTree(strModelName))
		{
			pModel = iterModel->second;
			m_mapModels.erase(iterModel);
			result = 1;
		}
		else /* removing model would cause dependency errors */
			pModel = NULL;
	}
	m_modelsMutex.unlock();
	return result;
}

int ModelMgr::DestroyModel(const QString & strModelName)
{
	Model * pModel;
	if (RemoveModel(strModelName, pModel))
	{
		delete pModel;
		return 1;
	}
	else
		return 0;
}

void ModelMgr::DestroyAllModels()
{
	std::map<QString, Model *>::iterator iterModel;

	m_modelsMutex.lock();
	ClearModelTree();

	for (iterModel = m_mapModels.begin(); iterModel != m_mapModels.end(); ++iterModel)
	{
		if (iterModel->second)
		{
			iterModel->second->Cleanup();
			delete iterModel->second;
			iterModel->second = NULL;
		}
	}

	m_mapModels.clear();
	m_modelsMutex.unlock();
}

void ModelMgr::AddNetworkCars(const std::map<in_addr_t, in_addr_t> & mapNewCars)
{
	std::map<in_addr_t, in_addr_t>::const_iterator iterNewCar;
	std::map<in_addr_t, CarModel *> * pCarRegistry;
	std::map<in_addr_t, CarModel *>::iterator iterFoundCar;
	QString strName, strType;
	std::map<QString, QString> mapParams;
	std::vector<std::pair<QString, QString> > vecDepends;

	m_modelsMutex.lock();
	for (iterNewCar = mapNewCars.begin(); iterNewCar != mapNewCars.end(); ++iterNewCar)
	{
		pCarRegistry = g_pCarRegistry->acquireLock();
		iterFoundCar = pCarRegistry->find(iterNewCar->first);
		if (iterFoundCar == pCarRegistry->end())
		{
			strName = IPAddressToString(iterNewCar->first);
			strType = NETMODEL_NAME;

			mapParams[CARMODEL_PARAM_CARIP] = strName;

			if (AddModel(strName, strType, mapParams))
				vecDepends.push_back(std::pair<QString, QString>(strName, ""));
			iterFoundCar = pCarRegistry->find(iterNewCar->first);
			if (iterFoundCar != pCarRegistry->end() && iterFoundCar->second != NULL) {
				iterFoundCar->second->SetOwnerIPAddress(iterNewCar->second);
	
				g_pLogger->LogInfo(QString("Added vehicle [%1] with owner [%2]\n").arg(IPAddressToString(iterNewCar->first)).arg(IPAddressToString(iterNewCar->second)));
			}
		}

		g_pCarRegistry->releaseLock();
	}

	if (!vecDepends.empty())
		BuildModelTree(vecDepends);

	m_modelsMutex.unlock();
}

void ModelMgr::ClearModelTree()
{
	m_nModelTreeNodes = 0;
	if (m_pModelTreeNodes)
	{
		delete[] m_pModelTreeNodes;
		m_pModelTreeNodes = NULL;
	}
}

void ModelMgr::ClearAllModelsError()
{
	unsigned int i;
	for (i = 0; i < m_nModelTreeNodes; i++)
		m_pModelTreeNodes[i].iBits &= ~MODELTREEBITS_ERROR;
}

void ModelMgr::MarkAllModelsDirty()
{
	unsigned int i;
	for (i = 0; i < m_nModelTreeNodes; i++)
		m_pModelTreeNodes[i].iBits |= MODELTREEBITS_DIRTY;
}
/*
bool ModelMgr::MarkModelsDirty(struct timeval tCurrent)
{
	unsigned int i;
	bool bDirty = false;
	for (i = 0; i < m_nModelTreeNodes; i++)
	{
		// if deadline reached/missed, mark as dirty
		if (MODELTREENODE_ISVALID(m_pModelTreeNodes[i]) && m_pModelTreeNodes[i].pModel && m_pModelTreeNodes[i].pModel->GetLastIteration() + m_pModelTreeNodes[i].pModel->GetDelay() <= tCurrent) {
			m_pModelTreeNodes[i].iBits |= MODELTREEBITS_DIRTY;
			m_pModelTreeNodes[i].iBits &= ~MODELTREEBITS_ERROR;
			bDirty = true;
		}
	}
	return bDirty;
}
*/

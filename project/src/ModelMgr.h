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

#ifndef _MODELMGR_H
#define _MODELMGR_H

#include "Model.h"

#include <vector>
#include <list>
#include <qmutex.h>

#include <arpa/inet.h>

#define MODELTREEBITS_CLEAN 0
#define MODELTREEBITS_DIRTY 1
#define MODELTREEBITS_ERROR 2
#define MODELTREEBITS_FATAL 4
#define MODELTREEBITS_POSTITER (1 << 16)

typedef struct ModelTreeNodeStruct
{
	QString strModelName;
	Model * pModel;
	std::list<struct ModelTreeNodeStruct *> listDepends;
	int iBits;
} ModelTreeNode;

#define MODELTREENODE_ISDIRTY(node) ((node).iBits & MODELTREEBITS_DIRTY)
#define MODELTREENODE_ISCLEAN(node) (!MODELTREENODE_ISDIRTY(node))
#define MODELTREENODE_ISERROR(node) ((node).iBits & MODELTREEBITS_ERROR)
#define MODELTREENODE_ISVALID(node) (((node).iBits & MODELTREEBITS_FATAL) == 0)

class ModelMgr{
public:
	ModelMgr();
	~ModelMgr();

	int RegisterModel(ModelCreator pfnModelCreator, const QString & strModelName);
	int UnregisterModel(const QString & strModelName);
	int IsRegisteredModel(const QString & strModelName) const;
	bool IsModelTypeOf(const QString & strModelType, const QString & strTargetType) const;
	int GetModelCreator(const QString & strModelName, ModelCreator & pfnModelCreator) const;

	int AddModel(const QString & strModelName, const QString & strModelType, const QString & strDepends, const std::map<QString, QString> & mapParams);
	int GetModel(const QString & strModelName, Model * & pModel);
	unsigned int GetAllModels(ModelTreeNode * & pModelNodes);
	void ReleaseAllModels();
	int RemoveModel(const QString & strModelName, Model * & pModel);
	int DestroyModel(const QString & strModelName);
	void DestroyAllModels();

	void AddNetworkCars(const std::map<in_addr_t, in_addr_t> & mapNewCars);

protected:
	int AddModel(const QString & strModelName, const QString & strModelType, const std::map<QString, QString> & mapParams);
	int BuildModelTree(const std::vector<std::pair<QString, QString> > & vecDependencies);
	int RemoveModelFromTree(const QString & strModelName);
	void ClearModelTree();

	void ClearAllModelsError();
	void MarkAllModelsDirty();
//	bool MarkModelsDirty(struct timeval tCurrent);

	// model type registry
	std::map<QString, ModelCreator> m_mapModelRegistry;

	// model registry
	std::map<QString, Model *> m_mapModels;
	ModelTreeNode * m_pModelTreeNodes;
	unsigned int m_nModelTreeNodes;
	QMutex m_modelsMutex;

	friend class Simulator;
};

#endif

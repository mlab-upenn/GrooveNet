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

#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qtable.h>
#include <qsplitter.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qinputdialog.h>
#include <qapplication.h>
#include <qcursor.h>

#include "QSimCreateDialog.h"
#include "QAutoGenDialog.h"
#include "Simulator.h"
#include "StringHelp.h"
#include "SimpleLinkModel.h"
#include "SimplePhysModel.h"
#include "CollisionPhysModel.h"
#include "MultiPhysModel.h"
#include "SimpleCommModel.h"
#include "AdaptiveCommModel.h"
#include "GrooveCommModel.h"
#include "GPSModel.h"
#include "SimModel.h"
#include "TrafficLightModel.h"
#include "MapVisual.h"
#include "CarListVisual.h"
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

#include <qlayout.h>
#include <qmessagebox.h>

#include "app16x16.xpm"

QSimCreateDialog::QSimCreateDialog(QWidget * parent, const char * name, bool modal, WFlags f)
: QDialog(parent, name, modal, f)
{
	unsigned int i;
	std::map<QString, std::map<QString, ModelParameter> >::iterator iterModelParam;

	QWidget * pButtonBox = new QWidget(this);
	QWidget * pTabVehicles = new QWidget(this);
	QWidget * pTabInfrastructureNodes = new QWidget(this);
	QWidget * pTabModels = new QWidget(this);
	QSplitter * splitterVehicles = new QSplitter(Qt::Vertical, pTabVehicles);
	QWidget * pVehiclesBox = new QWidget(splitterVehicles);
	QWidget * pVehiclesButtonBox = new QWidget(pVehiclesBox);
	QWidget * pVehicleTypeBox = new QWidget(splitterVehicles);
	QWidget * pVehicleNameBox = new QWidget(pVehicleTypeBox);
	QWidget * pVehicleButtons = new QWidget(pTabVehicles);
	QSplitter * splitterInfrastructureNodes = new QSplitter(Qt::Vertical, pTabInfrastructureNodes);
	QWidget * pInfrastructureNodesBox = new QWidget(splitterInfrastructureNodes);
	QWidget * pInfrastructureNodesButtonBox = new QWidget(pInfrastructureNodesBox);
	QWidget * pInfrastructureNodeTypeBox = new QWidget(splitterInfrastructureNodes);
	QWidget * pInfrastructureNodeNameBox = new QWidget(pInfrastructureNodeTypeBox);
	QWidget * pInfrastructureNodeButtons = new QWidget(pTabInfrastructureNodes);
	QSplitter * splitterModels = new QSplitter(Qt::Vertical, pTabModels);
	QWidget * pModelsBox = new QWidget(splitterModels);
	QWidget * pModelsButtonBox = new QWidget(pModelsBox);
	QWidget * pModelTypeBox = new QWidget(splitterModels);
	QWidget * pModelNameBox = new QWidget(pModelTypeBox);
	QWidget * pModelButtons = new QWidget(pTabModels);

	QVBoxLayout * pLayout = new QVBoxLayout(this, 8, 8);
	QVBoxLayout * pTabVehiclesLayout = new QVBoxLayout(pTabVehicles, 8, 8);
	QVBoxLayout * pTabInfrastructureNodesLayout = new QVBoxLayout(pTabInfrastructureNodes, 8, 8);
	QVBoxLayout * pTabModelsLayout = new QVBoxLayout(pTabModels, 8, 8);
	QHBoxLayout * pVehiclesBoxLayout = new QHBoxLayout(pVehiclesBox, 0, 8);
	QVBoxLayout * pVehiclesButtonBoxLayout = new QVBoxLayout(pVehiclesButtonBox, 0, 8);
	QVBoxLayout * pVehicleTypeBoxLayout = new QVBoxLayout(pVehicleTypeBox, 0, 8);
	QHBoxLayout * pVehicleNameBoxLayout = new QHBoxLayout(pVehicleNameBox, 0, 8);
	QHBoxLayout * pVehicleButtonsLayout = new QHBoxLayout(pVehicleButtons, 0, 8);
	QHBoxLayout * pInfrastructureNodesBoxLayout = new QHBoxLayout(pInfrastructureNodesBox, 0, 8);
	QVBoxLayout * pInfrastructureNodesButtonBoxLayout = new QVBoxLayout(pInfrastructureNodesButtonBox, 0, 8);
	QVBoxLayout * pInfrastructureNodeTypeBoxLayout = new QVBoxLayout(pInfrastructureNodeTypeBox, 0, 8);
	QHBoxLayout * pInfrastructureNodeNameBoxLayout = new QHBoxLayout(pInfrastructureNodeNameBox, 0, 8);
	QHBoxLayout * pInfrastructureNodeButtonsLayout = new QHBoxLayout(pInfrastructureNodeButtons, 0, 8);
	QHBoxLayout * pModelsBoxLayout = new QHBoxLayout(pModelsBox, 0, 8);
	QVBoxLayout * pModelsButtonBoxLayout = new QVBoxLayout(pModelsButtonBox, 0, 8);
	QVBoxLayout * pModelTypeBoxLayout = new QVBoxLayout(pModelTypeBox, 0, 8);
	QHBoxLayout * pModelNameBoxLayout = new QHBoxLayout(pModelNameBox, 0, 8);
	QHBoxLayout * pModelButtonsLayout = new QHBoxLayout(pModelButtons, 0, 8);
	QHBoxLayout * pButtonBoxLayout = new QHBoxLayout(pButtonBox, 0, 8);

	SimpleLinkModel::GetParams(m_mapModelParams[SIMPLELINKMODEL_NAME]);
	m_vecModelTypes.push_back(SIMPLELINKMODEL_NAME);
	SimplePhysModel::GetParams(m_mapModelParams[SIMPLEPHYSMODEL_NAME]);
	m_vecModelTypes.push_back(SIMPLEPHYSMODEL_NAME);
	CollisionPhysModel::GetParams(m_mapModelParams[COLLISIONPHYSMODEL_NAME]);
	m_vecModelTypes.push_back(COLLISIONPHYSMODEL_NAME);
	MultiPhysModel::GetParams(m_mapModelParams[MULTIPHYSMODEL_NAME]);
	m_vecModelTypes.push_back(MULTIPHYSMODEL_NAME);
	SimpleCommModel::GetParams(m_mapModelParams[SIMPLECOMMMODEL_NAME]);
	m_vecModelTypes.push_back(SIMPLECOMMMODEL_NAME);
	AdaptiveCommModel::GetParams(m_mapModelParams[ADAPTIVECOMMMODEL_NAME]);
	m_vecModelTypes.push_back(ADAPTIVECOMMMODEL_NAME);
	GrooveCommModel::GetParams(m_mapModelParams[GROOVECOMMMODEL_NAME]);
	m_vecModelTypes.push_back(GROOVECOMMMODEL_NAME);
	GPSModel::GetParams(m_mapModelParams[GPSMODEL_NAME]);
	m_vecModelTypes.push_back(GPSMODEL_NAME);
	SimModel::GetParams(m_mapModelParams[SIMMODEL_NAME]);
	m_vecModelTypes.push_back(SIMMODEL_NAME);
	FixedMobilityModel::GetParams(m_mapModelParams[FIXEDMOBILITYMODEL_NAME]);
	m_vecModelTypes.push_back(FIXEDMOBILITYMODEL_NAME);
	StreetSpeedModel::GetParams(m_mapModelParams[STREETSPEEDMODEL_NAME]);
	m_vecModelTypes.push_back(STREETSPEEDMODEL_NAME);
	UniformSpeedModel::GetParams(m_mapModelParams[UNIFORMSPEEDMODEL_NAME]);
	m_vecModelTypes.push_back(UNIFORMSPEEDMODEL_NAME);
	CarFollowingModel::GetParams(m_mapModelParams[CARFOLLOWINGMODEL_NAME]);
	m_vecModelTypes.push_back(CARFOLLOWINGMODEL_NAME);
	RandomWalkModel::GetParams(m_mapModelParams[RANDOMWALKMODEL_NAME]);
	m_vecModelTypes.push_back(RANDOMWALKMODEL_NAME);
	DjikstraTripModel::GetParams(m_mapModelParams[DJIKSTRATRIPMODEL_NAME]);
	m_vecModelTypes.push_back(DJIKSTRATRIPMODEL_NAME);
	SightseeingModel::GetParams(m_mapModelParams[SIGHTSEEINGMODEL_NAME]);
	m_vecModelTypes.push_back(SIGHTSEEINGMODEL_NAME);
	SimUnconstrainedModel::GetParams(m_mapModelParams[SIMUNCONSTRAINEDMODEL_NAME]);
	m_vecModelTypes.push_back(SIMUNCONSTRAINEDMODEL_NAME);
	RandomWaypointModel::GetParams(m_mapModelParams[RANDOMWAYPOINTMODEL_NAME]);
	m_vecModelTypes.push_back(RANDOMWAYPOINTMODEL_NAME);
	InfrastructureNodeModel::GetParams(m_mapModelParams[INFRASTRUCTURENODEMODEL_NAME]);
	m_vecModelTypes.push_back(INFRASTRUCTURENODEMODEL_NAME);
	TrafficLightModel::GetParams(m_mapModelParams[TRAFFICLIGHTMODEL_NAME]);
	m_vecModelTypes.push_back(TRAFFICLIGHTMODEL_NAME);
	MapVisual::GetParams(m_mapModelParams[MAPVISUAL_NAME]);
	m_vecModelTypes.push_back(MAPVISUAL_NAME);
	CarListVisual::GetParams(m_mapModelParams[CARLISTVISUAL_NAME]);
	m_vecModelTypes.push_back(CARLISTVISUAL_NAME);

	for (iterModelParam = m_mapModelParams.begin(); iterModelParam != m_mapModelParams.end(); ++iterModelParam) {
		iterModelParam->second[PARAM_DEPENDS].strValue = "";
		iterModelParam->second[PARAM_DEPENDS].strDesc = "DEPENDS (models) -- A semicolon-delimited list of models upon which this model depends. This is important only for initialization and cleanup, since the event-driven simulator does not resolve dependencies.";
		iterModelParam->second[PARAM_DEPENDS].eType = ModelParameterTypeModels;
	}

	setCaption("GrooveNet - Create New Simulation...");
	setIcon(app16x16_xpm);
	setSizeGripEnabled(true);

	m_tabsTabs = new QTabWidget(this);
	m_listVehicles = new QListBox(pVehiclesBox);
	m_listInfrastructureNodes = new QListBox(pInfrastructureNodesBox);
	m_listModels = new QListBox(pModelsBox);
	m_buttonVehicleAdd = new QPushButton("Add", pVehiclesButtonBox);
	m_pVehiclesAddMenu = new QPopupMenu(this);
	m_buttonVehicleRemove = new QPushButton("Remove", pVehiclesButtonBox);
	m_buttonVehicleDuplicate = new QPushButton("Duplicate", pVehiclesButtonBox);
	m_buttonInfrastructureNodeAdd = new QPushButton("Add", pInfrastructureNodesButtonBox);
	m_pInfrastructureNodesAddMenu = new QPopupMenu(this);
	m_buttonInfrastructureNodeRemove = new QPushButton("Remove", pInfrastructureNodesButtonBox);
	m_buttonInfrastructureNodeDuplicate = new QPushButton("Duplicate", pInfrastructureNodesButtonBox);
	m_buttonModelAdd = new QPushButton("Add", pModelsButtonBox);
	m_pModelsAddMenu = new QPopupMenu(this);
	m_buttonModelRemove = new QPushButton("Remove", pModelsButtonBox);
	m_buttonModelDuplicate = new QPushButton("Duplicate", pModelsButtonBox);
	m_lblVehicleName = new QLabel("Model Name:", pVehicleNameBox);
	m_txtVehicleName = new QLineEdit(pVehicleNameBox);
	m_tableVehicleProperties = new QTable(0, 2, pVehicleTypeBox);
	m_txtVehiclePropertiesHelp = new QTextEdit("", QString::null, splitterVehicles);
	m_buttonVehicleAutoGen = new QPushButton("Auto-Generate", pVehicleButtons);
	m_buttonVehicleRemoveAll = new QPushButton("Remove All", pVehicleButtons);
	m_lblInfrastructureNodeName = new QLabel("Model Name:", pInfrastructureNodeNameBox);
	m_txtInfrastructureNodeName = new QLineEdit(pInfrastructureNodeNameBox);
	m_tableInfrastructureNodeProperties = new QTable(0, 2, pInfrastructureNodeTypeBox);
	m_txtInfrastructureNodePropertiesHelp = new QTextEdit("", QString::null, splitterInfrastructureNodes);
	m_buttonInfrastructureNodeAutoGen = new QPushButton("Auto-Generate", pInfrastructureNodeButtons);
	m_buttonInfrastructureNodeRemoveAll = new QPushButton("Remove All", pInfrastructureNodeButtons);
	m_lblModelName = new QLabel("Model Name:", pModelNameBox);
	m_txtModelName = new QLineEdit(pModelNameBox);
	m_tableModelProperties = new QTable(0, 2, pModelTypeBox);
	m_txtModelPropertiesHelp = new QTextEdit("", QString::null, splitterModels);
	m_buttonModelRemoveAll = new QPushButton("Remove All", pModelButtons);
	m_buttonOK = new QPushButton("&OK", pButtonBox);
	m_buttonCancel = new QPushButton("&Cancel", pButtonBox);

	connect(m_listVehicles, SIGNAL(highlighted(int)), this, SLOT(slotVehiclesListHighlighted(int)));
	connect(m_listVehicles, SIGNAL(selectionChanged()), this, SLOT(slotVehiclesListSelChanged()));
	connect(m_pVehiclesAddMenu, SIGNAL(activated(int)), this, SLOT(slotVehiclesAdd(int)));
	connect(m_buttonVehicleRemove, SIGNAL(clicked()), this, SLOT(slotVehiclesRemove()));
	connect(m_buttonVehicleDuplicate, SIGNAL(clicked()), this, SLOT(slotVehiclesDuplicate()));
	connect(m_txtVehicleName, SIGNAL(textChanged(const QString& )), this, SLOT(slotVehicleNameChanged(const QString& )));
	connect(m_tableVehicleProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotVehiclePropertiesValueChanged(int, int )));
	connect(m_tableVehicleProperties, SIGNAL(currentChanged(int, int)), this, SLOT(slotVehiclePropertiesCurrentChanged(int, int )));
	connect(m_buttonVehicleAutoGen, SIGNAL(clicked()), this, SLOT(slotVehiclesAutoGen()));
	connect(m_buttonVehicleRemoveAll, SIGNAL(clicked()), this, SLOT(slotVehiclesRemoveAll()));
	connect(m_listInfrastructureNodes, SIGNAL(highlighted(int)), this, SLOT(slotInfrastructureNodesListHighlighted(int)));
	connect(m_listInfrastructureNodes, SIGNAL(selectionChanged()), this, SLOT(slotInfrastructureNodesListSelChanged()));
	connect(m_pInfrastructureNodesAddMenu, SIGNAL(activated(int)), this, SLOT(slotInfrastructureNodesAdd(int)));
	connect(m_buttonInfrastructureNodeRemove, SIGNAL(clicked()), this, SLOT(slotInfrastructureNodesRemove()));
	connect(m_buttonInfrastructureNodeDuplicate, SIGNAL(clicked()), this, SLOT(slotInfrastructureNodesDuplicate()));
	connect(m_txtInfrastructureNodeName, SIGNAL(textChanged(const QString& )), this, SLOT(slotInfrastructureNodeNameChanged(const QString& )));
	connect(m_tableInfrastructureNodeProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotInfrastructureNodePropertiesValueChanged(int, int )));
	connect(m_tableInfrastructureNodeProperties, SIGNAL(currentChanged(int, int)), this, SLOT(slotInfrastructureNodePropertiesCurrentChanged(int, int )));
	connect(m_buttonInfrastructureNodeAutoGen, SIGNAL(clicked()), this, SLOT(slotInfrastructureNodesAutoGen()));
	connect(m_buttonInfrastructureNodeRemoveAll, SIGNAL(clicked()), this, SLOT(slotInfrastructureNodesRemoveAll()));
	connect(m_listModels, SIGNAL(highlighted(int)), this, SLOT(slotModelsListHighlighted(int)));
	connect(m_listModels, SIGNAL(selectionChanged()), this, SLOT(slotModelsListSelChanged()));
	connect(m_pModelsAddMenu, SIGNAL(activated(int)), this, SLOT(slotModelsAdd(int)));
	connect(m_buttonModelRemove, SIGNAL(clicked()), this, SLOT(slotModelsRemove()));
	connect(m_buttonModelDuplicate, SIGNAL(clicked()), this, SLOT(slotModelsDuplicate()));
	connect(m_txtModelName, SIGNAL(textChanged(const QString& )), this, SLOT(slotModelNameChanged(const QString& )));
	connect(m_tableModelProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotModelPropertiesValueChanged(int, int )));
	connect(m_tableModelProperties, SIGNAL(currentChanged(int, int)), this, SLOT(slotModelPropertiesCurrentChanged(int, int )));
	connect(m_buttonModelRemoveAll, SIGNAL(clicked()), this, SLOT(slotModelsRemoveAll()));
	connect(m_buttonOK, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
	m_listVehicles->setSelectionMode(QListBox::Extended);
	m_listInfrastructureNodes->setSelectionMode(QListBox::Extended);
	m_listModels->setSelectionMode(QListBox::Extended);
	m_listVehicles->clearSelection();
	m_listInfrastructureNodes->clearSelection();
	m_listModels->clearSelection();
	for (i = 0; i < m_vecModelTypes.size(); i++)
	{
		if (g_pSimulator->m_ModelMgr.IsModelTypeOf(m_vecModelTypes[i], CARMODEL_NAME))
			m_pVehiclesAddMenu->insertItem(m_vecModelTypes[i], i);
		else if (g_pSimulator->m_ModelMgr.IsModelTypeOf(m_vecModelTypes[i], INFRASTRUCTURENODEMODEL_NAME))
			m_pInfrastructureNodesAddMenu->insertItem(m_vecModelTypes[i], i);
		else
			m_pModelsAddMenu->insertItem(m_vecModelTypes[i], i);
	}
	m_buttonVehicleAdd->setPopup(m_pVehiclesAddMenu);
	m_buttonInfrastructureNodeAdd->setPopup(m_pInfrastructureNodesAddMenu);
	m_buttonModelAdd->setPopup(m_pModelsAddMenu);
	m_buttonOK->setDefault(true);
	m_tabsTabs->addTab(pTabVehicles, "&Vehicles");
	m_tabsTabs->addTab(pTabInfrastructureNodes, "&Infrastructure Nodes");
	m_tabsTabs->addTab(pTabModels, "Other &Models");
	m_tableVehicleProperties->verticalHeader()->hide();
	m_tableVehicleProperties->setLeftMargin(0);
	m_tableVehicleProperties->horizontalHeader()->setClickEnabled(false);
	m_tableVehicleProperties->horizontalHeader()->setMovingEnabled(false);
	m_tableVehicleProperties->horizontalHeader()->setLabel(0, "Parameter");
	m_tableVehicleProperties->horizontalHeader()->setLabel(1, "Value");
	m_tableVehicleProperties->setSorting(false);
	m_tableVehicleProperties->setSelectionMode(QTable::SingleRow);
	m_tableVehicleProperties->setFocusStyle(QTable::FollowStyle);
	m_tableVehicleProperties->setColumnReadOnly(0, true);
	m_tableVehicleProperties->setColumnReadOnly(1, false);
	m_tableVehicleProperties->setColumnStretchable(1, true);
	m_txtVehiclePropertiesHelp->setReadOnly(true);
	m_tableInfrastructureNodeProperties->verticalHeader()->hide();
	m_tableInfrastructureNodeProperties->setLeftMargin(0);
	m_tableInfrastructureNodeProperties->horizontalHeader()->setClickEnabled(false);
	m_tableInfrastructureNodeProperties->horizontalHeader()->setMovingEnabled(false);
	m_tableInfrastructureNodeProperties->horizontalHeader()->setLabel(0, "Parameter");
	m_tableInfrastructureNodeProperties->horizontalHeader()->setLabel(1, "Value");
	m_tableInfrastructureNodeProperties->setSorting(false);
	m_tableInfrastructureNodeProperties->setSelectionMode(QTable::SingleRow);
	m_tableInfrastructureNodeProperties->setFocusStyle(QTable::FollowStyle);
	m_tableInfrastructureNodeProperties->setColumnReadOnly(0, true);
	m_tableInfrastructureNodeProperties->setColumnReadOnly(1, false);
	m_tableInfrastructureNodeProperties->setColumnStretchable(1, true);
	m_txtInfrastructureNodePropertiesHelp->setReadOnly(true);
	m_tableModelProperties->verticalHeader()->hide();
	m_tableModelProperties->setLeftMargin(0);
	m_tableModelProperties->horizontalHeader()->setClickEnabled(false);
	m_tableModelProperties->horizontalHeader()->setMovingEnabled(false);
	m_tableModelProperties->horizontalHeader()->setLabel(0, "Parameter");
	m_tableModelProperties->horizontalHeader()->setLabel(1, "Value");
	m_tableModelProperties->setSorting(false);
	m_tableModelProperties->setSelectionMode(QTable::SingleRow);
	m_tableModelProperties->setFocusStyle(QTable::FollowStyle);
	m_tableModelProperties->setColumnReadOnly(0, true);
	m_tableModelProperties->setColumnReadOnly(1, false);
	m_tableModelProperties->setColumnStretchable(1, true);
	m_txtModelPropertiesHelp->setReadOnly(true);
	m_buttonOK->setAutoDefault(false);
	m_buttonCancel->setAutoDefault(false);
	m_buttonVehicleRemoveAll->setEnabled(m_listVehicles->count() > 0);
	m_buttonInfrastructureNodeRemoveAll->setEnabled(m_listInfrastructureNodes->count() > 0);
	m_buttonModelRemoveAll->setEnabled(m_listModels->count() > 0);
	m_listVehicles->setCurrentItem(m_listVehicles->count() > 0 ? 0 : -1);
	m_listInfrastructureNodes->setCurrentItem(m_listInfrastructureNodes->count() > 0 ? 0 : -1);
	m_listModels->setCurrentItem(m_listModels->count() > 0 ? 0 : -1);
	slotVehiclesListHighlighted(m_listVehicles->currentItem());
	slotVehiclesListSelChanged();
	slotInfrastructureNodesListHighlighted(m_listInfrastructureNodes->currentItem());
	slotInfrastructureNodesListSelChanged();
	slotModelsListHighlighted(m_listModels->currentItem());
	slotModelsListSelChanged();

	pLayout->addWidget(m_tabsTabs, 1);
	pLayout->addWidget(pButtonBox);
	pTabVehiclesLayout->addWidget(splitterVehicles, 1);
	pTabVehiclesLayout->addWidget(pVehicleButtons);
	pVehiclesBoxLayout->addWidget(m_listVehicles, 1);
	pVehiclesBoxLayout->addWidget(pVehiclesButtonBox);
	pVehiclesButtonBoxLayout->addWidget(m_buttonVehicleAdd, 0, Qt::AlignCenter);
	pVehiclesButtonBoxLayout->addWidget(m_buttonVehicleRemove, 0, Qt::AlignCenter);
	pVehiclesButtonBoxLayout->addWidget(m_buttonVehicleDuplicate, 0, Qt::AlignCenter);
	pVehiclesButtonBoxLayout->addStretch(1);
	pVehicleTypeBoxLayout->addWidget(pVehicleNameBox);
	pVehicleTypeBoxLayout->addWidget(m_tableVehicleProperties, 1);
//	pVehicleTypeBoxLayout->addWidget(m_txtVehiclePropertiesHelp, 0);
	pVehicleNameBoxLayout->addWidget(m_lblVehicleName);
	pVehicleNameBoxLayout->addWidget(m_txtVehicleName, 1);
	pVehicleButtonsLayout->addWidget(m_buttonVehicleAutoGen, 0, Qt::AlignCenter);
	pVehicleButtonsLayout->addWidget(m_buttonVehicleRemoveAll, 0, Qt::AlignCenter);
	pTabInfrastructureNodesLayout->addWidget(splitterInfrastructureNodes, 1);
	pTabInfrastructureNodesLayout->addWidget(pInfrastructureNodeButtons);
	pInfrastructureNodesBoxLayout->addWidget(m_listInfrastructureNodes, 1);
	pInfrastructureNodesBoxLayout->addWidget(pInfrastructureNodesButtonBox);
	pInfrastructureNodesButtonBoxLayout->addWidget(m_buttonInfrastructureNodeAdd, 0, Qt::AlignCenter);
	pInfrastructureNodesButtonBoxLayout->addWidget(m_buttonInfrastructureNodeRemove, 0, Qt::AlignCenter);
	pInfrastructureNodesButtonBoxLayout->addWidget(m_buttonInfrastructureNodeDuplicate, 0, Qt::AlignCenter);
	pInfrastructureNodesButtonBoxLayout->addStretch(1);
	pInfrastructureNodeTypeBoxLayout->addWidget(pInfrastructureNodeNameBox);
	pInfrastructureNodeTypeBoxLayout->addWidget(m_tableInfrastructureNodeProperties, 1);
//	pInfrastructureNodeTypeBoxLayout->addWidget(m_txtInfrastructureNodePropertiesHelp, 0);
	pInfrastructureNodeNameBoxLayout->addWidget(m_lblInfrastructureNodeName);
	pInfrastructureNodeNameBoxLayout->addWidget(m_txtInfrastructureNodeName, 1);
	pInfrastructureNodeButtonsLayout->addWidget(m_buttonInfrastructureNodeAutoGen, 0, Qt::AlignCenter);
	pInfrastructureNodeButtonsLayout->addWidget(m_buttonInfrastructureNodeRemoveAll, 0, Qt::AlignCenter);
	pTabModelsLayout->addWidget(splitterModels, 1);
	pTabModelsLayout->addWidget(pModelButtons);
	pModelsBoxLayout->addWidget(m_listModels, 1);
	pModelsBoxLayout->addWidget(pModelsButtonBox);
	pModelsButtonBoxLayout->addWidget(m_buttonModelAdd, 0, Qt::AlignCenter);
	pModelsButtonBoxLayout->addWidget(m_buttonModelRemove, 0, Qt::AlignCenter);
	pModelsButtonBoxLayout->addWidget(m_buttonModelDuplicate, 0, Qt::AlignCenter);
	pModelsButtonBoxLayout->addStretch(1);
	pModelTypeBoxLayout->addWidget(pModelNameBox);
	pModelTypeBoxLayout->addWidget(m_tableModelProperties, 1);
//	pModelTypeBoxLayout->addWidget(m_txtModelPropertiesHelp, 0);
	pModelNameBoxLayout->addWidget(m_lblModelName);
	pModelNameBoxLayout->addWidget(m_txtModelName, 1);
	pModelButtonsLayout->addWidget(m_buttonModelRemoveAll, 0, Qt::AlignCenter);
	pButtonBoxLayout->addStretch(1);
	pButtonBoxLayout->addWidget(m_buttonOK);
	pButtonBoxLayout->addWidget(m_buttonCancel);
}

QSimCreateDialog::~QSimCreateDialog()
{
}

void QSimCreateDialog::slotVehiclesListHighlighted(int index)
{
	m_lblVehicleName->setEnabled(index > -1);
	m_txtVehicleName->setEnabled(index > -1);
	disconnect(m_txtVehicleName, SIGNAL(textChanged(const QString& )), this, SLOT(slotVehicleNameChanged(const QString& )));
	disconnect(m_tableVehicleProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotVehiclePropertiesValueChanged(int, int )));
	if (index > -1)
	{
		int iRow;
		std::map<QString, std::map<QString, QString> >::iterator iterModel = m_mapModels.find(m_vecVehicles[index].first);
		std::map<QString, std::map<QString, ModelParameter> >::iterator iterParams = m_mapModelParams.find(m_vecVehicles[index].second);
		std::map<QString, ModelParameter>::iterator iterParam;
		m_txtVehicleName->setText(m_vecVehicles[index].first);
		if (iterModel != m_mapModels.end())
		{
			std::map<QString, QString>::iterator iterValue;
			m_tableVehicleProperties->setNumRows(0);
			m_tableVehicleProperties->setNumRows(iterModel->second.size());
			for (iterValue = iterModel->second.begin(), iRow = 0; iterValue != iterModel->second.end(); ++iterValue, iRow++)
			{
				iterParam = iterParams->second.find(iterValue->first);
				m_tableVehicleProperties->setText(iRow, 0, iterValue->first);
				if (iterParam != iterParams->second.end())
					AddTableItem(m_tableVehicleProperties, iRow, 1, iterValue->second, &iterParam->second);
			}
			m_tableVehicleProperties->sortColumn(0, true, true);
		}
		else
			m_tableVehicleProperties->setNumRows(0);
	}
	else {
		m_txtVehicleName->setText("");
		m_tableVehicleProperties->setNumRows(0);
	}
	slotVehiclePropertiesCurrentChanged(-1, -1);
	connect(m_tableVehicleProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotVehiclePropertiesValueChanged(int, int )));
	connect(m_txtVehicleName, SIGNAL(textChanged(const QString& )), this, SLOT(slotVehicleNameChanged(const QString& )));
	m_tableVehicleProperties->adjustColumn(0);
}

void QSimCreateDialog::slotVehiclesListSelChanged()
{
	bool bSelection = false;
	unsigned int i;
	for (i = 0; i < m_listVehicles->count(); i++)
	{
		if (m_listVehicles->isSelected(i))
		{
			bSelection = true;
			break;
		}
	}
	m_buttonVehicleRemove->setEnabled(bSelection);
	m_buttonVehicleDuplicate->setEnabled(bSelection);
}

void QSimCreateDialog::slotVehiclesAdd(int id)
{
	if (id > -1)
	{
		int index = m_listVehicles->currentItem() + 1;
		QString strVehicleType = m_vecModelTypes[id], strName = GetNextModelName(strVehicleType);
		in_addr_t ipAddress;
		std::vector<std::pair<QString, QString> >::iterator iterIndex = m_vecVehicles.begin();
		std::map<QString, std::map<QString, QString> >::iterator iterModel = m_mapModels.insert(std::pair<QString, std::map<QString, QString> >(strName, std::map<QString, QString>())).first;
		std::map<QString, std::map<QString, ModelParameter> >::iterator iterParams = m_mapModelParams.find(strVehicleType);
		std::map<QString, ModelParameter>::iterator iterParam;
		for (iterParam = iterParams->second.begin(); iterParam != iterParams->second.end(); ++iterParam)
			iterModel->second[iterParam->first] = iterParam->second.strValue;
		if (index >= (signed)m_vecVehicles.size())
			iterIndex = m_vecVehicles.end();
		else
			iterIndex += index;
		iterIndex = m_vecVehicles.insert(iterIndex, std::pair<QString, QString>(strName, strVehicleType));
		if (index > 0 && StringToIPAddress(m_mapModels[m_vecVehicles[index-1].first][CARMODEL_PARAM_CARIP], ipAddress))
			ipAddress = GetNextIPAddress(ipAddress);
		else
			ipAddress = GetNextIPAddress(GetIPAddress());
		iterModel->second[CARMODEL_PARAM_CARIP] = IPAddressToString(ipAddress);
		m_listVehicles->insertItem(QString("[%1] %2 (%3)").arg(iterIndex->second).arg(iterIndex->first).arg(iterModel->second[CARMODEL_PARAM_CARIP]), index);
		m_setModelNames.insert(strName);
		m_setVehicleIPs.insert(ipAddress);
		m_buttonVehicleRemoveAll->setEnabled(m_listVehicles->count() > 0);
		m_listVehicles->setCurrentItem(index);
		slotVehiclesListHighlighted(m_listVehicles->currentItem());
	}
}

void QSimCreateDialog::slotVehiclesRemove()
{
	unsigned int index = 0;
	std::set<QString> setAssocModels;
	while (index < m_listVehicles->count())
	{
		if (m_listVehicles->isSelected(index))
		{
			in_addr_t ipAddress;
			std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(m_vecVehicles[index].first);
			std::map<QString, std::map<QString, ModelParameter> >::iterator iterModelParams = m_mapModelParams.find(m_vecVehicles[index].second);
			std::map<QString, QString>::iterator iterParam;
			std::map<QString, ModelParameter>::iterator iterModelParam;
			std::vector<std::pair<QString, QString> >::iterator iterName = m_vecVehicles.begin();
			iterName += index;

			for (iterParam = iterValues->second.begin(); iterParam != iterValues->second.end(); ++iterParam)
			{
				iterModelParam = iterModelParams->second.find(iterParam->first);
				if (iterModelParam != iterModelParams->second.end() && (iterModelParam->second.eType & 0xF) == ModelParameterTypeModel)
					setAssocModels.insert(iterParam->second);
			}
			m_setModelNames.erase(iterValues->first);
			if (StringToIPAddress(iterValues->second[CARMODEL_PARAM_CARIP], ipAddress))
				m_setVehicleIPs.erase(ipAddress);
			m_mapModels.erase(iterValues);
			m_vecVehicles.erase(iterName);
			m_listVehicles->removeItem(index);
		} else
			index++;
	}
	RemoveAssocModels(setAssocModels);

	m_buttonVehicleRemoveAll->setEnabled(m_listVehicles->count() > 0);
}

void QSimCreateDialog::slotVehiclesDuplicate()
{
	unsigned int index;
	for (index = 0; index < m_listVehicles->count(); index++)
	{
		if (m_listVehicles->isSelected(index))
		{
			in_addr_t ipAddress;
			std::vector<std::pair<QString, QString> >::iterator iterName = m_vecVehicles.begin(), iterNextName;
			QString strName = GetNextModelName(iterName->second);
			std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(m_vecVehicles[index].first), iterNextValues;
			iterName += index;

			if (StringToIPAddress(iterValues->second[CARMODEL_PARAM_CARIP], ipAddress))
				ipAddress = GetNextIPAddress(ipAddress);
			else
				ipAddress = GetNextIPAddress(GetIPAddress());
			iterNextName = m_vecVehicles.insert(m_vecVehicles.end(), std::pair<QString, QString>(strName, iterName->second));
			iterNextValues = m_mapModels.insert(std::pair<QString, std::map<QString, QString> >(iterNextName->first, iterValues->second)).first;
			iterNextValues->second[CARMODEL_PARAM_CARIP] = IPAddressToString(ipAddress);
			m_listVehicles->insertItem(QString("[%1] %2 (%3)").arg(iterNextName->second).arg(iterNextName->first).arg(iterNextValues->second[CARMODEL_PARAM_CARIP]));
			m_setModelNames.insert(strName);
			m_setVehicleIPs.insert(ipAddress);
		}
	}
	m_buttonVehicleRemoveAll->setEnabled(m_listVehicles->count() > 0);
}

void QSimCreateDialog::slotVehicleNameChanged(const QString & strName)
{
	int index = m_listVehicles->currentItem();
	int iCursor = m_txtVehicleName->cursorPosition();
	disconnect(m_listVehicles, SIGNAL(highlighted(int)), this, SLOT(slotVehiclesListHighlighted(int)));
	if (index > -1)
	{
		bool bSelected = m_listVehicles->isSelected(index);
		QString strNameOld = m_vecVehicles[index].first;
		std::vector<std::pair<QString, QString> >::iterator iterName = m_vecVehicles.begin();
		std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(strNameOld), iterNewValues;

		iterName += index;
		if (m_setModelNames.insert(strName).second)
		{
			m_setModelNames.erase(strNameOld);
			iterNewValues = m_mapModels.insert(std::pair<QString, std::map<QString, QString> >(strName, iterValues->second)).first;
			m_mapModels.erase(iterValues);
			m_vecVehicles[index].first = strName;
			m_listVehicles->changeItem(QString("[%1] %2 (%3)").arg(m_vecVehicles[index].second).arg(strName).arg(iterNewValues->second[CARMODEL_PARAM_CARIP]), index);
		}
		m_listVehicles->setSelected(index, bSelected);
	}
	connect(m_listVehicles, SIGNAL(highlighted(int)), this, SLOT(slotVehiclesListHighlighted(int)));
	m_txtVehicleName->setCursorPosition(iCursor);
}

void QSimCreateDialog::slotVehiclePropertiesValueChanged(int row, int col)
{
	int index = m_listVehicles->currentItem();
	disconnect(m_listVehicles, SIGNAL(highlighted(int)), this, SLOT(slotVehiclesListHighlighted(int)));
	if (index > -1 && row > -1 && col == 1)
	{
		bool bSelected = m_listVehicles->isSelected(index);
		QString strParam = m_tableVehicleProperties->text(row, 0);
		QString strValue = m_tableVehicleProperties->text(row, col);
		std::vector<std::pair<QString, QString> >::iterator iterName = m_vecVehicles.begin();
		std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(m_vecVehicles[index].first);
		std::map<QString, ModelParameter>::iterator iterParam = m_mapModelParams[m_vecVehicles[index].second].find(strParam);

		iterName += index;
		if (iterParam == m_mapModelParams[m_vecVehicles[index].second].end() || ValidateTableItem(strValue, &iterParam->second))
		{
			if (strParam.compare(CARMODEL_PARAM_CARIP) == 0) // need to do additional uniqueness check
			{
				in_addr_t ipOld, ipAddress;
				if (StringToIPAddress(strValue, ipAddress) && m_setVehicleIPs.insert(ipAddress).second)
				{
					if (StringToIPAddress(iterValues->second[CARMODEL_PARAM_CARIP], ipOld))
						m_setVehicleIPs.erase(ipOld);
					iterValues->second[strParam] = strValue;
					m_listVehicles->changeItem(QString("[%1] %2 (%3)").arg(iterName->second).arg(iterName->first).arg(strValue), index);
				} else
					AddTableItem(m_tableVehicleProperties, row, col, iterValues->second[strParam], &iterParam->second);
			}
			else
				iterValues->second[strParam] = strValue;
		}
		else
			AddTableItem(m_tableVehicleProperties, row, col, iterValues->second[strParam], &iterParam->second);
		m_listVehicles->setSelected(index, bSelected);
	}
	connect(m_listVehicles, SIGNAL(highlighted(int)), this, SLOT(slotVehiclesListHighlighted(int)));
}

void QSimCreateDialog::slotVehiclePropertiesCurrentChanged(int row, int col)
{
	int index = m_listVehicles->currentItem();
	if (index > -1 && row > -1)
	{
		QString strParam = m_tableVehicleProperties->text(row, 0);
		std::map<QString, ModelParameter>::iterator iterParam = m_mapModelParams[m_vecVehicles[index].second].find(strParam);
		if (iterParam != m_mapModelParams[m_vecVehicles[index].second].end())
			m_txtVehiclePropertiesHelp->setText(iterParam->second.strDesc);
		else
			m_txtVehiclePropertiesHelp->setText("");
	}
	else
		m_txtVehiclePropertiesHelp->setText("");
}

void QSimCreateDialog::slotVehiclesAutoGen()
{
	QAutoGenDialog * pDialog = new QAutoGenDialog(this);
	pDialog->SetParams(&m_mapModelParams, &m_vecModelTypes, CARMODEL_NAME);
	pDialog->m_pModels = &m_mapModels;
	pDialog->m_pVehicleList = &m_vecVehicles;
	pDialog->m_pModelList = &m_vecModels;
	pDialog->m_pVehicleIPs = &m_setVehicleIPs;
	pDialog->m_pModelNames = &m_setModelNames;
	pDialog->m_strIPAddressParam = CARMODEL_PARAM_CARIP;

	if (pDialog->exec() == QDialog::Accepted)
	{
		std::map<QString, std::map<QString, QString> >::iterator iterModel;
		unsigned int i;
		m_listVehicles->clear();
		m_listModels->clear();
		for (i = 0; i < m_vecModels.size(); i++)
		{
			m_listModels->insertItem(QString("[%1] %2").arg(m_vecModels[i].second).arg(m_vecModels[i].first));
		}
		for (i = 0; i < m_vecVehicles.size(); i++)
		{
			iterModel = m_mapModels.find(m_vecVehicles[i].first);
			m_listVehicles->insertItem(QString("[%1] %2 (%3)").arg(m_vecVehicles[i].second).arg(m_vecVehicles[i].first).arg(iterModel->second[CARMODEL_PARAM_CARIP]));
		}
		m_buttonVehicleRemoveAll->setEnabled(m_listVehicles->count() > 0);
		m_buttonModelRemoveAll->setEnabled(m_listModels->count() > 0);
		m_listVehicles->setCurrentItem(m_listVehicles->count() > 0 ? 0 : -1);
		m_listModels->setCurrentItem(m_listModels->count() > 0 ? 0 : -1);
		slotVehiclesListHighlighted(m_listVehicles->currentItem());
		slotVehiclesListSelChanged();
		slotModelsListHighlighted(m_listModels->currentItem());
		slotModelsListSelChanged();
	}
	delete pDialog;
}

void QSimCreateDialog::slotVehiclesRemoveAll()
{
	unsigned int i;
	in_addr_t ipAddress;
	std::map<QString, std::map<QString, QString> >::iterator iterValues;
	std::set<QString> setAssocModels;

	for (i = 0; i < m_vecVehicles.size(); i++)
	{
		std::map<QString, std::map<QString, ModelParameter> >::iterator iterModelParams = m_mapModelParams.find(m_vecVehicles[i].second);
		std::map<QString, QString>::iterator iterParam;
		std::map<QString, ModelParameter>::iterator iterModelParam;
		iterValues = m_mapModels.find(m_vecVehicles[i].first);

		for (iterParam = iterValues->second.begin(); iterParam != iterValues->second.end(); ++iterParam)
		{
			iterModelParam = iterModelParams->second.find(iterParam->first);
			if (iterModelParam != iterModelParams->second.end() && (iterModelParam->second.eType & 0xF) == ModelParameterTypeModel)
				setAssocModels.insert(iterParam->second);
		}
		m_setModelNames.erase(iterValues->first);
		if (StringToIPAddress(iterValues->second[CARMODEL_PARAM_CARIP], ipAddress))
			m_setVehicleIPs.erase(ipAddress);
		m_mapModels.erase(iterValues);
	}
	m_listVehicles->clear();
	m_vecVehicles.clear();
	m_buttonVehicleRemoveAll->setEnabled(m_listVehicles->count() > 0);
	m_listVehicles->setCurrentItem(-1);
	slotVehiclesListHighlighted(m_listVehicles->currentItem());
	slotVehiclesListSelChanged();

	RemoveAssocModels(setAssocModels);
}

void QSimCreateDialog::slotInfrastructureNodesListHighlighted(int index)
{
	m_lblInfrastructureNodeName->setEnabled(index > -1);
	m_txtInfrastructureNodeName->setEnabled(index > -1);
	disconnect(m_txtInfrastructureNodeName, SIGNAL(textChanged(const QString& )), this, SLOT(slotInfrastructureNodeNameChanged(const QString& )));
	disconnect(m_tableInfrastructureNodeProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotInfrastructureNodePropertiesValueChanged(int, int )));
	if (index > -1)
	{
		int iRow;
		std::map<QString, std::map<QString, QString> >::iterator iterModel = m_mapModels.find(m_vecInfrastructureNodes[index].first);
		std::map<QString, std::map<QString, ModelParameter> >::iterator iterParams = m_mapModelParams.find(m_vecInfrastructureNodes[index].second);
		std::map<QString, ModelParameter>::iterator iterParam;
		m_txtInfrastructureNodeName->setText(m_vecInfrastructureNodes[index].first);
		if (iterModel != m_mapModels.end())
		{
			std::map<QString, QString>::iterator iterValue;
			m_tableInfrastructureNodeProperties->setNumRows(0);
			m_tableInfrastructureNodeProperties->setNumRows(iterModel->second.size());
			for (iterValue = iterModel->second.begin(), iRow = 0; iterValue != iterModel->second.end(); ++iterValue, iRow++)
			{
				iterParam = iterParams->second.find(iterValue->first);
				m_tableInfrastructureNodeProperties->setText(iRow, 0, iterValue->first);
				if (iterParam != iterParams->second.end())
					AddTableItem(m_tableInfrastructureNodeProperties, iRow, 1, iterValue->second, &iterParam->second);
			}
			m_tableInfrastructureNodeProperties->sortColumn(0, true, true);
		} else
			m_tableInfrastructureNodeProperties->setNumRows(0);
	}
	else {
		m_txtInfrastructureNodeName->setText("");
		m_tableInfrastructureNodeProperties->setNumRows(0);
	}
	slotInfrastructureNodePropertiesCurrentChanged(-1, -1);
	connect(m_tableInfrastructureNodeProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotInfrastructureNodePropertiesValueChanged(int, int )));
	connect(m_txtInfrastructureNodeName, SIGNAL(textChanged(const QString& )), this, SLOT(slotInfrastructureNodeNameChanged(const QString& )));
	m_tableInfrastructureNodeProperties->adjustColumn(0);
}

void QSimCreateDialog::slotInfrastructureNodesListSelChanged()
{
	bool bSelection = false;
	unsigned int i;
	for (i = 0; i < m_listInfrastructureNodes->count(); i++)
	{
		if (m_listInfrastructureNodes->isSelected(i))
		{
			bSelection = true;
			break;
		}
	}
	m_buttonInfrastructureNodeRemove->setEnabled(bSelection);
	m_buttonInfrastructureNodeDuplicate->setEnabled(bSelection);
}

void QSimCreateDialog::slotInfrastructureNodesAdd(int id)
{
	if (id > -1)
	{
		int index = m_listInfrastructureNodes->currentItem() + 1;
		QString strInfrastructureNodeType = m_vecModelTypes[id], strName = GetNextModelName(strInfrastructureNodeType);
		in_addr_t ipAddress;
		std::vector<std::pair<QString, QString> >::iterator iterIndex = m_vecInfrastructureNodes.begin();
		std::map<QString, std::map<QString, QString> >::iterator iterModel = m_mapModels.insert(std::pair<QString, std::map<QString, QString> >(strName, std::map<QString, QString>())).first;
		std::map<QString, std::map<QString, ModelParameter> >::iterator iterParams = m_mapModelParams.find(strInfrastructureNodeType);
		std::map<QString, ModelParameter>::iterator iterParam;
		for (iterParam = iterParams->second.begin(); iterParam != iterParams->second.end(); ++iterParam)
			iterModel->second[iterParam->first] = iterParam->second.strValue;
		if (index >= (signed)m_vecInfrastructureNodes.size())
			iterIndex = m_vecInfrastructureNodes.end();
		else
			iterIndex += index;
		iterIndex = m_vecInfrastructureNodes.insert(iterIndex, std::pair<QString, QString>(strName, strInfrastructureNodeType));
		if (index > 0 && StringToIPAddress(m_mapModels[m_vecInfrastructureNodes[index-1].first][INFRASTRUCTURENODEMODEL_PARAM_NODEIP], ipAddress))
			ipAddress = GetNextIPAddress(ipAddress);
		else
			ipAddress = GetNextIPAddress(GetIPAddress());
		iterModel->second[INFRASTRUCTURENODEMODEL_PARAM_NODEIP] = IPAddressToString(ipAddress);
		m_listInfrastructureNodes->insertItem(QString("[%1] %2 (%3)").arg(iterIndex->second).arg(iterIndex->first).arg(iterModel->second[INFRASTRUCTURENODEMODEL_PARAM_NODEIP]), index);
		m_setModelNames.insert(strName);
		m_setVehicleIPs.insert(ipAddress);
		m_buttonInfrastructureNodeRemoveAll->setEnabled(m_listInfrastructureNodes->count() > 0);
		m_listInfrastructureNodes->setCurrentItem(index);
		slotInfrastructureNodesListHighlighted(m_listInfrastructureNodes->currentItem());
	}
}

void QSimCreateDialog::slotInfrastructureNodesRemove()
{
	unsigned int index = 0;
	std::set<QString> setAssocModels;
	while (index < m_listInfrastructureNodes->count())
	{
		if (m_listInfrastructureNodes->isSelected(index))
		{
			in_addr_t ipAddress;
			std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(m_vecInfrastructureNodes[index].first);
			std::map<QString, std::map<QString, ModelParameter> >::iterator iterModelParams = m_mapModelParams.find(m_vecInfrastructureNodes[index].second);
			std::map<QString, QString>::iterator iterParam;
			std::map<QString, ModelParameter>::iterator iterModelParam;
			std::vector<std::pair<QString, QString> >::iterator iterName = m_vecInfrastructureNodes.begin();
			iterName += index;

			for (iterParam = iterValues->second.begin(); iterParam != iterValues->second.end(); ++iterParam)
			{
				iterModelParam = iterModelParams->second.find(iterParam->first);
				if (iterModelParam != iterModelParams->second.end() && (iterModelParam->second.eType & 0xF) == ModelParameterTypeModel)
					setAssocModels.insert(iterParam->second);
			}
			m_setModelNames.erase(iterValues->first);
			if (StringToIPAddress(iterValues->second[INFRASTRUCTURENODEMODEL_PARAM_NODEIP], ipAddress))
				m_setVehicleIPs.erase(ipAddress);
			m_mapModels.erase(iterValues);
			m_vecInfrastructureNodes.erase(iterName);
			m_listInfrastructureNodes->removeItem(index);
		} else
			index++;
	}
	m_buttonInfrastructureNodeRemoveAll->setEnabled(m_listInfrastructureNodes->count() > 0);

	RemoveAssocModels(setAssocModels);
}

void QSimCreateDialog::slotInfrastructureNodesDuplicate()
{
	unsigned int index;
	for (index = 0; index < m_listInfrastructureNodes->count(); index++)
	{
		if (m_listInfrastructureNodes->isSelected(index))
		{
			in_addr_t ipAddress;
			std::vector<std::pair<QString, QString> >::iterator iterName = m_vecInfrastructureNodes.begin(), iterNextName;
			QString strName = GetNextModelName(iterName->second);
			std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(m_vecInfrastructureNodes[index].first), iterNextValues;
			iterName += index;

			if (StringToIPAddress(iterValues->second[INFRASTRUCTURENODEMODEL_PARAM_NODEIP], ipAddress))
				ipAddress = GetNextIPAddress(ipAddress);
			else
				ipAddress = GetNextIPAddress(GetIPAddress());
			iterNextName = m_vecInfrastructureNodes.insert(m_vecInfrastructureNodes.end(), std::pair<QString, QString>(strName, iterName->second));
			iterNextValues = m_mapModels.insert(std::pair<QString, std::map<QString, QString> >(iterNextName->first, iterValues->second)).first;
			iterNextValues->second[INFRASTRUCTURENODEMODEL_PARAM_NODEIP] = IPAddressToString(ipAddress);
			m_listInfrastructureNodes->insertItem(QString("[%1] %2 (%3)").arg(iterNextName->second).arg(iterNextName->first).arg(iterNextValues->second[INFRASTRUCTURENODEMODEL_PARAM_NODEIP]));
			m_setModelNames.insert(strName);
			m_setVehicleIPs.insert(ipAddress);
		}
	}
	m_buttonInfrastructureNodeRemoveAll->setEnabled(m_listInfrastructureNodes->count() > 0);
}

void QSimCreateDialog::slotInfrastructureNodeNameChanged(const QString & strName)
{
	int index = m_listInfrastructureNodes->currentItem();
	int iCursor = m_txtInfrastructureNodeName->cursorPosition();
	disconnect(m_listInfrastructureNodes, SIGNAL(highlighted(int)), this, SLOT(slotInfrastructureNodesListHighlighted(int)));
	if (index > -1)
	{
		bool bSelected = m_listInfrastructureNodes->isSelected(index);
		QString strNameOld = m_vecInfrastructureNodes[index].first;
		std::vector<std::pair<QString, QString> >::iterator iterName = m_vecInfrastructureNodes.begin();
		std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(strNameOld), iterNewValues;

		iterName += index;
		if (m_setModelNames.insert(strName).second)
		{
			m_setModelNames.erase(strNameOld);
			iterNewValues = m_mapModels.insert(std::pair<QString, std::map<QString, QString> >(strName, iterValues->second)).first;
			m_mapModels.erase(iterValues);
			m_vecInfrastructureNodes[index].first = strName;
			m_listInfrastructureNodes->changeItem(QString("[%1] %2 (%3)").arg(m_vecInfrastructureNodes[index].second).arg(strName).arg(iterNewValues->second[INFRASTRUCTURENODEMODEL_PARAM_NODEIP]), index);
		}
		m_listInfrastructureNodes->setSelected(index, bSelected);
	}
	connect(m_listInfrastructureNodes, SIGNAL(highlighted(int)), this, SLOT(slotInfrastructureNodesListHighlighted(int)));
	m_txtInfrastructureNodeName->setCursorPosition(iCursor);
}

void QSimCreateDialog::slotInfrastructureNodePropertiesValueChanged(int row, int col)
{
	int index = m_listInfrastructureNodes->currentItem();
	disconnect(m_listInfrastructureNodes, SIGNAL(highlighted(int)), this, SLOT(slotInfrastructureNodesListHighlighted(int)));
	if (index > -1 && row > -1 && col == 1)
	{
		bool bSelected = m_listInfrastructureNodes->isSelected(index);
		QString strParam = m_tableInfrastructureNodeProperties->text(row, 0);
		QString strValue = m_tableInfrastructureNodeProperties->text(row, col);
		std::vector<std::pair<QString, QString> >::iterator iterName = m_vecInfrastructureNodes.begin();
		std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(m_vecInfrastructureNodes[index].first);
		std::map<QString, ModelParameter>::iterator iterParam = m_mapModelParams[m_vecInfrastructureNodes[index].second].find(strParam);

		iterName += index;
		if (iterParam == m_mapModelParams[m_vecInfrastructureNodes[index].second].end() || ValidateTableItem(strValue, &iterParam->second))
		{
			if (strParam.compare(INFRASTRUCTURENODEMODEL_PARAM_NODEIP) == 0) // need to do additional uniqueness check
			{
				in_addr_t ipOld, ipAddress;
				if (StringToIPAddress(strValue, ipAddress) && m_setVehicleIPs.insert(ipAddress).second)
				{
					if (StringToIPAddress(iterValues->second[INFRASTRUCTURENODEMODEL_PARAM_NODEIP], ipOld))
						m_setVehicleIPs.erase(ipOld);
					iterValues->second[strParam] = strValue;
					m_listInfrastructureNodes->changeItem(QString("[%1] %2 (%3)").arg(iterName->second).arg(iterName->first).arg(strValue), index);
				} else
					AddTableItem(m_tableInfrastructureNodeProperties, row, col, iterValues->second[strParam], &iterParam->second);
			}
			else
				iterValues->second[strParam] = strValue;
		}
		else
			AddTableItem(m_tableInfrastructureNodeProperties, row, col, iterValues->second[strParam], &iterParam->second);
		m_listInfrastructureNodes->setSelected(index, bSelected);
	}
	connect(m_listInfrastructureNodes, SIGNAL(highlighted(int)), this, SLOT(slotInfrastructureNodesListHighlighted(int)));
}

void QSimCreateDialog::slotInfrastructureNodePropertiesCurrentChanged(int row, int col)
{
	int index = m_listInfrastructureNodes->currentItem();
	if (index > -1 && row > -1)
	{
		QString strParam = m_tableInfrastructureNodeProperties->text(row, 0);
		std::map<QString, ModelParameter>::iterator iterParam = m_mapModelParams[m_vecInfrastructureNodes[index].second].find(strParam);
		if (iterParam != m_mapModelParams[m_vecInfrastructureNodes[index].second].end())
			m_txtInfrastructureNodePropertiesHelp->setText(iterParam->second.strDesc);
		else
			m_txtInfrastructureNodePropertiesHelp->setText("");
	}
	else
		m_txtInfrastructureNodePropertiesHelp->setText("");
}

void QSimCreateDialog::slotInfrastructureNodesAutoGen()
{
	QAutoGenDialog * pDialog = new QAutoGenDialog(this);
	pDialog->SetParams(&m_mapModelParams, &m_vecModelTypes, INFRASTRUCTURENODEMODEL_NAME);
	pDialog->m_pModels = &m_mapModels;
	pDialog->m_pVehicleList = &m_vecInfrastructureNodes;
	pDialog->m_pModelList = &m_vecModels;
	pDialog->m_pVehicleIPs = &m_setVehicleIPs;
	pDialog->m_pModelNames = &m_setModelNames;
	pDialog->m_strIPAddressParam = INFRASTRUCTURENODEMODEL_PARAM_NODEIP;

	if (pDialog->exec() == QDialog::Accepted)
	{
		std::map<QString, std::map<QString, QString> >::iterator iterModel;
		unsigned int i;
		m_listInfrastructureNodes->clear();
		m_listModels->clear();
		for (i = 0; i < m_vecModels.size(); i++)
		{
			m_listModels->insertItem(QString("[%1] %2").arg(m_vecModels[i].second).arg(m_vecModels[i].first));
		}
		for (i = 0; i < m_vecInfrastructureNodes.size(); i++)
		{
			iterModel = m_mapModels.find(m_vecInfrastructureNodes[i].first);
			m_listInfrastructureNodes->insertItem(QString("[%1] %2 (%3)").arg(m_vecInfrastructureNodes[i].second).arg(m_vecInfrastructureNodes[i].first).arg(iterModel->second[INFRASTRUCTURENODEMODEL_PARAM_NODEIP]));
		}
		m_buttonInfrastructureNodeRemoveAll->setEnabled(m_listInfrastructureNodes->count() > 0);
		m_buttonModelRemoveAll->setEnabled(m_listModels->count() > 0);
		m_listInfrastructureNodes->setCurrentItem(m_listInfrastructureNodes->count() > 0 ? 0 : -1);
		m_listModels->setCurrentItem(m_listModels->count() > 0 ? 0 : -1);
		slotInfrastructureNodesListHighlighted(m_listInfrastructureNodes->currentItem());
		slotInfrastructureNodesListSelChanged();
		slotModelsListHighlighted(m_listModels->currentItem());
		slotModelsListSelChanged();
	}
	delete pDialog;
}

void QSimCreateDialog::slotInfrastructureNodesRemoveAll()
{
	unsigned int i;
	in_addr_t ipAddress;
	std::map<QString, std::map<QString, QString> >::iterator iterValues;
	std::set<QString> setAssocModels;

	for (i = 0; i < m_vecInfrastructureNodes.size(); i++)
	{
		std::map<QString, std::map<QString, ModelParameter> >::iterator iterModelParams = m_mapModelParams.find(m_vecInfrastructureNodes[i].second);
		std::map<QString, QString>::iterator iterParam;
		std::map<QString, ModelParameter>::iterator iterModelParam;
		iterValues = m_mapModels.find(m_vecInfrastructureNodes[i].first);

		for (iterParam = iterValues->second.begin(); iterParam != iterValues->second.end(); ++iterParam)
		{
			iterModelParam = iterModelParams->second.find(iterParam->first);
			if (iterModelParam != iterModelParams->second.end() && (iterModelParam->second.eType & 0xF) == ModelParameterTypeModel)
				setAssocModels.insert(iterParam->second);
		}
		m_setModelNames.erase(iterValues->first);
		if (StringToIPAddress(iterValues->second[INFRASTRUCTURENODEMODEL_PARAM_NODEIP], ipAddress))
			m_setVehicleIPs.erase(ipAddress);
		m_mapModels.erase(iterValues);
	}
	m_listInfrastructureNodes->clear();
	m_vecInfrastructureNodes.clear();
	m_buttonInfrastructureNodeRemoveAll->setEnabled(m_listInfrastructureNodes->count() > 0);
	m_listInfrastructureNodes->setCurrentItem(-1);
	slotInfrastructureNodesListHighlighted(m_listInfrastructureNodes->currentItem());
	slotInfrastructureNodesListSelChanged();

	RemoveAssocModels(setAssocModels);
}

void QSimCreateDialog::slotModelsListHighlighted(int index)
{
	m_lblModelName->setEnabled(index > -1);
	m_txtModelName->setEnabled(index > -1);
	disconnect(m_txtModelName, SIGNAL(textChanged(const QString& )), this, SLOT(slotModelNameChanged(const QString& )));
	disconnect(m_tableModelProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotModelPropertiesValueChanged(int, int )));
	if (index > -1)
	{
		int iRow;
		std::map<QString, std::map<QString, QString> >::iterator iterModel = m_mapModels.find(m_vecModels[index].first);
		std::map<QString, std::map<QString, ModelParameter> >::iterator iterParams = m_mapModelParams.find(m_vecModels[index].second);
		std::map<QString, ModelParameter>::iterator iterParam;
		m_txtModelName->setText(m_vecModels[index].first);
		if (iterModel != m_mapModels.end())
		{
			std::map<QString, QString>::iterator iterValue;
			m_tableModelProperties->setNumRows(0);
			m_tableModelProperties->setNumRows(iterModel->second.size());
			for (iterValue = iterModel->second.begin(), iRow = 0; iterValue != iterModel->second.end(); ++iterValue, iRow++)
			{
				iterParam = iterParams->second.find(iterValue->first);
				m_tableModelProperties->setText(iRow, 0, iterValue->first);
				if (iterParam != iterParams->second.end())
					AddTableItem(m_tableModelProperties, iRow, 1, iterValue->second, &iterParam->second);
			}
			m_tableModelProperties->sortColumn(0, true, true);
		} else
			m_tableModelProperties->setNumRows(0);
	}
	else {
		m_txtModelName->setText("");
		m_tableModelProperties->setNumRows(0);
	}
	slotModelPropertiesCurrentChanged(-1, -1);
	connect(m_tableModelProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotModelPropertiesValueChanged(int, int )));
	connect(m_txtModelName, SIGNAL(textChanged(const QString& )), this, SLOT(slotModelNameChanged(const QString& )));
	m_tableModelProperties->adjustColumn(0);
}

void QSimCreateDialog::slotModelsListSelChanged()
{
	bool bSelection = false;
	unsigned int i;
	for (i = 0; i < m_listModels->count(); i++)
	{
		if (m_listModels->isSelected(i))
		{
			bSelection = true;
			break;
		}
	}
	m_buttonModelRemove->setEnabled(bSelection);
	m_buttonModelDuplicate->setEnabled(bSelection);
}

void QSimCreateDialog::slotModelsAdd(int id)
{
	if (id > -1)
	{
		int index = m_listModels->currentItem() + 1;
		QString strModelType = m_vecModelTypes[id], strName = GetNextModelName(strModelType);
		if (strName.isEmpty())
			return;
		std::vector<std::pair<QString, QString> >::iterator iterIndex = m_vecModels.begin();
		std::map<QString, std::map<QString, QString> >::iterator iterModel = m_mapModels.insert(std::pair<QString, std::map<QString, QString> >(strName, std::map<QString, QString>())).first;
		std::map<QString, std::map<QString, ModelParameter> >::iterator iterParams = m_mapModelParams.find(strModelType);
		std::map<QString, ModelParameter>::iterator iterParam;
		for (iterParam = iterParams->second.begin(); iterParam != iterParams->second.end(); ++iterParam)
			iterModel->second[iterParam->first] = iterParam->second.strValue;
		if (index >= (signed)m_vecModels.size())
			iterIndex = m_vecModels.end();
		else
			iterIndex += index;
		iterIndex = m_vecModels.insert(iterIndex, std::pair<QString, QString>(strName, strModelType));
		m_listModels->insertItem(QString("[%1] %2").arg(iterIndex->second).arg(iterIndex->first), index);
		m_setModelNames.insert(strName);
		m_buttonModelRemoveAll->setEnabled(m_listModels->count() > 0);
		m_listModels->setCurrentItem(index);
		slotModelsListHighlighted(m_listModels->currentItem());
	}
}

void QSimCreateDialog::slotModelsRemove()
{
	unsigned int index = 0;
	while (index < m_listModels->count())
	{
		if (m_listModels->isSelected(index))
		{
			std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(m_vecModels[index].first);
			std::vector<std::pair<QString, QString> >::iterator iterName = m_vecModels.begin();
			iterName += index;
			m_setModelNames.erase(iterValues->first);
			m_mapModels.erase(iterValues);
			m_vecModels.erase(iterName);
			m_listModels->removeItem(index);
		} else
			index++;
	}
	m_buttonModelRemoveAll->setEnabled(m_listModels->count() > 0);
}

void QSimCreateDialog::slotModelsDuplicate()
{
	unsigned int index;
	for (index = 0; index < m_listModels->count(); index++)
	{
		if (m_listModels->isSelected(index))
		{
			QString strName = GetModelName(m_vecModels[index].first);
			if (strName.isEmpty())
				return;
			std::vector<std::pair<QString, QString> >::iterator iterName = m_vecModels.begin(), iterNextName;
			std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(m_vecModels[index].first), iterNextValues;
			iterName += index;

			iterNextName = m_vecModels.insert(m_vecModels.end(), std::pair<QString, QString>(strName, iterName->second));
			iterNextValues = m_mapModels.insert(std::pair<QString, std::map<QString, QString> >(iterNextName->first, iterValues->second)).first;
			m_listModels->insertItem(QString("[%1] %2").arg(iterNextName->second).arg(iterNextName->first));
			m_setModelNames.insert(strName);
		}
	}
	m_buttonModelRemoveAll->setEnabled(m_listModels->count() > 0);
}

void QSimCreateDialog::slotModelNameChanged(const QString & strName)
{
	int index = m_listModels->currentItem();
	int iCursor = m_txtModelName->cursorPosition();
	disconnect(m_listModels, SIGNAL(highlighted(int)), this, SLOT(slotModelsListHighlighted(int)));
	if (index > -1)
	{
		bool bSelected = m_listModels->isSelected(index);
		QString strNameOld = m_vecModels[index].first;
		std::vector<std::pair<QString, QString> >::iterator iterName = m_vecModels.begin();
		std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(strNameOld), iterNewValues;

		iterName += index;
		if (m_setModelNames.insert(strName).second)
		{
			m_setModelNames.erase(strNameOld);
			iterNewValues = m_mapModels.insert(std::pair<QString, std::map<QString, QString> >(strName, iterValues->second)).first;
			m_mapModels.erase(iterValues);
			m_vecModels[index].first = strName;
			m_listModels->changeItem(QString("[%1] %2").arg(m_vecModels[index].second).arg(strName), index);
		}
		m_listModels->setSelected(index, bSelected);
	}
	connect(m_listModels, SIGNAL(highlighted(int)), this, SLOT(slotModelsListHighlighted(int)));
	m_txtModelName->setCursorPosition(iCursor);
}

void QSimCreateDialog::slotModelPropertiesValueChanged(int row, int col)
{
	int index = m_listModels->currentItem();
	disconnect(m_listModels, SIGNAL(highlighted(int)), this, SLOT(slotModelsListHighlighted(int)));
	if (index > -1 && row > -1 && col == 1)
	{
		bool bSelected = m_listModels->isSelected(index);
		QString strParam = m_tableModelProperties->text(row, 0);
		QString strValue = m_tableModelProperties->text(row, col);
		std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(m_vecModels[index].first);
		std::map<QString, ModelParameter>::iterator iterParam = m_mapModelParams[m_vecModels[index].second].find(strParam);

		if (iterParam == m_mapModelParams[m_vecModels[index].second].end() || ValidateTableItem(strValue, &iterParam->second))
			iterValues->second[strParam] = strValue;
		else
			AddTableItem(m_tableModelProperties, row, col, iterValues->second[strParam], &iterParam->second);
		m_listModels->setSelected(index, bSelected);
	}
	connect(m_listModels, SIGNAL(highlighted(int)), this, SLOT(slotModelsListHighlighted(int)));
}

void QSimCreateDialog::slotModelPropertiesCurrentChanged(int row, int col)
{
	int index = m_listModels->currentItem();
	if (index > -1 && row > -1)
	{
		QString strParam = m_tableModelProperties->text(row, 0);
		std::map<QString, ModelParameter>::iterator iterParam = m_mapModelParams[m_vecModels[index].second].find(strParam);
		if (iterParam != m_mapModelParams[m_vecModels[index].second].end())
			m_txtModelPropertiesHelp->setText(iterParam->second.strDesc);
		else
			m_txtModelPropertiesHelp->setText("");
	}
	else
		m_txtModelPropertiesHelp->setText("");
}

void QSimCreateDialog::slotModelsRemoveAll()
{
	unsigned int i;
	std::map<QString, std::map<QString, QString> >::iterator iterValues;
	for (i = 0; i < m_vecModels.size(); i++)
	{
		iterValues = m_mapModels.find(m_vecModels[i].first);
		m_setModelNames.erase(iterValues->first);
		m_mapModels.erase(iterValues);
	}
	m_listModels->clear();
	m_vecModels.clear();
	m_buttonModelRemoveAll->setEnabled(m_listModels->count() > 0);
	m_listModels->setCurrentItem(-1);
	slotModelsListHighlighted(m_listModels->currentItem());
	slotModelsListSelChanged();
}

void QSimCreateDialog::slotOK()
{
	unsigned int i;
	std::map<QString, std::map<QString, QString> > mapModels;
	std::map<QString, QString> mapParams;
	std::map<QString, std::map<QString, QString> >::iterator iterModel;
	std::map<QString, QString>::iterator iterValue;
	qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	for (i = 0; i < m_vecModels.size(); i++)
	{
		iterModel = m_mapModels.find(m_vecModels[i].first);
		mapParams = iterModel->second;
		mapParams[PARAM_TYPE] = m_vecModels[i].second;
		mapModels.insert(std::pair<QString, std::map<QString, QString> >(m_vecModels[i].first, mapParams));
		mapParams.clear();
	}
	for (i = 0; i < m_vecInfrastructureNodes.size(); i++)
	{
		iterModel = m_mapModels.find(m_vecInfrastructureNodes[i].first);
		mapParams = iterModel->second;
		mapParams[PARAM_TYPE] = m_vecInfrastructureNodes[i].second;
		mapModels.insert(std::pair<QString, std::map<QString, QString> >(m_vecInfrastructureNodes[i].first, mapParams));
		mapParams.clear();
	}
	for (i = 0; i < m_vecVehicles.size(); i++)
	{
		iterModel = m_mapModels.find(m_vecVehicles[i].first);
		mapParams = iterModel->second;
		mapParams[PARAM_TYPE] = m_vecVehicles[i].second;
		mapModels.insert(std::pair<QString, std::map<QString, QString> >(m_vecVehicles[i].first, mapParams));
		mapParams.clear();
	}
	if (g_pSimulator->New(mapModels)) {
		qApp->restoreOverrideCursor();
		accept();
	} else {
		qApp->restoreOverrideCursor();
		QMessageBox::information(this, "GrooveNet", "Failed to create simulation. Please double-check parameters and try again.", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	}
}

void QSimCreateDialog::AddExistingModels()
{
	ModelTreeNode * pModelNodes = NULL;
	unsigned int iModelNodes = g_pSimulator->m_ModelMgr.GetAllModels(pModelNodes), i;
	bool bVehicle, bInfrastructureNode;
	QString strName, strModelType, strDepends;
	in_addr_t ipAddress;
	std::map<QString, QString> mapValues;
	std::map<QString, QString>::iterator iterValue;
	std::map<QString, std::map<QString, QString> >::iterator iterModel;
	std::map<QString, std::map<QString, ModelParameter> >::iterator iterModelParams;
	std::map<QString, ModelParameter>::iterator iterParam;
	std::list<ModelTreeNode *>::iterator iterDepend;

	for (i = 0; i < iModelNodes; i++)
	{
		strName = pModelNodes[i].strModelName;
		if (pModelNodes[i].pModel != NULL)
		{
			bVehicle = pModelNodes[i].pModel->IsModelTypeOf(CARMODEL_NAME, true);
			bInfrastructureNode = pModelNodes[i].pModel->IsModelTypeOf(INFRASTRUCTURENODEMODEL_NAME, true);
			strModelType = pModelNodes[i].pModel->GetModelType();
			iterModel = m_mapModels.insert(std::pair<QString, std::map<QString, QString> >(strName, std::map<QString, QString>())).first;
			iterModelParams = m_mapModelParams.find(strModelType);
			for (iterParam = iterModelParams->second.begin(); iterParam != iterModelParams->second.end(); ++iterParam)
				iterModel->second[iterParam->first] = iterParam->second.strValue;
			pModelNodes[i].pModel->Save(mapValues);
			for (iterValue = mapValues.begin(); iterValue != mapValues.end(); ++iterValue)
				iterModel->second[iterValue->first] = iterValue->second;
			strDepends = iterModel->second[PARAM_DEPENDS];
			for (iterDepend = pModelNodes[i].listDepends.begin(); iterDepend != pModelNodes[i].listDepends.end(); ++iterDepend)
			{
				if (strDepends.isEmpty())
					strDepends = (*iterDepend)->strModelName;
				else
					strDepends += (';' + (*iterDepend)->strModelName);
			}
			iterModel->second[PARAM_DEPENDS] = strDepends;
			mapValues.clear();
			if (bVehicle)
			{
				if (StringToIPAddress(iterModel->second[CARMODEL_PARAM_CARIP], ipAddress))
					m_setVehicleIPs.insert(ipAddress);
				m_vecVehicles.push_back(std::pair<QString, QString>(strName, strModelType));
				m_listVehicles->insertItem(QString("[%1] %2 (%3)").arg(strModelType).arg(strName).arg(iterModel->second[CARMODEL_PARAM_CARIP]));
			}
			else if (bInfrastructureNode)
			{
				if (StringToIPAddress(iterModel->second[INFRASTRUCTURENODEMODEL_PARAM_NODEIP], ipAddress))
					m_setVehicleIPs.insert(ipAddress);
				m_vecInfrastructureNodes.push_back(std::pair<QString, QString>(strName, strModelType));
				m_listInfrastructureNodes->insertItem(QString("[%1] %2 (%3)").arg(strModelType).arg(strName).arg(iterModel->second[INFRASTRUCTURENODEMODEL_PARAM_NODEIP]));
			}
			else
			{
				m_vecModels.push_back(std::pair<QString, QString>(strName, strModelType));
				m_listModels->insertItem(QString("[%1] %2").arg(strModelType).arg(strName));
			}
		}
		m_setModelNames.insert(strName);
	}
	g_pSimulator->m_ModelMgr.ReleaseAllModels();
	m_buttonVehicleRemoveAll->setEnabled(m_listVehicles->count() > 0);
	m_buttonInfrastructureNodeRemoveAll->setEnabled(m_listInfrastructureNodes->count() > 0);
	m_buttonModelRemoveAll->setEnabled(m_listModels->count() > 0);
	m_listVehicles->setCurrentItem(m_listVehicles->count() > 0 ? 0 : -1);
	m_listInfrastructureNodes->setCurrentItem(m_listInfrastructureNodes->count() > 0 ? 0 : -1);
	m_listModels->setCurrentItem(m_listModels->count() > 0 ? 0 : -1);
	slotVehiclesListHighlighted(m_listVehicles->currentItem());
	slotVehiclesListSelChanged();
	slotInfrastructureNodesListHighlighted(m_listInfrastructureNodes->currentItem());
	slotInfrastructureNodesListSelChanged();
	slotModelsListHighlighted(m_listModels->currentItem());
	slotModelsListSelChanged();
	setCaption("GrooveNet - Edit Existing Simulation...");
}

in_addr_t QSimCreateDialog::GetNextIPAddress(in_addr_t ipStart)
{
	while (m_setVehicleIPs.find(ipStart) != m_setVehicleIPs.end())
		ipStart++;
	return ipStart;
}

QString QSimCreateDialog::GetNextModelName(const QString & strBase)
{
	unsigned int i = 0;
	QString strRet;
	while (m_setModelNames.find(strRet = QString("%1%2").arg(strBase).arg(i)) != m_setModelNames.end())
		i++;
	return strRet;
}

QString QSimCreateDialog::GetModelName(const QString & strLast)
{
	bool bOK;
	QString strText;
	do
	{
		strText = QInputDialog::getText("GrooveNet - Choose Model Name...", "Enter a name for this model; this name MUST be unique.", QLineEdit::Normal, strLast, &bOK, this);
		if (!bOK || strText.isEmpty())
			return QString::null;
	} while (m_setModelNames.find(strText) != m_setModelNames.end());
	return strText;
}

void QSimCreateDialog::AddTableItem(QTable * pTable, int row, int col, const QString & strValue, const ModelParameter * pParam)
{
	QStringList listOptions;
	QStringList listModelTypes;
	QStringList::iterator iterModelType;
	unsigned int i;
	bool bFound = false;
	QComboTableItem * pComboItem;
	switch (pParam->eType)
	{
	case ModelParameterTypeBool:
		listOptions.push_back("True");
		listOptions.push_back("False");
		pTable->setItem(row, col, pComboItem = new QComboTableItem(pTable, listOptions, false));
		pComboItem->setCurrentItem(StringToBoolean(strValue) ? 0 : 1);
		break;
	case ModelParameterTypeYesNo:
		listOptions.push_back("Yes");
		listOptions.push_back("No");
		pTable->setItem(row, col, pComboItem = new QComboTableItem(pTable, listOptions, false));
		pComboItem->setCurrentItem(StringToBoolean(strValue) ? 0 : 1);
		break;
	case ModelParameterTypeModel:
		listModelTypes = QStringList::split(';', pParam->strAuxData, false);
		for (i = 0; i < m_vecVehicles.size(); i++)
		{
			for (iterModelType = listModelTypes.begin(); iterModelType != listModelTypes.end(); ++iterModelType)
			{
				if (g_pSimulator->m_ModelMgr.IsModelTypeOf(m_vecVehicles[i].second, *iterModelType))
				{
					if (m_vecVehicles[i].first == strValue)
						bFound = true;
					listOptions.push_back(m_vecVehicles[i].first);
				}
			}
		}
		for (i = 0; i < m_vecModels.size(); i++)
		{
			for (iterModelType = listModelTypes.begin(); iterModelType != listModelTypes.end(); ++iterModelType)
			{
				if (g_pSimulator->m_ModelMgr.IsModelTypeOf(m_vecModels[i].second, *iterModelType))
				{
					if (m_vecModels[i].first == strValue)
						bFound = true;
					listOptions.push_back(m_vecModels[i].first);
				}
			}
		}
		listOptions.push_back("NULL");
		if (!bFound && strValue != "NULL")
			listOptions.push_back(strValue);
		listOptions.sort();
		pTable->setItem(row, col, pComboItem = new QComboTableItem(pTable, listOptions, true));
		pComboItem->setCurrentItem(strValue);
		break;
	case ModelParameterTypeGPS:
		listOptions = QStringList::split(';', pParam->strAuxData, false);
		pTable->setItem(row, col, pComboItem = new QComboTableItem(pTable, listOptions, false));
		pComboItem->setCurrentItem(strValue);
		break;
	default:
		pTable->setText(row, col, strValue);
		break;
	}
}

bool QSimCreateDialog::ValidateTableItem(const QString & strValue, const ModelParameter * pParam)
{
	int iSep;
	switch (pParam->eType)
	{
	case ModelParameterTypeInt:
		if (pParam->strAuxData.isEmpty())
			return true;
		else if ((iSep = pParam->strAuxData.find(':')) > -1) // range
		{
			QString strMin = pParam->strAuxData.left(iSep);
			QString strMax = pParam->strAuxData.mid(iSep+1);
			long iValue = (long)StringToNumber(strValue);
			return (strMin.isEmpty() || iValue >= (long)StringToNumber(strMin)) && (strMax.isEmpty() || iValue <= (long)StringToNumber(strMax));
		}
		else // enumeration
		{
			QStringList listValues = QStringList::split(';', pParam->strAuxData, false);
			QStringList::iterator iterValue;
			long iValue = (long)StringToNumber(strValue);
			for (iterValue = listValues.begin(); iterValue != listValues.end(); ++iterValue)
			{
				if (iValue == (long)StringToNumber(*iterValue))
					return true;
			}
			return false;
		}
	case ModelParameterTypeFloat:
		if (pParam->strAuxData.isEmpty())
			return true;
		else if ((iSep = pParam->strAuxData.find(':')) > -1) // range
		{
			QString strMin = pParam->strAuxData.left(iSep);
			QString strMax = pParam->strAuxData.mid(iSep+1);
			double fValue = StringToNumber(strValue);
			return (strMin.isEmpty() || fValue >= StringToNumber(strMin)) && (strMax.isEmpty() || fValue <= StringToNumber(strMax));
		}
		else // enumeration
		{
			QStringList listValues = QStringList::split(';', pParam->strAuxData, false);
			QStringList::iterator iterValue;
			double fValue = StringToNumber(strValue);
			for (iterValue = listValues.begin(); iterValue != listValues.end(); ++iterValue)
			{
				if (fValue == StringToNumber(*iterValue))
					return true;
			}
			return false;
		}
	case ModelParameterTypeCoords:
	{
		Coords ptPosition;
		return ptPosition.FromString(strValue);
	}
	default:
		return true;
	}
}

void QSimCreateDialog::RemoveAssocModels(const std::set<QString> & setModelNames)
{
	unsigned int index = 0;
	std::set<QString> setAssocModels;
	while (index < m_vecModels.size())
	{
		if (setModelNames.find(m_vecModels[index].first) != setModelNames.end())
		{
			std::map<QString, std::map<QString, QString> >::iterator iterValues = m_mapModels.find(m_vecModels[index].first);
			std::map<QString, std::map<QString, ModelParameter> >::iterator iterModelParams = m_mapModelParams.find(m_vecModels[index].second);
			std::map<QString, QString>::iterator iterParam;
			std::map<QString, ModelParameter>::iterator iterModelParam;
			std::vector<std::pair<QString, QString> >::iterator iterName = m_vecModels.begin();
			iterName += index;

			for (iterParam = iterValues->second.begin(); iterParam != iterValues->second.end(); ++iterParam)
			{
				iterModelParam = iterModelParams->second.find(iterParam->first);
				if (iterModelParam != iterModelParams->second.end() && (iterModelParam->second.eType & 0xF) == ModelParameterTypeModel)
					setAssocModels.insert(iterParam->second);
			}
			m_setModelNames.erase(iterValues->first);
			m_mapModels.erase(iterValues);
			m_vecModels.erase(iterName);
			m_listModels->removeItem(index);
		} else
			index++;
	}
	m_buttonModelRemoveAll->setEnabled(m_listModels->count() > 0);

	if (!setAssocModels.empty())
		RemoveAssocModels(setAssocModels);
}

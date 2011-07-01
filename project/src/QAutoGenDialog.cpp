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

#include "QMapWidget.h"

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qtable.h>
#include <qtextedit.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qfile.h>

#include "QAutoGenModelDialog.h"
#include "QFileTableItem.h"
#include "Simulator.h"
#include "SimModel.h"
#include "StringHelp.h"
#include "Logger.h"

#include <qlayout.h>
#include <qsplitter.h>

#include "app16x16.xpm"

#define OTHERADDRESSES "<Other Addresses>"

#define COMBOITEM_FIXED "Fixed"
#define COMBOITEM_RANDOM "Random"
#define COMBOITEM_FILE "File"

QAutoGenDialog::QAutoGenDialog(QWidget * parent, const char * name, WFlags f)
: QDialog(parent, name, f), m_pModels(NULL), m_pVehicleList(NULL), m_pModelList(NULL), m_pVehicleIPs(NULL), m_pModelNames(NULL), m_pMapParams(NULL), m_pVecModelTypes(NULL)
{
	QWidget * pVehiclesBox = new QWidget(this);
	QLabel * labelVehicles = new QLabel("Node Type:", pVehiclesBox);
	QWidget * pRegionTypeBox = new QWidget(this);
	QLabel * labelRegionIDs = new QLabel("Parameter:", pRegionTypeBox);
	QLabel * labelRegionType = new QLabel("Region Type:", pRegionTypeBox);
	QSplitter * splitterDivider = new QSplitter(Qt::Vertical, this);
	QWidget * pMapBox = new QWidget(splitterDivider);
	QWidget * pButtonBox = new QWidget(this);

	QVBoxLayout * pLayout = new QVBoxLayout(this, 8, 8);
	QHBoxLayout * pVehiclesBoxLayout = new QHBoxLayout(pVehiclesBox, 0, 8);
	QHBoxLayout * pRegionTypeBoxLayout = new QHBoxLayout(pRegionTypeBox, 0, 8);
	QVBoxLayout * pMapBoxLayout = new QVBoxLayout(pMapBox, 0, 8);
	QHBoxLayout * pButtonBoxLayout = new QHBoxLayout(pButtonBox, 0, 8);

	setCaption("GrooveNet - Auto-Generate Nodes...");
	setIcon(app16x16_xpm);

	m_comboVehicleType = new QComboBox(false, pVehiclesBox);
	m_txtVehicles = new QSpinBox(1, 99999, 1, pVehiclesBox);
	m_comboRegionIDs = new QComboBox(true, pRegionTypeBox);
	m_comboRegionType = new QComboBox(false, pRegionTypeBox);
	m_pMap = new QMapWidget(pMapBox);
	m_labelRegionInfo = new QLabel("", pMapBox);
	m_tableProperties = new QTable(0, 3, splitterDivider);
	m_txtPropertiesHelp = new QTextEdit("", QString::null, splitterDivider);
	m_buttonOK = new QPushButton("&OK", pButtonBox);
	m_buttonCancel = new QPushButton("&Cancel", pButtonBox);

	connect(m_comboVehicleType, SIGNAL(activated(int)), this, SLOT(slotVehicleTypeActivated(int)));
	connect(m_comboRegionIDs, SIGNAL(activated(int)), this, SLOT(slotParameterActivated(int)));
	connect(m_comboRegionType, SIGNAL(activated(int)), this, SLOT(slotRegionTypeActivated(int)));
	connect(m_pMap, SIGNAL(selectionChanged()), this, SLOT(slotMapSelectionChanged()));
	connect(m_tableProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotPropertiesValueChanged(int, int)));
	connect(m_tableProperties, SIGNAL(currentChanged(int, int)), this, SLOT(slotPropertiesCurrentChanged(int, int)));
	connect(m_buttonOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

	m_txtVehicles->setSuffix(" nodes");
	m_comboRegionIDs->insertItem(OTHERADDRESSES);
	m_comboRegionType->insertItem("Bounding Rectangle", 0);
	m_pMap->SetSelectionMode(QMapWidget::SelectionModeRect);
	m_tableProperties->verticalHeader()->hide();
	m_tableProperties->setLeftMargin(0);
	m_tableProperties->horizontalHeader()->setClickEnabled(false);
	m_tableProperties->horizontalHeader()->setMovingEnabled(false);
	m_tableProperties->horizontalHeader()->setLabel(0, "Parameter");
	m_tableProperties->horizontalHeader()->setLabel(1, "Type");
	m_tableProperties->horizontalHeader()->setLabel(2, "Value");
	m_tableProperties->setSorting(false);
	m_tableProperties->setSelectionMode(QTable::SingleRow);
	m_tableProperties->setFocusStyle(QTable::FollowStyle);
	m_tableProperties->setColumnReadOnly(0, true);
	m_tableProperties->setColumnReadOnly(1, false);
	m_tableProperties->setColumnReadOnly(2, false);
	m_tableProperties->setColumnStretchable(2, true);
	m_txtPropertiesHelp->setReadOnly(true);
	m_buttonOK->setAutoDefault(false);
	m_buttonCancel->setAutoDefault(false);
	slotVehicleTypeActivated(m_comboVehicleType->currentItem());
	slotParameterActivated(m_comboRegionIDs->currentItem());
	slotRegionTypeActivated(m_comboRegionType->currentItem());

	pLayout->addWidget(pVehiclesBox);
	pLayout->addWidget(pRegionTypeBox);
	pLayout->addWidget(splitterDivider);
//	pLayout->addWidget(m_txtPropertiesHelp);
	pLayout->addWidget(pButtonBox);
	pVehiclesBoxLayout->addWidget(labelVehicles, 0, Qt::AlignLeft);
	pVehiclesBoxLayout->addWidget(m_comboVehicleType, 1);
	pVehiclesBoxLayout->addWidget(m_txtVehicles, 1);
	pRegionTypeBoxLayout->addWidget(labelRegionIDs, 0, Qt::AlignLeft);
	pRegionTypeBoxLayout->addWidget(m_comboRegionIDs, 1);
	pRegionTypeBoxLayout->addWidget(labelRegionType, 0, Qt::AlignLeft);
	pRegionTypeBoxLayout->addWidget(m_comboRegionType, 1);
	pMapBoxLayout->addWidget(m_pMap, 1);
	pMapBoxLayout->addWidget(m_labelRegionInfo, 0, Qt::AlignLeft);
	pButtonBoxLayout->addStretch(1);
	pButtonBoxLayout->addWidget(m_buttonOK, 0, Qt::AlignCenter);
	pButtonBoxLayout->addWidget(m_buttonCancel, 0, Qt::AlignCenter);
}

QAutoGenDialog::~QAutoGenDialog()
{
}

void QAutoGenDialog::SetParams(std::map<QString, std::map<QString, ModelParameter> > * pMapParams, std::vector<QString> * pVecModelTypes, const QString & strModelBaseType)
{
	unsigned int i;
	std::map<QString, std::map<QString, AutoGenParameter> >::iterator iterModel;
	std::map<QString, std::map<QString, ModelParameter> >::iterator iterModelParams;
	std::map<QString, ModelParameter>::iterator iterParam;

	m_pMapParams = pMapParams;
	m_pVecModelTypes = pVecModelTypes;
	m_comboVehicleType->clear();
	if (m_pVecModelTypes != NULL && m_pMapParams != NULL)
	{
		for (i = 0; i < m_pVecModelTypes->size(); i++)
		{
			iterModel = m_mapModels.insert(std::pair<QString, std::map<QString, AutoGenParameter> >((*m_pVecModelTypes)[i], std::map<QString, AutoGenParameter>())).first;
			iterModelParams = m_pMapParams->find((*m_pVecModelTypes)[i]);
			for (iterParam = iterModelParams->second.begin(); iterParam != iterModelParams->second.end(); ++iterParam)
				AddParameterFromModel(iterParam->first, iterParam->second, iterModel->second[iterParam->first]);
			if (g_pSimulator->m_ModelMgr.IsModelTypeOf((*m_pVecModelTypes)[i], strModelBaseType))
				m_comboVehicleType->insertItem((*m_pVecModelTypes)[i]);
		}
	}
	m_comboVehicleType->setCurrentText(SIMMODEL_NAME);
//	m_comboVehicleType->setCurrentItem(m_comboVehicleType->count() > 0 ? 0 : -1);
	slotVehicleTypeActivated(m_comboVehicleType->currentItem());
}

void QAutoGenDialog::slotVehicleTypeActivated(int index)
{
	m_buttonOK->setEnabled(index > -1);
	disconnect(m_tableProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotPropertiesValueChanged(int, int )));
	if (index > -1)
	{
		int iRow;
		std::map<QString, std::map<QString, AutoGenParameter> >::iterator iterModel = index > -1 && index < m_comboVehicleType->count() ? m_mapModels.find(m_comboVehicleType->text(index)) : m_mapModels.end();
		if (iterModel != m_mapModels.end())
		{
			std::map<QString, std::map<QString, ModelParameter> >::iterator iterParams = m_pMapParams->find(iterModel->first);
			std::map<QString, ModelParameter>::iterator iterParam;
			std::map<QString, AutoGenParameter>::iterator iterValue;
			m_tableProperties->setNumRows(0);
			m_tableProperties->setNumRows(iterModel->second.size());
			for (iterValue = iterModel->second.begin(), iRow = 0; iterValue != iterModel->second.end(); ++iterValue, iRow++)
			{
				iterParam = iterParams->second.find(iterValue->first);
				m_tableProperties->setText(iRow, 0, iterValue->first);
				if (iterParam != iterParams->second.end())
					m_tableProperties->setRowReadOnly(iRow, !AddTableItem(m_tableProperties, iRow, iterValue->second, true));
			}
			m_tableProperties->sortColumn(0, true, true);
		} else
			m_tableProperties->setNumRows(0);
	}
	else
		m_tableProperties->setNumRows(0);
	connect(m_tableProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotPropertiesValueChanged(int, int )));
	m_tableProperties->adjustColumn(0);
}

void QAutoGenDialog::slotParameterActivated(int index)
{
	QString strText = index > -1 && index < m_comboRegionIDs->count() ? m_comboRegionIDs->text(index) : QString::null;
	if (strText.isEmpty())
	{
		m_pMap->setEnabled(false);
		m_pMap->SetSelectionRect(Rect());
	}
	else
	{
		std::map<QString, Rect>::iterator iterMapSelection = m_mapMapSelections.find(strText);
		if (iterMapSelection == m_mapMapSelections.end())
			iterMapSelection = m_mapMapSelections.insert(std::pair<QString, Rect>(strText, Rect())).first;
		m_pMap->setEnabled(true);
		switch (m_pMap->GetSelectionMode())
		{
		case QMapWidget::SelectionModeRect:
			m_pMap->SetSelectionRect(iterMapSelection->second);
			if (iterMapSelection->second.m_iLeft != iterMapSelection->second.m_iRight || iterMapSelection->second.m_iTop != iterMapSelection->second.m_iBottom)
				m_pMap->recenter(Coords((iterMapSelection->second.m_iLeft + iterMapSelection->second.m_iRight) / 2, (iterMapSelection->second.m_iTop + iterMapSelection->second.m_iBottom) / 2));
			m_labelRegionInfo->setText(QString("Bounding box area: %1 sq. km.").arg(iterMapSelection->second.GetArea() * KILOMETERSPERMILE * KILOMETERSPERMILE));
			break;
		default:
			break;
		}
	}
}

void QAutoGenDialog::slotRegionTypeActivated(int index)
{
	switch (index)
	{
	case 0:
		m_pMap->SetSelectionMode(QMapWidget::SelectionModeRect);
		break;
	default:
		break;
	}
}

void QAutoGenDialog::slotMapSelectionChanged()
{
	std::map<QString, Rect>::iterator iterMapSelection = m_comboRegionIDs->currentItem() > -1 ? m_mapMapSelections.find(m_comboRegionIDs->text(m_comboRegionIDs->currentItem())) : m_mapMapSelections.end();
	if (iterMapSelection != m_mapMapSelections.end())
	{
		switch (m_pMap->GetSelectionMode())
		{
		case QMapWidget::SelectionModeRect:
			iterMapSelection->second = m_pMap->GetSelectionRect();
			m_labelRegionInfo->setText(QString("Bounding box area: %1 sq. km.").arg(iterMapSelection->second.GetArea() * KILOMETERSPERMILE * KILOMETERSPERMILE));
			break;
		default:
			break;
		}
	}
}

void QAutoGenDialog::slotPropertiesValueChanged(int row, int col)
{
	QString strParam = m_tableProperties->text(row, 0);
	std::map<QString, std::map<QString, AutoGenParameter> >::iterator iterModel = m_mapModels.find(m_comboVehicleType->currentText());
	if (iterModel != m_mapModels.end())
	{
		std::map<QString, AutoGenParameter>::iterator iterParam = iterModel->second.find(strParam);
		if (iterParam != iterModel->second.end())
		{
			if (col == 1)
			{
				QString strText = ((QComboTableItem *)m_tableProperties->item(row, 1))->currentText();
				switch (iterParam->second.eType & (~AutoGenParamTypeNoRandom))
				{
				case AutoGenParamTypeFixed:
				case AutoGenParamTypeRandom:
				case AutoGenParamTypeFile:
					if ((iterParam->second.eType & AutoGenParamTypeNoRandom) != AutoGenParamTypeNoRandom && strText.compare(COMBOITEM_RANDOM) == 0)
						iterParam->second.eType = AutoGenParamTypeRandom;
					else if (strText.compare(COMBOITEM_FILE) == 0)
						iterParam->second.eType = AutoGenParamTypeFile;
					else
						iterParam->second.eType = AutoGenParamTypeFixed;
					m_tableProperties->clearCell(row, 2);
					AddTableItem(m_tableProperties, row, iterParam->second);
					break;
				default:
					break;
				}
			}
			else if (col == 2)
			{
				iterParam->second.strValue = m_tableProperties->text(row, 2);
			}
		}
	}
}

void QAutoGenDialog::slotPropertiesCurrentChanged(int row, int col)
{
	if (row > -1)
	{
		QString strParam = m_tableProperties->text(row, 0);
		std::map<QString, std::map<QString, ModelParameter> >::iterator iterModel = m_pMapParams->find(m_comboVehicleType->currentText());
		if (iterModel != m_pMapParams->end())
		{
			std::map<QString, ModelParameter>::iterator iterParam = iterModel->second.find(strParam);
			if (iterParam != iterModel->second.end())
				m_txtPropertiesHelp->setText(iterParam->second.strDesc);
			else
				m_txtPropertiesHelp->setText("");
		}
		else
			m_txtPropertiesHelp->setText("");
	}
	else
		m_txtPropertiesHelp->setText("");
}

void QAutoGenDialog::accept()
{
	unsigned int i, iVehicles = m_txtVehicles->value(), iCar;
	QString strParam, strValue, strName, strType = m_comboVehicleType->currentText();
	std::map<QString, QString> mapParams;
	std::map<QString, std::map<QString, AutoGenParameter> >::iterator iterParams = m_mapModels.find(strType), iterAssocParams;
	std::map<QString, AutoGenParameter>::iterator iterParam;
	QStringList listDepends;
	QStringList::iterator iterDepend;
	in_addr_t ipAddress, ipTemp;
	std::map<QString, QString> mapAssocModels;
	std::map<QString, QString>::iterator iterAssocModel;
	std::map<QString, QString> mapAssocModelTypes;
	std::map<QString, QString>::iterator iterAssocModelType;
	std::vector<std::pair<QString, QString> > vecAssocModelTypes;
	std::map<QString, std::map<QString, AutoGenParameter> > mapAssocModelParams;
	std::map<QString, std::map<QString, ModelParameter> >::iterator iterModelType = m_pMapParams->find(strType), iterModelParams;
	std::map<QString, ModelParameter>::iterator iterModelParam;
	std::set<QString> setDepends;
	std::set<QString>::iterator iterSetDepend;
	QAutoGenModelDialog * pDialog;
	int nResult;
	bool bAddressSet = false;
	std::map<QString, Rect>::iterator iterAddressParams;

	// get record list for different parameters
	for (iterAddressParams = m_mapMapSelections.begin(); iterAddressParams != m_mapMapSelections.end(); ++iterAddressParams)
	{
		switch (m_comboRegionType->currentItem())
		{
		case 0:
			g_pMapDB->GetRecordsInRegion(m_mapRandomRecords[iterAddressParams->first], iterAddressParams->second);
			break;
		default:
			break;
		}
	}

	// make list of associated models
	for (iterParam = iterParams->second.begin(); iterParam != iterParams->second.end(); ++iterParam)
	{
		strParam = iterParam->first, strValue = iterParam->second.strValue;
		iterModelParam = iterModelType->second.find(strParam);
		if (iterParam->second.eType == AutoGenParamTypeNewModel)
		{
			vecAssocModelTypes.push_back(std::pair<QString, QString>(strParam, strValue));
			mapAssocModelTypes.insert(vecAssocModelTypes.back());
		}
	}

	for (i = 0; i < vecAssocModelTypes.size(); i++)
	{
		if (vecAssocModelTypes[i].second.compare(NULLMODEL_NAME) != 0)
		{
			pDialog = new QAutoGenModelDialog(vecAssocModelTypes[i].first, vecAssocModelTypes[i].second, this);
			m_mapModelIndexes[vecAssocModelTypes[i].second] = 0;
			iterModelParams = m_pMapParams->find(vecAssocModelTypes[i].second);
			iterAssocParams = m_mapModels.find(vecAssocModelTypes[i].second);
			iterAssocParams = mapAssocModelParams.insert(std::pair<QString, std::map<QString, AutoGenParameter> >(vecAssocModelTypes[i].first, iterAssocParams->second)).first;
			pDialog->m_pVecModelTypes = m_pVecModelTypes;
			pDialog->m_pAssocModelTypeMap = &mapAssocModelTypes;
			pDialog->m_pAssocModelTypes = &vecAssocModelTypes;
			pDialog->m_pModelParams = &iterModelParams->second;
			// generate auto-generator parameters
			pDialog->m_pParams = &iterAssocParams->second;
			pDialog->InitializeModelParams();
			nResult = pDialog->exec();
			delete pDialog;
			if (nResult == QDialog::Rejected)
				return;
		}
	}

	iCar = 0;
	qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	ipAddress = GetIPAddress();
	for (i = 0; i < iVehicles; i++)
	{
		// create this vehicle
		while (m_pModelNames->find(strName = QString("%1%2").arg(strType).arg(iCar)) != m_pModelNames->end())
			iCar++;
		for (iterParam = iterParams->second.begin(); iterParam != iterParams->second.end(); ++iterParam)
		{
			strParam = iterParam->first, strValue = iterParam->second.strValue;
			iterModelParam = iterModelType->second.find(strParam);
			switch (iterParam->second.eType & (~AutoGenParamTypeNoRandom))
			{
			case AutoGenParamTypeNewModel:
				iterAssocModelType = mapAssocModelTypes.find(strParam);
				setDepends.insert(mapParams[iterAssocModelType->first] = GetAssocModel(iterAssocModelType->first, iterAssocModelType->second, mapAssocModels, mapAssocModelTypes, mapAssocModelParams, i));
				break;
			case AutoGenParamTypeDepends:
				listDepends = QStringList::split(';', strValue);
				for (iterDepend = listDepends.begin(); iterDepend != listDepends.end(); ++iterDepend)
					setDepends.insert(*iterDepend);
				break;
			case AutoGenParamTypeIP:
				if (!bAddressSet && StringToIPAddress(strValue, ipTemp))
					ipAddress = ipTemp;
				break;
			case AutoGenParamTypeRandom:
				mapParams[strParam] = GetRandomParameter(strParam, strValue, iterModelParam->second);
				break;
			case AutoGenParamTypeFile:
				mapParams[strParam] = GetFileParameter(strParam, strValue, i);
				break;
			default:
				mapParams[strParam] = strValue;
				break;
			}
		}
		while (m_pVehicleIPs->find(ipAddress) != m_pVehicleIPs->end())
			ipAddress++;
		bAddressSet = true;
		mapParams[m_strIPAddressParam] = IPAddressToString(ipAddress);
		setDepends.erase("NULL");
		mapParams[PARAM_DEPENDS] = QString::null;
		for (iterSetDepend = setDepends.begin(); iterSetDepend != setDepends.end(); ++iterSetDepend)
		{
			if (mapParams[PARAM_DEPENDS].isEmpty())
				mapParams[PARAM_DEPENDS] = *iterSetDepend;
			else
				mapParams[PARAM_DEPENDS] += (';' + *iterSetDepend);
		}
		setDepends.clear();
		m_pVehicleIPs->insert(ipAddress);
		m_pModels->insert(std::pair<QString, std::map<QString, QString> >(strName, mapParams));
		m_pVehicleList->push_back(std::pair<QString, QString>(strName, strType));
		m_pModelNames->insert(strName);
		mapAssocModels.clear();
		iCar++;
		ipAddress++;
	}

	m_mapFileData.clear();
	qApp->restoreOverrideCursor();
	QDialog::accept();
}

bool QAutoGenDialog::AddTableItem(QTable * pTable, int row, AutoGenParameter & param, bool bUpdateTypes)
{
	QStringList listOptions;
	int iSep;
	QString strCurrent;
	bool bEditable = true;
	unsigned int i;
	switch (param.eType & (~AutoGenParamTypeNoRandom))
	{
	case AutoGenParamTypeFixed:
		listOptions.push_back(COMBOITEM_FIXED);
		if ((param.eType & AutoGenParamTypeNoRandom) != AutoGenParamTypeNoRandom)
			listOptions.push_back(COMBOITEM_RANDOM);
		listOptions.push_back(COMBOITEM_FILE);
		strCurrent = COMBOITEM_FIXED;
		if (param.strAuxData.isEmpty())
			pTable->setText(row, 2, param.strValue);
		else
		{
			if ((iSep = param.strAuxData.find(':')) > -1)
			{ // range
				pTable->setText(row, 2, param.strValue);
			}
			else
			{ // enumeration
				QStringList listCombo = QStringList::split(';', param.strAuxData, false);
				pTable->setItem(row, 2, new QComboTableItem(pTable, listCombo, false));
				((QComboTableItem *)pTable->item(row, 2))->setCurrentItem(param.strValue);
				param.strValue = pTable->text(row, 2);
			}
		}
		break;
	case AutoGenParamTypeRandom:
		pTable->setText(row, 2, param.strValue);
		listOptions.push_back(COMBOITEM_FIXED);
		if ((param.eType & AutoGenParamTypeNoRandom) != AutoGenParamTypeNoRandom)
			listOptions.push_back(COMBOITEM_RANDOM);
		listOptions.push_back(COMBOITEM_FILE);
		strCurrent = COMBOITEM_RANDOM;
		break;
	case AutoGenParamTypeFile:
		pTable->setItem(row, 2, new QFileTableItem(pTable));
		pTable->item(row, 2)->setText(param.strValue);
		listOptions.push_back(COMBOITEM_FIXED);
		if ((param.eType & AutoGenParamTypeNoRandom) != AutoGenParamTypeNoRandom)
			listOptions.push_back(COMBOITEM_RANDOM);
		listOptions.push_back(COMBOITEM_FILE);
		strCurrent = COMBOITEM_FILE;
		break;
	case AutoGenParamTypeNewModel:
		for (i = 0; i < m_pVecModelTypes->size(); i++)
		{
			if (g_pSimulator->m_ModelMgr.IsModelTypeOf((*m_pVecModelTypes)[i], param.strAuxData))
				listOptions.push_back((*m_pVecModelTypes)[i]);
		}
		listOptions.push_back(NULLMODEL_NAME);
		pTable->setItem(row, 2, new QComboTableItem(pTable, listOptions, false));
		((QComboTableItem *)pTable->item(row, 2))->setCurrentItem(param.strValue);
		param.strValue = pTable->text(row, 2);
		listOptions.clear();
		listOptions.push_back(COMBOITEM_FIXED);
		strCurrent = COMBOITEM_FIXED;
		break;
	default:
		pTable->setText(row, 2, param.strValue);
		listOptions.push_back(COMBOITEM_FIXED);
		strCurrent = COMBOITEM_FIXED;
		break;
	}
	if (bUpdateTypes)
	{
		if (!listOptions.empty()) {
			pTable->setItem(row, 1, new QComboTableItem(pTable, listOptions, false));
			((QComboTableItem *)pTable->item(row, 1))->setCurrentItem(strCurrent);
		} else
			pTable->clearCell(row, 1);
	}
	return bEditable;
}

void QAutoGenDialog::AddParameterFromModel(const QString & strParam, const ModelParameter & modelValue, AutoGenParameter & param)
{
	if (strParam.compare(PARAM_DEPENDS) == 0)
	{
		param.eType = AutoGenParamTypeDepends;
		param.strValue = modelValue.strValue;
		param.strAuxData = QString::null;
	}
	else if (strParam.compare(m_strIPAddressParam) == 0)
	{
		param.eType = AutoGenParamTypeIP;
		param.strValue = modelValue.strValue;
		param.strAuxData = QString::null;
	}
	else
	{
		switch (modelValue.eType & 0xF)
		{
		case ModelParameterTypeBool:
			param.eType = AutoGenParamTypeFixed;
			if ((modelValue.eType & ModelParameterFixed) == ModelParameterFixed)
				param.eType = (AutoGenParamType)(AutoGenParamTypeNoRandom | param.eType);
			param.strValue = StringToBoolean(modelValue.strValue) ? "True" : "False";
			param.strAuxData = modelValue.strAuxData.isEmpty() ? "True;False" : modelValue.strAuxData;
			break;
		case ModelParameterTypeYesNo:
			param.eType = AutoGenParamTypeFixed;
			if ((modelValue.eType & ModelParameterFixed) == ModelParameterFixed)
				param.eType = (AutoGenParamType)(AutoGenParamTypeNoRandom | param.eType);
			param.strValue = StringToBoolean(modelValue.strValue) ? "Yes" : "No";
			param.strAuxData = modelValue.strAuxData.isEmpty() ? "Yes;No" : modelValue.strAuxData;
			break;
		case ModelParameterTypeModel:
			param.eType = AutoGenParamTypeNewModel;
			param.strValue = NULLMODEL_NAME;
			param.strAuxData = modelValue.strAuxData;
			break;
		case ModelParameterTypeIP:
			param.eType = AutoGenParamTypeIP;
			param.strValue = modelValue.strValue;
			param.strAuxData = modelValue.strAuxData;
			break;
		default:
			param.eType = AutoGenParamTypeFixed;
			if ((modelValue.eType & ModelParameterFixed) == ModelParameterFixed)
				param.eType = (AutoGenParamType)(AutoGenParamTypeNoRandom | param.eType);
			param.strValue = modelValue.strValue;
			param.strAuxData = modelValue.strAuxData;
			break;
		}
	}
}

QString QAutoGenDialog::GetAssocModel(const QString & strParam, const QString & strType, std::map<QString, QString> & mapAssocModels, const std::map<QString, QString> & mapAssocModelTypes, std::map<QString, std::map<QString, AutoGenParameter> > & mapAssocParams, unsigned int iVehicle)
{
	// first, check list of models to see if one of the right type exists
	std::map<QString, QString>::iterator iterModel = mapAssocModels.find(strParam);

	if (iterModel == mapAssocModels.end())
	{
		if (strType.compare(NULLMODEL_NAME) == 0)
			iterModel = mapAssocModels.insert(std::pair<QString, QString>(strParam, "NULL")).first;
		else
		{
			// no such luck, let's make a new model
			std::map<QString, std::map<QString, ModelParameter> >::iterator iterModelParams = m_pMapParams->find(strType);
			std::map<QString, ModelParameter>::iterator iterParam;
			std::map<QString, unsigned int>::iterator iterModelIndex = m_mapModelIndexes.find(strType);
			std::map<QString, QString>::const_iterator iterModelType;
			std::map<QString, QString> mapParams;
			std::map<QString, std::map<QString, AutoGenParameter> >::iterator iterAutoParams = mapAssocParams.find(strParam);
			std::map<QString, AutoGenParameter>::iterator iterAutoParam;
			QStringList listDepends;
			QStringList::iterator iterDepend;
			std::set<QString> setDepends;
			std::set<QString>::iterator iterSetDepend;
			QString strName;
			unsigned iModel = iterModelIndex == m_mapModelIndexes.end() ? 0 : iterModelIndex->second;
			while (m_pModelNames->find(strName = QString("%1%2").arg(strType).arg(iModel)) != m_pModelNames->end())
				iModel++;
			if (iterModelParams != m_pMapParams->end() && iterAutoParams != mapAssocParams.end())
			{
				for (iterAutoParam = iterAutoParams->second.begin(); iterAutoParam != iterAutoParams->second.end(); ++iterAutoParam)
				{
					iterParam = iterModelParams->second.find(iterAutoParam->first);
					switch (iterAutoParam->second.eType)
					{
					case AutoGenParamTypeDepends:
						listDepends = QStringList::split(';', iterAutoParam->second.strValue);
						for (iterDepend = listDepends.begin(); iterDepend != listDepends.end(); ++iterDepend)
							setDepends.insert(*iterDepend);
						break;
					case AutoGenParamTypeNewModel:
						iterModelType = mapAssocModelTypes.find(iterAutoParam->first);
						if (iterModelType == mapAssocModelTypes.end())
							mapParams[iterAutoParam->first] = GetAssocModel(iterAutoParam->first, iterAutoParam->second.strValue, mapAssocModels, mapAssocModelTypes, mapAssocParams, iVehicle);
						else
							mapParams[iterAutoParam->first] = GetAssocModel(iterAutoParam->first, iterModelType->second, mapAssocModels, mapAssocModelTypes, mapAssocParams, iVehicle);
						setDepends.insert(mapParams[iterAutoParam->first]);
						break;
					case AutoGenParamTypeRandom: // randomize
						mapParams[iterAutoParam->first] = GetRandomParameter(iterAutoParam->first, iterAutoParam->second.strValue, iterParam->second);
						break;
					case AutoGenParamTypeFile: // choose file
						mapParams[iterAutoParam->first] = GetFileParameter(iterAutoParam->first, iterAutoParam->second.strValue, iVehicle);
						break;
					default:
						mapParams[iterAutoParam->first] = iterAutoParam->second.strValue;
						break;
					}
				}
			}
			setDepends.erase("NULL");
			mapParams[PARAM_DEPENDS] = QString::null;
			for (iterSetDepend = setDepends.begin(); iterSetDepend != setDepends.end(); ++iterSetDepend)
			{
				if (mapParams[PARAM_DEPENDS].isEmpty())
					mapParams[PARAM_DEPENDS] = *iterSetDepend;
				else
					mapParams[PARAM_DEPENDS] += (';' + *iterSetDepend);
			}
			iterModel = mapAssocModels.insert(std::pair<QString, QString>(strParam, strName)).first;
			m_pModels->insert(std::pair<QString, std::map<QString, QString> >(strName, mapParams));
			m_pModelList->push_back(std::pair<QString, QString>(strName, strType));
			m_pModelNames->insert(strName);
			listDepends.clear();
			if (iterModelIndex != m_mapModelIndexes.end())
				iterModelIndex->second = iModel + 1;
			else
				m_mapModelIndexes[strType] = iModel + 1;
		}
	}
	return iterModel->second;
}

QString QAutoGenDialog::GetRandomParameter(const QString & strParam, const QString & strValue, const ModelParameter & param)
{
	QString strRet;
	QStringList listRandom = QStringList::split(';', strValue);
	unsigned int iRandom = RandUInt(0, listRandom.size());
	int iSep = strValue.find(':');
	switch (param.eType & 0xF)
	{
	case ModelParameterTypeInt:
		if (iSep > -1)
			strRet = QString("%1").arg(RandInt((long)StringToNumber(strValue.left(iSep)), (long)StringToNumber(strValue.mid(iSep+1))));
		else if (!listRandom.empty())
			strRet = listRandom[iRandom];
		break;
	case ModelParameterTypeFloat:
		if (iSep > -1)
			strRet = QString("%1").arg(RandDouble(StringToNumber(strValue.left(iSep)), StringToNumber(strValue.mid(iSep+1))));
		else if (!listRandom.empty())
			strRet = listRandom[iRandom];
		break;
	case ModelParameterTypeAddress:
		strRet = GetRandomAddress(strParam);
		break;
	case ModelParameterTypeAddresses: // TODO: generate random address list
		if (!listRandom.empty())
			strRet = listRandom[iRandom];
		break;
	case ModelParameterTypeCoords:
		if (iSep > -1)
		{
			Coords ptMin, ptMax;
			ptMin.FromString(strValue.left(iSep));
			ptMax.FromString(strValue.mid(iSep+1));
			strRet = Coords(RandInt(ptMin.m_iLong, ptMax.m_iLong), RandInt(ptMin.m_iLat, ptMax.m_iLat)).ToString();
		}
		else if (!listRandom.empty())
			strRet = listRandom[iRandom];
		else
			strRet = GetRandomPosition(strParam);
		break;
	default:
		if (!listRandom.empty())
			strRet = listRandom[iRandom];
		break;
	}
	return strRet;
}

QString QAutoGenDialog::GetRandomAddress(const QString & strParam)
{
	QString strAddress;
	Address sAddress;
	std::map<QString, std::vector<unsigned int> >::iterator iterRecords = m_mapRandomRecords.find(strParam);
	std::map<QString, Rect>::iterator iterMapSelection = m_mapMapSelections.find(strParam);
	unsigned int i, iMaxRetries = 3, iRecord, iShapePoint;
	float fProgress;
	MapRecord * pRecord;

	if (iterRecords == m_mapRandomRecords.end())
	{
		iterRecords = m_mapRandomRecords.find(OTHERADDRESSES);
		iterMapSelection = m_mapMapSelections.find(OTHERADDRESSES);
		if (iterRecords == m_mapRandomRecords.end())
			return strAddress;
	}

	// choose a random location in this region
	if (iterRecords->second.empty())
		return strAddress;

	for (i = 0; i < iMaxRetries; i++)
	{
		iRecord = RandUInt(0, iterRecords->second.size());
		pRecord = g_pMapDB->GetRecord(iterRecords->second[iRecord]);
		if (pRecord->nShapePoints > 1)
		{
			iShapePoint = RandUInt(0, pRecord->nShapePoints - 1);
			fProgress = (float)RandDouble(0., 1.);
			if (g_pMapDB->AddressFromRecord(&sAddress, iterRecords->second[iRecord], iShapePoint, fProgress))
			{
				strAddress = AddressToString(&sAddress);
				if (StringToAddress(strAddress, &sAddress) && sAddress.iRecord != (unsigned)-1 && iterMapSelection->second.intersectRect(pRecord->rBounds))
					break;
			}
		}
	}
	return strAddress;
}

QString QAutoGenDialog::GetRandomPosition(const QString & strParam)
{
	QString strAddress;
	Address sAddress;
	std::map<QString, Rect>::iterator iterMapSelection = m_mapMapSelections.find(strParam);


	// choose a random location in this region
	if (iterMapSelection == m_mapMapSelections.end())
	{
		iterMapSelection = m_mapMapSelections.find(OTHERADDRESSES);
		if (iterMapSelection == m_mapMapSelections.end())
			return "";
	}

	return Coords(RandInt(iterMapSelection->second.m_iLeft, iterMapSelection->second.m_iRight), RandInt(iterMapSelection->second.m_iBottom, iterMapSelection->second.m_iTop)).ToString();
}

QString QAutoGenDialog::GetFileParameter(const QString & strParam, const QString & strFilename, unsigned int iVehicle)
{
	std::map<QString, std::vector<QString> >::iterator iterFileData = m_mapFileData.find(strFilename);
	if (iterFileData == m_mapFileData.end())
	{
		QFile fileData(strFilename);
		if (fileData.exists() && fileData.open(IO_ReadOnly | IO_Translate))
		{
			QTextStream streamData(&fileData);
			QString strLine;
			iterFileData = m_mapFileData.insert(std::pair<QString, std::vector<QString> >(strFilename, std::vector<QString>())).first;
			while (!(strLine = streamData.readLine()).isNull())
			{
				strLine = strLine.stripWhiteSpace();
				if (strLine.isEmpty())
					continue;
				iterFileData->second.push_back(strLine);
			}
			fileData.close();
		}
	}
	if (iterFileData == m_mapFileData.end())
		return "";
	else
		return iterFileData->second[iVehicle % iterFileData->second.size()];
}

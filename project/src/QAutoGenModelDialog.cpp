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

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qtable.h>
#include <qapplication.h>
#include <qcursor.h>

#include "QAutoGenModelDialog.h"
#include "QFileTableItem.h"
#include "Simulator.h"
#include "SimModel.h"
#include "StringHelp.h"

#include <qlayout.h>
#include <qframe.h>
#include <qsplitter.h>

#include "app16x16.xpm"

#define COMBOITEM_FIXED "Fixed"
#define COMBOITEM_RANDOM "Random"
#define COMBOITEM_FILE "File"

QAutoGenModelDialog::QAutoGenModelDialog(const QString & strModelName, const QString & strModelType, QWidget * parent, const char * name, WFlags f)
: QDialog(parent, name, f), m_pVecModelTypes(NULL), m_pModelParams(NULL), m_pParams(NULL), m_pAssocModelTypeMap(NULL), m_pAssocModelTypes(NULL), m_strModelName(strModelName), m_strModelType(strModelType)
{
	QSplitter * splitterDivider = new QSplitter(Qt::Vertical, this);
	QWidget * pButtonBox = new QWidget(this);

	QVBoxLayout * pLayout = new QVBoxLayout(this, 8, 8);
	QHBoxLayout * pButtonLayout = new QHBoxLayout(pButtonBox, 0, 8);

	setCaption("GrooveNet - Specify Model Parameters...");
	setIcon(app16x16_xpm);

	m_labelModel = new QLabel(QString("Model: [%1] %2").arg(strModelType).arg(strModelName), this);
	m_tableProperties = new QTable(0, 3, splitterDivider);
	m_txtPropertiesHelp = new QTextEdit("", QString::null, splitterDivider);
	m_buttonProceed = new QPushButton("&Proceed", pButtonBox);
	m_buttonCancel = new QPushButton("&Cancel", pButtonBox);

	connect(m_tableProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotPropertiesValueChanged(int, int)));
	connect(m_tableProperties, SIGNAL(currentChanged(int, int)), this, SLOT(slotPropertiesCurrentChanged(int, int)));
	connect(m_buttonProceed, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

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
	m_buttonProceed->setAutoDefault(false);

	pLayout->addWidget(m_labelModel, 0, Qt::AlignLeft);
	pLayout->addWidget(splitterDivider, 1);
//	pLayout->addWidget(m_txtPropertiesHelp);
	pLayout->addWidget(pButtonBox, 0);
	pButtonLayout->addWidget(m_buttonProceed, 0, Qt::AlignCenter);
	pButtonLayout->addWidget(m_buttonCancel, 0, Qt::AlignCenter);
}

QAutoGenModelDialog::~QAutoGenModelDialog()
{
}

void QAutoGenModelDialog::InitializeModelParams()
{
	int iRow;
	std::map<QString, ModelParameter>::iterator iterParam;
	std::map<QString, AutoGenParameter>::iterator iterValue;

	disconnect(m_tableProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotPropertiesValueChanged(int, int )));
	m_tableProperties->setNumRows(0);
	for (iterValue = m_pParams->begin(), iRow = 0; iterValue != m_pParams->end(); ++iterValue)
	{
		iterParam = m_pModelParams->find(iterValue->first);
		m_tableProperties->insertRows(iRow);
		m_tableProperties->setText(iRow, 0, iterValue->first);
		if (iterParam != m_pModelParams->end())
			m_tableProperties->setRowReadOnly(iRow, !AddTableItem(m_tableProperties, iRow, iterValue->first, iterValue->second, true, (iterValue->second.eType == AutoGenParamTypeNewModel && m_pAssocModelTypeMap->find(iterValue->first) != m_pAssocModelTypeMap->end())));
		else
			m_tableProperties->setRowReadOnly(iRow, true);
		iRow++;
	}
	m_tableProperties->sortColumn(0, true, true);
	connect(m_tableProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotPropertiesValueChanged(int, int )));
	m_tableProperties->adjustColumn(0);
}

void QAutoGenModelDialog::slotPropertiesValueChanged(int row, int col)
{
	QString strParam = m_tableProperties->text(row, 0);
	std::map<QString, AutoGenParameter>::iterator iterParam = m_pParams->find(strParam);
	if (iterParam != m_pParams->end())
	{
		if (col == 1 && m_tableProperties->item(row, 1) != NULL)
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
				AddTableItem(m_tableProperties, row, iterParam->first, iterParam->second);
				break;
			default:
				break; // no actual change here
			}
		}
		else if (col == 2)
		{
			iterParam->second.strValue = m_tableProperties->text(row, 2);
		}
	}
}
void QAutoGenModelDialog::slotPropertiesCurrentChanged(int row, int col)
{
	if (row > -1)
	{
		QString strParam = m_tableProperties->text(row, 0);
		std::map<QString, ModelParameter>::iterator iterParam = m_pModelParams->find(strParam);
		if (iterParam != m_pModelParams->end())
			m_txtPropertiesHelp->setText(iterParam->second.strDesc);
		else
			m_txtPropertiesHelp->setText("");
	}
	else
		m_txtPropertiesHelp->setText("");
}

void QAutoGenModelDialog::accept()
{
	std::map<QString, AutoGenParameter>::iterator iterValue;
	for (iterValue = m_pParams->begin(); iterValue != m_pParams->end(); ++iterValue)
	{
		if (iterValue->second.eType == AutoGenParamTypeNewModel && m_pAssocModelTypeMap->find(iterValue->first) == m_pAssocModelTypeMap->end())
		{
			m_pAssocModelTypes->push_back(std::pair<QString, QString>(iterValue->first, iterValue->second.strValue));
			m_pAssocModelTypeMap->insert(m_pAssocModelTypes->back());
		}
	}

	QDialog::accept();
}

bool QAutoGenModelDialog::AddTableItem(QTable * pTable, int row, const QString & strParam, AutoGenParameter & param, bool bUpdateTypes, bool bDisable)
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
			if (g_pSimulator->m_ModelMgr.IsModelTypeOf((*m_pVecModelTypes)[i], param.strAuxData) && m_strModelType.compare((*m_pVecModelTypes)[i]) != 0)
				listOptions.push_back((*m_pVecModelTypes)[i]);
		}
		listOptions.push_back(NULLMODEL_NAME);
		pTable->setItem(row, 2, new QComboTableItem(pTable, listOptions, false));
		if (m_pAssocModelTypeMap->find(strParam) != m_pAssocModelTypeMap->end())
			param.strValue = (*m_pAssocModelTypeMap)[strParam];
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
	return bEditable && !bDisable;
}

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

#include "QConfigureDialog.h"
#include "QExpandableTableItem.h"
#include "QSettingColorTableItem.h"
#include "Simulator.h"

#include <qtable.h>
#include <qheader.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcolordialog.h>
#include <qfiledialog.h>

#include "app16x16.xpm"

QConfigureDialog::QConfigureDialog(QWidget *parent, const char *name)
: QDialog(parent, name), m_Settings(*g_pSettings)
{
	QVBoxLayout * layout = new QVBoxLayout(this, 8, 8, "layout");
	QWidget * pButtonBox = new QWidget(this, "buttonbox");
	QHBoxLayout * buttonBoxLayout = new QHBoxLayout(pButtonBox, 0, 8, "buttonbox.layout");

	m_pTableProperties = new QTable(0, 2, this);
	connect(m_pTableProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotSettingsChanged(int, int)));
	connect(m_pTableProperties, SIGNAL(doubleClicked(int, int, int, const QPoint& )), this, SLOT(slotTableDoubleClicked(int, int, int, const QPoint& )));
	connect(m_pTableProperties, SIGNAL(clicked(int, int, int, const QPoint& )), this, SLOT(slotTableClicked(int, int, int, const QPoint& )));
	m_pDefault = new QPushButton("&Default", pButtonBox, "default");
	connect(m_pDefault, SIGNAL(clicked()), this, SLOT(slotDefault()));
	m_pDefault->setEnabled(true);
	m_pDefault->setAutoDefault(false);
	m_pOK = new QPushButton("&OK", pButtonBox, "ok");
	connect(m_pOK, SIGNAL(clicked()), this, SLOT(slotOK()));
	m_pOK->setEnabled(false);
	m_pOK->setAutoDefault(false);
	m_pCancel = new QPushButton("&Close", pButtonBox, "cancel");
	connect(m_pCancel, SIGNAL(clicked()), this, SLOT(reject()));
	m_pCancel->setEnabled(true);
	m_pCancel->setAutoDefault(false);
	m_pApply = new QPushButton("&Apply", pButtonBox, "apply");
	connect(m_pApply, SIGNAL(clicked()), this, SLOT(slotApply()));
	m_pApply->setEnabled(false);
	m_pApply->setAutoDefault(false);

	buttonBoxLayout->addWidget(m_pDefault);
	buttonBoxLayout->addStretch(1);
	buttonBoxLayout->addWidget(m_pOK);
	buttonBoxLayout->addWidget(m_pCancel);
	buttonBoxLayout->addWidget(m_pApply);

	layout->addWidget(m_pTableProperties, 1);
	layout->addWidget(pButtonBox);

	setCaption("Configure GrooveNet...");
	setIcon(app16x16_xpm);
	setSizeGripEnabled(true);

	Setup();
}

void QConfigureDialog::Setup()
{
	QHeader * pHHeader = m_pTableProperties->horizontalHeader();

	m_pTableProperties->verticalHeader()->hide();
	m_pTableProperties->setLeftMargin(0);
	pHHeader->setClickEnabled(false);
	pHHeader->setMovingEnabled(false);
	pHHeader->setLabel(0, "Property");
	pHHeader->setLabel(1, "Value");
	m_pTableProperties->setSorting(false);
	m_pTableProperties->setSelectionMode(QTable::SingleRow);
	m_pTableProperties->setFocusStyle(QTable::FollowStyle);
	m_pTableProperties->setColumnReadOnly(0, true);
	m_pTableProperties->setColumnReadOnly(1, false);
	m_pTableProperties->setColumnStretchable(1, true);

	m_mapGroups.clear();
	m_mapChildren.clear();
	UpdateTable();
	m_pTableProperties->adjustColumn(0);
}

void QConfigureDialog::UpdateTable()
{
	unsigned int i;
	int iRow, iIndents;
	QStringList currentGroupPath;
	QStringList::iterator iterCurrent, iterLast;
	QString strGroupName, strKey;
	std::map<QString, QExpandableTableItem *>::iterator iterGroup;
	std::map<QString, QTableItem *>::iterator iterChild;
	QExpandableTableItem * pCurrent;
	QTableItem * pItem;

	for (i = 0; i < SETTINGS_NUM; i++)
	{
		strGroupName = QString::null;
		currentGroupPath = QStringList::split('/', m_Settings.m_sSettings[i].GetKey(), false);
		iterLast = currentGroupPath.end();
		if (iterLast != currentGroupPath.begin())
			--iterLast;

		// get/add groups that this setting belongs to
		pCurrent = NULL;
		iIndents = 0;
		for (iterCurrent = currentGroupPath.begin(); iterCurrent != iterLast; ++iterCurrent)
		{
			strGroupName += ('/' + *iterCurrent);
			iterGroup = m_mapGroups.find(strGroupName);
			if (iterGroup == m_mapGroups.end()) {
				strKey = *iterCurrent;
				iterGroup = m_mapGroups.insert(std::pair<QString, QExpandableTableItem *>(strGroupName, new QExpandableTableItem(m_pTableProperties, QString().fill(' ', iIndents) + strKey))).first;
				if (pCurrent != NULL) {
					pCurrent->AddChild(strKey, iterGroup->second);
					iRow = pCurrent->row() + pCurrent->ChildCount();
				} else
					iRow = m_pTableProperties->numRows();
				m_pTableProperties->insertRows(iRow);
				m_pTableProperties->setItem(iRow, 0, iterGroup->second);
				m_pTableProperties->setRowReadOnly(iRow, true);
				if (pCurrent != NULL && (!pCurrent->IsExpanded() || m_pTableProperties->isRowHidden(pCurrent->row())))
					m_pTableProperties->hideRow(iRow);
			}
			pCurrent = iterGroup->second;
			iIndents += 2;
		}
		if (iterLast != currentGroupPath.end())
		{
			strKey = m_Settings.m_sSettings[i].GetLabel();
			if (pCurrent != NULL)
				pItem = pCurrent->GetChild(strKey);
			else {
				iterChild = m_mapChildren.find(strKey);
				pItem = (iterChild == m_mapChildren.end() ? NULL : iterChild->second);
			}
			if (pItem != NULL)
				m_Settings.m_sSettings[i].UpdateItem(pItem);
			else
			{
				pItem = m_Settings.m_sSettings[i].CreateItem(m_pTableProperties);
				if (pCurrent != NULL) {
					pCurrent->AddChild(strKey, pItem);
					iRow = pCurrent->row() + pCurrent->ChildCount();
				} else
					iRow = m_pTableProperties->numRows();
				m_pTableProperties->insertRows(iRow);
				m_pTableProperties->setText(iRow, 0, QString().fill(' ', iIndents) + strKey);
				m_pTableProperties->setItem(iRow, 1, pItem);
				if (pCurrent != NULL && (!pCurrent->IsExpanded() || m_pTableProperties->isRowHidden(pCurrent->row())))
					m_pTableProperties->hideRow(iRow);
			}
		}
	}
}

void QConfigureDialog::slotOK()
{
	unsigned int i;
	for (i = 0; i < SETTINGS_NUM; i++)
		g_pSettings->m_sSettings[i] = m_Settings.m_sSettings[i];
	accept();
}

void QConfigureDialog::slotApply()
{
	unsigned int i;
	for (i = 0; i < SETTINGS_NUM; i++)
		g_pSettings->m_sSettings[i] = m_Settings.m_sSettings[i];
	UpdateTable();
	if (g_pSimulator != NULL)
		g_pSimulator->TriggerSettingsChanged();

	m_pOK->setEnabled(false);
	m_pCancel->setText("&Close");
	m_pApply->setEnabled(false);
}

void QConfigureDialog::slotDefault()
{
	m_Settings.RestoreDefaults();
	UpdateTable();
	slotSettingsChanged(0, 0);
}

void QConfigureDialog::slotSettingsChanged(int row __attribute__ ((unused)) , int col __attribute__ ((unused)) )
{
	m_pOK->setEnabled(true);
	m_pCancel->setText("&Cancel");
	m_pApply->setEnabled(true);
}

void QConfigureDialog::slotTableDoubleClicked(int row, int col __attribute__ ((unused)) , int button __attribute__ ((unused)) , const QPoint & mousePos __attribute__ ((unused)) )
{
	QTableItem * pItem = m_pTableProperties->item(row, 0);
	if (pItem != NULL && pItem->rtti() == EXPANDABLETABLEITEM_RTTI_VALUE)
	{
		if (((QExpandableTableItem *)pItem)->IsExpanded())
			((QExpandableTableItem *)pItem)->Collapse();
		else
			((QExpandableTableItem *)pItem)->Expand();
	}
}

void QConfigureDialog::slotTableClicked(int row, int col, int button, const QPoint & mousePos __attribute__ ((unused)) )
{
	QTableItem * pItem = m_pTableProperties->item(row, col);
	if (pItem != NULL && button == Qt::RightButton)
	{
		switch (pItem->rtti())
		{
		case SETTINGTEXTTABLEITEM_RTTI_VALUE:
		{
			if (((QSettingTextTableItem *)pItem)->m_pSetting != NULL && ((QSettingTextTableItem *)pItem)->m_pSetting->GetType() == Setting::SettingTypeFile)
			{
				QString strFilename = QFileDialog::getOpenFileName(pItem->text(), QString::null, this, "filedialog", "GrooveNet - Choose Filename...");
				if (!strFilename.isNull()) {
					pItem->setText(strFilename);
					slotSettingsChanged(row, col);
					m_pTableProperties->updateCell(row, col);
				}
			}
			break;
		}
		default:
			break;
		}
	}
}

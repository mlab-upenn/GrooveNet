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

#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qlistbox.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qapplication.h>

#include "CarListVisual.h"
#include "CarRegistry.h"
#include "InfrastructureNodeRegistry.h"
#include "QTableVisualizer.h"
#include "QMapObjectTableItem.h"
#include "QMessageDialog.h"
#include "MainWindow.h"
#include "StringHelp.h"

CarListVisual::CarListVisual(const QString & strModelName)
: TableVisualizer(strModelName)
{
}

CarListVisual::CarListVisual(const CarListVisual & copy)
: TableVisualizer(copy), m_mapObjectsToRows(copy.m_mapObjectsToRows)
{
}

CarListVisual::~CarListVisual()
{
}

CarListVisual & CarListVisual::operator = (const CarListVisual & copy)
{
	TableVisualizer::operator =(copy);

	m_mapObjectsToRows = copy.m_mapObjectsToRows;

	return *this;
}

void CarListVisual::UpdateTable()
{
	std::map<in_addr_t, CarModel *> * pCarRegistry;
	std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry;
	std::map<in_addr_t, CarModel *>::iterator iterObject;
	std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterNodeObject;
	std::map<int, MapObject *> * pMapObjects;
	std::map<int, MapObject *>::iterator iterMapObject;
	std::pair<std::map<in_addr_t, int>::iterator, bool> pairObjectRow;
	std::vector<bool> vecUpdated;
	int i;
	QDraggingTable * pTable;

	// unable to update
	if (m_pWidget != NULL && (pTable = ((QTableVisualizer *)m_pWidget)->m_pTable) != NULL)
	{
		vecUpdated.resize(pTable->numRows());
		for (i = 0; i < pTable->numRows(); i++)
			vecUpdated[i] = false;
	
		pMapObjects = g_pMapObjects->acquireLock();
		pCarRegistry = g_pCarRegistry->acquireLock();
		for (iterObject = pCarRegistry->begin(); iterObject != pCarRegistry->end(); ++iterObject)
		{
			pairObjectRow = m_mapObjectsToRows.insert(std::pair<in_addr_t, int>(iterObject->first, pTable->numRows()));
			if (pairObjectRow.second)
			{
				pTable->insertRows(pairObjectRow.first->second);
				pTable->setItem(pairObjectRow.first->second, 0, new QMapObjectTableItem(NULL, QTableItem::Never, iterObject->second == NULL ? -1 : iterObject->second->GetMapObjectID()));
			}
			else
			{
				vecUpdated[pairObjectRow.first->second] = true;
				((QMapObjectTableItem*)pTable->item(pairObjectRow.first->second, 0))->SetID(iterObject->second == NULL ? -1 : iterObject->second->GetMapObjectID());
				pTable->updateCell(pairObjectRow.first->second, 0);
			}
	
			// update data for this row
			for (i = 1; i < pTable->numCols(); i++)
			{
				pTable->setText(pairObjectRow.first->second, i, iterObject->second == NULL ? "" : iterObject->second->GetCarListColumnText((CarListColumn)i));
			}
		}
		g_pCarRegistry->releaseLock();
	
		pNodeRegistry = g_pInfrastructureNodeRegistry->acquireLock();
		for (iterNodeObject = pNodeRegistry->begin(); iterNodeObject != pNodeRegistry->end(); ++iterNodeObject)
		{
			pairObjectRow = m_mapObjectsToRows.insert(std::pair<in_addr_t, int>(iterNodeObject->first, pTable->numRows()));
			if (pairObjectRow.second)
			{
				pTable->insertRows(pairObjectRow.first->second);
				pTable->setItem(pairObjectRow.first->second, 0, new QMapObjectTableItem(NULL, QTableItem::Never, iterNodeObject->second == NULL ? -1 : iterNodeObject->second->GetMapObjectID()));
			}
			else
			{
				vecUpdated[pairObjectRow.first->second] = true;
				((QMapObjectTableItem*)pTable->item(pairObjectRow.first->second, 0))->SetID(iterNodeObject->second == NULL ? -1 : iterNodeObject->second->GetMapObjectID());
				pTable->updateCell(pairObjectRow.first->second, 0);
			}
	
			// update data for this row
			for (i = 1; i < pTable->numCols(); i++)
			{
				pTable->setText(pairObjectRow.first->second, i, iterNodeObject->second == NULL ? "" : iterNodeObject->second->GetCarListColumnText((CarListColumn)i));
			}
		}
		g_pInfrastructureNodeRegistry->releaseLock();
		g_pMapObjects->releaseLock();
	
		for (i = 0; i < (signed)vecUpdated.size(); i++)
		{
			if (!vecUpdated[i])
			{
				pTable->removeRow(i);
				std::map<in_addr_t, int>::iterator iterObjectRow = m_mapObjectsToRows.begin();
				while (iterObjectRow != m_mapObjectsToRows.end()) {
					if (iterObjectRow->second == i)
					{
						std::map<in_addr_t, int>::iterator iterTemp = iterObjectRow;
						++iterObjectRow;
						m_mapObjectsToRows.erase(iterTemp);
						continue;
					}
	
					if (iterObjectRow->second > i)
						iterObjectRow->second--;
				}
			}
		}
	}
}

int CarListVisual::Init(const std::map<QString, QString> & mapParams)
{
	if (TableVisualizer::Init(mapParams))
		return 1;

	QDraggingTable * pTable = ((QTableVisualizer *)m_pWidget)->m_pTable;

	pTable->setNumCols(7);

	QHeader * pHHeader = pTable->horizontalHeader();
	pHHeader->setMovingEnabled(false);
	pHHeader->setLabel(0, "");
	pTable->setColumnWidth(0, 32);
	pHHeader->setLabel(1, "Vehicle");
	pTable->setColumnWidth(1, 120);
	pHHeader->setLabel(2, "Type");
	pTable->setColumnWidth(2, 150);
	pHHeader->setLabel(3, "Longitude");
	pTable->setColumnWidth(3, 120);
	pHHeader->setLabel(4, "Latitude");
	pTable->setColumnWidth(4, 120);
	pHHeader->setLabel(5, "Speed");
	pTable->setColumnWidth(5, 70);
	pHHeader->setLabel(6, "Heading");
	pTable->setColumnWidth(6, 90);
	pTable->verticalHeader()->hide();
	pTable->setColumnMovingEnabled(false);
	pTable->setFocusStyle(QTable::FollowStyle);
	pTable->setLeftMargin(0);
	pTable->setReadOnly(true);
	pTable->setRowMovingEnabled(false);
	pTable->setSelectionMode(QTable::SingleRow);
	pTable->setShowGrid(true);
	pTable->setSorting(false);
	pTable->SetDragObjectCreator(CarListVisualDragObjectCreator, this);
	pTable->setDragEnabled(true);
	m_pRightClickMenu = new QPopupMenu(pTable);
	m_pRightClickMenu->insertItem("Send Message...", 0);

	UpdateTable();

	return 0;
}

int CarListVisual::ProcessEvent(SimEvent & event)
{
	if (TableVisualizer::ProcessEvent(event))
		return 1;

	switch (event.GetEventID())
	{
	case EVENT_VISUALIZER_UPDATE:
		UpdateTable();
		break;
	default:
		break;
	}
	return 0;
}

int CarListVisual::PostRun()
{
	if (TableVisualizer::PostRun())
		return 1;

	UpdateTable();

	return 0;
}

int CarListVisual::Save(std::map<QString, QString> & mapParams)
{
	if (TableVisualizer::Save(mapParams))
		return 1;

	return 0;
}

int CarListVisual::Cleanup()
{
	QDraggingTable * pTable = m_pWidget == NULL ? NULL : ((QTableVisualizer *)m_pWidget)->m_pTable;
	if (pTable != NULL)
		pTable->setNumRows(0);

	if (TableVisualizer::Cleanup())
		return 1;

	m_mapObjectsToRows.clear();

	return 0;
}

void CarListVisual::tableContextMenuRequested(int row, int col __attribute__ ((unused)) , const QPoint & pos)
{
	int iResult;
	std::map<in_addr_t, int>::iterator iterRow;
	std::map<in_addr_t, CarModel *> * pCarRegistry;

	for (iterRow = m_mapObjectsToRows.begin(); iterRow != m_mapObjectsToRows.end(); ++iterRow)
	{
		if (iterRow->second == row)
			break;
	}

	if (iterRow == m_mapObjectsToRows.end())
		return;

	pCarRegistry = g_pCarRegistry->acquireLock();
	m_pRightClickMenu->setItemEnabled(0, pCarRegistry->find(iterRow->first) != pCarRegistry->end());
	g_pCarRegistry->releaseLock();
	iResult = m_pRightClickMenu->exec(pos);

	switch (iResult)
	{
	case 0:
	{
		QMessageDialog * pDialog = new QMessageDialog(iterRow->first, g_pMainWindow, 0, Qt::WDestructiveClose);
		pDialog->m_txtTXTime->setEnabled(false);
		pDialog->m_labelTXTime->hide();
		pDialog->m_txtTXTime->hide();
		pDialog->m_pfnAcceptCallback = DispatchMessage;
		pDialog->show();
		break;
	}
	default:
		break;
	}
}

void CarListVisual::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	TableVisualizer::GetParams(mapParams);
}

QDragObject * CarListVisualDragObjectCreator(TableVisualizer * pTableVisualizer)
{
	CarListVisual * pCarList = (CarListVisual *)pTableVisualizer;
	QDragObject * pDragObject = NULL;
	std::map<in_addr_t, int>::iterator iterRow;
	std::map<in_addr_t, CarModel *> * pCarRegistry;
	std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry;
	std::map<in_addr_t, CarModel *>::iterator iterCar;
	std::map<in_addr_t, InfrastructureNodeModel *>::iterator iterNode;
	int row = ((QTableVisualizer *)pCarList->GetWidget())->m_pTable->currentRow();

	for (iterRow = pCarList->m_mapObjectsToRows.begin(); iterRow != pCarList->m_mapObjectsToRows.end(); ++iterRow)
	{
		if (iterRow->second == row)
		{
			pCarRegistry = g_pCarRegistry->acquireLock();
			pNodeRegistry = g_pInfrastructureNodeRegistry->acquireLock();
			iterCar = pCarRegistry->find(iterRow->first);
			iterNode = pNodeRegistry->find(iterRow->first);
			if (iterCar != pCarRegistry->end() && iterCar->second != NULL)
			{
				pDragObject = new QTextDrag("OBJECT:" + iterCar->second->GetModelName(), ((QTableVisualizer *)pCarList->GetWidget())->m_pTable);
			}
			else if (iterNode != pNodeRegistry->end() && iterNode->second != NULL)
			{
				pDragObject = new QTextDrag("OBJECT:" + iterNode->second->GetModelName(), ((QTableVisualizer *)pCarList->GetWidget())->m_pTable);
			}
			g_pInfrastructureNodeRegistry->releaseLock();
			g_pCarRegistry->releaseLock();
			break;
		}
	}
	return pDragObject;
}

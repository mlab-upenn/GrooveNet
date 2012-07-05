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

#ifndef _QSIMCREATEDIALOG_H
#define _QSIMCREATEDIALOG_H

#include <qdialog.h>

#include "Model.h"

#include <vector>
#include <set>
#include <arpa/inet.h>

class QTabWidget;
class QPushButton;
class QListBox;
class QTable;
class QPopupMenu;
class QLabel;
class QLineEdit;
class QTextEdit;

class QSimCreateDialog : public QDialog
{
Q_OBJECT
public:
	QSimCreateDialog(QWidget * parent = NULL, const char * name = 0, bool modal = false, WFlags f = 0);
	virtual ~QSimCreateDialog();

	void AddExistingModels();

	std::vector<std::pair<QString, QString> > m_vecVehicles;
	std::vector<std::pair<QString, QString> > m_vecInfrastructureNodes;
	std::vector<std::pair<QString, QString> > m_vecModels;
	std::map<QString, std::map<QString, QString> > m_mapModels;

protected slots:
	virtual void slotVehiclesListHighlighted(int index);
	virtual void slotVehiclesListSelChanged();
	virtual void slotVehiclesAdd(int id);
	virtual void slotVehiclesRemove();
	virtual void slotVehiclesDuplicate();
	virtual void slotVehicleNameChanged(const QString & strName);
	virtual void slotVehiclePropertiesValueChanged(int row, int col);
	virtual void slotVehiclePropertiesCurrentChanged(int row, int col);
	virtual void slotVehiclesAutoGen();
	virtual void slotVehiclesRemoveAll();

	virtual void slotInfrastructureNodesListHighlighted(int index);
	virtual void slotInfrastructureNodesListSelChanged();
	virtual void slotInfrastructureNodesAdd(int id);
	virtual void slotInfrastructureNodesRemove();
	virtual void slotInfrastructureNodesDuplicate();
	virtual void slotInfrastructureNodeNameChanged(const QString & strName);
	virtual void slotInfrastructureNodePropertiesValueChanged(int row, int col);
	virtual void slotInfrastructureNodePropertiesCurrentChanged(int row, int col);
	virtual void slotInfrastructureNodesAutoGen();
	virtual void slotInfrastructureNodesRemoveAll();

	virtual void slotModelsListHighlighted(int index);
	virtual void slotModelsListSelChanged();
	virtual void slotModelsAdd(int id);
	virtual void slotModelsRemove();
	virtual void slotModelsDuplicate();
	virtual void slotModelNameChanged(const QString & strName);
	virtual void slotModelPropertiesValueChanged(int row, int col);
	virtual void slotModelPropertiesCurrentChanged(int row, int col);
	virtual void slotModelsRemoveAll();

	virtual void slotOK();

protected:
	in_addr_t GetNextIPAddress(in_addr_t ipStart);
	QString GetNextModelName(const QString & strBase);
	QString GetModelName(const QString & strLast = QString::null);
	void AddTableItem(QTable * pTable, int row, int col, const QString & strValue, const ModelParameter * pParam);
	bool ValidateTableItem(const QString & strValue, const ModelParameter * pParam);
	void RemoveAssocModels(const std::set<QString> & setModelNames);

	QTabWidget * m_tabsTabs;
	QPushButton * m_buttonOK, * m_buttonCancel, * m_buttonVehicleAdd, * m_buttonVehicleRemove, * m_buttonVehicleDuplicate, * m_buttonInfrastructureNodeAdd, * m_buttonInfrastructureNodeRemove, * m_buttonInfrastructureNodeDuplicate, * m_buttonModelAdd, * m_buttonModelRemove, * m_buttonModelDuplicate, * m_buttonVehicleRemoveAll, * m_buttonVehicleAutoGen, * m_buttonInfrastructureNodeRemoveAll, * m_buttonInfrastructureNodeAutoGen, * m_buttonModelRemoveAll;
	QListBox * m_listVehicles, * m_listInfrastructureNodes, * m_listModels;
	QLabel * m_lblVehicleName, * m_lblInfrastructureNodeName, * m_lblModelName;
	QLineEdit * m_txtVehicleName, * m_txtInfrastructureNodeName, * m_txtModelName;
	QTable * m_tableVehicleProperties, * m_tableInfrastructureNodeProperties, * m_tableModelProperties;
	QTextEdit * m_txtVehiclePropertiesHelp, * m_txtInfrastructureNodePropertiesHelp, * m_txtModelPropertiesHelp;
	QPopupMenu * m_pVehiclesAddMenu, * m_pInfrastructureNodesAddMenu, * m_pModelsAddMenu;

	std::map<QString, std::map<QString, ModelParameter> > m_mapModelParams;
	std::vector<QString> m_vecModelTypes;
	std::set<in_addr_t> m_setVehicleIPs;
	std::set<QString> m_setModelNames;
};

#endif

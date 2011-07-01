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

#ifndef _QAUTOGENDIALOG_H
#define _QAUTOGENDIALOG_H

#include <qdialog.h>

class QMapWidget;
class QPushButton;
class QComboBox;
class QSpinBox;
class QLabel;
class QTextEdit;
class QTable;

#include "Model.h"
#include "Coords.h"

#include <vector>
#include <set>
#include <arpa/inet.h>

typedef enum AutoGenParamTypeEnum
{
	AutoGenParamTypeFixed = 0,
	AutoGenParamTypeRandom,
	AutoGenParamTypeFile,
	AutoGenParamTypeDepends,
	AutoGenParamTypeIP,
	AutoGenParamTypeNewModel,
	AutoGenParamTypeNoRandom = 16
} AutoGenParamType;

typedef struct AutoGenParameterStruct
{
	QString strValue;
	AutoGenParamType eType;
	QString strAuxData;
} AutoGenParameter;

class QAutoGenDialog : public QDialog
{
Q_OBJECT
public:
	QAutoGenDialog(QWidget * parent = NULL, const char * name = 0, WFlags f = 0);
	virtual ~QAutoGenDialog();

	void SetParams(std::map<QString, std::map<QString, ModelParameter> > * pMapParams, std::vector<QString> * pVecModelTypes, const QString & strModelBaseType);

	std::map<QString, std::map<QString, QString> > * m_pModels;
	std::vector<std::pair<QString, QString> > * m_pVehicleList, * m_pModelList;
	std::set<in_addr_t> * m_pVehicleIPs;
	std::set<QString> * m_pModelNames;
	QString m_strIPAddressParam;

protected slots:
	virtual void slotVehicleTypeActivated(int index);
	virtual void slotParameterActivated(int index);
	virtual void slotRegionTypeActivated(int index);
	virtual void slotMapSelectionChanged();
	virtual void slotPropertiesValueChanged(int row, int col);
	virtual void slotPropertiesCurrentChanged(int row, int col);
	virtual void accept();

protected:
	bool AddTableItem(QTable * pTable, int row, AutoGenParameter & param, bool bUpdateTypes = false);
	void AddParameterFromModel(const QString & strParam, const ModelParameter & modelValue, AutoGenParameter & param);
	QString GetAssocModel(const QString & strParam, const QString & strType, std::map<QString, QString> & mapAssocModels, const std::map<QString, QString> & mapAssocModelTypes, std::map<QString, std::map<QString, AutoGenParameter> > & mapAssocParams, unsigned int iVehicle);
	QString GetRandomParameter(const QString & strParam, const QString & strValue, const ModelParameter & param);
	QString GetFileParameter(const QString & strParam, const QString & strFilename, unsigned int iVehicle);
	QString GetRandomAddress(const QString & strParam);
	QString GetRandomPosition(const QString & strParam);

	QMapWidget * m_pMap;
	QLabel * m_labelRegionInfo;
	QPushButton * m_buttonOK, * m_buttonCancel;
	QComboBox * m_comboVehicleType, * m_comboRegionIDs, * m_comboRegionType;
	QSpinBox * m_txtVehicles;
	QTable * m_tableProperties;
	QTextEdit * m_txtPropertiesHelp;
	std::map<QString, std::map<QString, ModelParameter> > * m_pMapParams;
	std::map<QString, unsigned int> m_mapModelIndexes;
	std::vector<QString> * m_pVecModelTypes;
	std::map<QString, std::map<QString, AutoGenParameter> > m_mapModels;
	std::map<QString, Rect > m_mapMapSelections;
	std::map<QString, std::vector<unsigned int> > m_mapRandomRecords;
	std::map<QString, std::vector<QString> > m_mapFileData;
};

#endif

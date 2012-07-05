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

#include "Settings.h"
#include "Global.h"
#include "StringHelp.h"
#include "QSettingTextTableItem.h"
#include "QSettingFileTableItem.h"
#include "QSettingColorTableItem.h"
#include "QSettingComboTableItem.h"

#define SETTINGS_COMPANY "CMU.EDU"
#define SETTINGS_PACKAGE PACKAGE_TITLE

Setting::Setting()
: m_eType(SettingTypeText), m_sValue(m_sDefault), m_pMin(NULL), m_pMax(NULL), m_vecEnum(NULL), m_iEnum(0)
{
}

Setting::Setting(const QString & strKey, SettingType eType, const SettingData & sDefault, const SettingData * pMin, const SettingData * pMax, const SettingData * vecEnum, unsigned int iEnum)
: m_strKey(strKey), m_eType(eType), m_sDefault(sDefault), m_sValue(sDefault), m_pMin(NULL), m_pMax(NULL), m_vecEnum(NULL), m_iEnum(iEnum)
{
	unsigned int i;
	if (pMin != NULL)
	{
		m_pMin = new SettingData(*pMin);
	}
	if (pMax != NULL)
	{
		m_pMax = new SettingData(*pMax);
	}
	if (vecEnum != NULL) {
		m_vecEnum = new SettingData[m_iEnum];
		for (i = 0; i < m_iEnum; i++)
			m_vecEnum[i] = vecEnum[i];
	}
}

Setting::Setting(const Setting & copy)
: m_listEnumStrings(copy.m_listEnumStrings), m_strKey(copy.m_strKey), m_eType(copy.m_eType), m_sDefault(copy.m_sDefault), m_sValue(copy.m_sValue), m_pMin(NULL), m_pMax(NULL), m_vecEnum(NULL), m_iEnum(copy.m_iEnum)
{
	unsigned int i;
	if (copy.m_pMin != NULL)
		m_pMin = new SettingData(*copy.m_pMin);
	if (copy.m_pMax != NULL)
		m_pMax = new SettingData(*copy.m_pMax);
	if (copy.m_vecEnum != NULL) {
		m_vecEnum = new SettingData[m_iEnum];
		for (i = 0; i < m_iEnum; i++)
			m_vecEnum[i] = copy.m_vecEnum[i];
	}
}

Setting::~Setting()
{
	if (m_pMin != NULL)
		delete m_pMin;
	if (m_pMax != NULL)
		delete m_pMax;
	if (m_vecEnum != NULL)
		delete[] m_vecEnum;
}

Setting & Setting::operator = (const Setting & copy)
{
	SettingData * pMin = copy.m_pMin, * pMax = copy.m_pMax, * vecEnum = copy.m_vecEnum;
	unsigned int i;

	m_listEnumStrings = copy.m_listEnumStrings;
	m_strKey = copy.m_strKey;
	m_eType = copy.m_eType;
	m_sDefault = copy.m_sDefault;
	m_sValue = copy.m_sValue;
	m_iEnum = copy.m_iEnum;
	if (pMin != NULL)
		pMin = new SettingData(*pMin);
	if (pMax != NULL)
		pMin = new SettingData(*pMin);
	if (vecEnum != NULL) {
		vecEnum = new SettingData[m_iEnum];
		for (i = 0; i < m_iEnum; i++)
			vecEnum[i] = copy.m_vecEnum[i];
	}
	if (m_pMin != NULL)
		delete m_pMin;
	if (m_pMax != NULL)
		delete m_pMax;
	if (m_vecEnum != NULL)
		delete[] m_vecEnum;
	m_pMin = pMin;
	m_pMax = pMax;
	m_vecEnum = vecEnum;
	return *this;
}

bool Setting::IsValid(const SettingData & sValue) const
{
	bool bValid;
	unsigned int i;
	if ((m_eType & SettingTypeList) == SettingTypeList)
	{
		bValid = false;
		for (i = 0; i < m_iEnum; i++)
		{
			if (Compare(m_vecEnum[i], sValue) == 0) {
				bValid = true;
				break;
			}
		}
	}
	else if ((m_eType & SettingTypeRange) == SettingTypeRange)
	{
		bValid = (m_pMin == NULL || Compare(*m_pMin, sValue) <= 0) && (m_pMax == NULL || Compare(*m_pMax, sValue) >= 0);
	}
	else
		bValid = true;

	if (bValid)
	{
		in_addr_t ipAddress;
		switch (m_eType & 0xF)
		{
		case SettingTypeText:
		case SettingTypeFile:
			return true;
		case SettingTypeColor:
			return QColor(sValue.strValue).isValid();
		case SettingTypeIP:
			return StringToIPAddress(sValue.strValue, ipAddress);
		case SettingTypeBool:
		case SettingTypeInt:
		case SettingTypeFloat:
			return true;
		default:
			return false;
		}
	} else
		return false;
}

bool Setting::SetValue(const SettingData & sValue)
{
	bool bValid;
	in_addr_t ipValue;
	if (bValid = IsValid(sValue))
	{
		switch (m_eType & 0xF)
		{
		case SettingTypeText:
		case SettingTypeFile:
			m_sValue.strValue = sValue.strValue;
			break;
		case SettingTypeColor:
		{
			QColor clr(sValue.strValue);
			m_sValue.strValue = clr.isValid() ? clr.name() : "";
			break;
		}
		case SettingTypeIP:
			StringToIPAddress(sValue.strValue, ipValue);
			m_sValue.strValue = IPAddressToString(ipValue);
			break;
		case SettingTypeBool:
			m_sValue.bValue = sValue.bValue;
			break;
		case SettingTypeInt:
			m_sValue.iValue = sValue.iValue;
			break;
		case SettingTypeFloat:
			m_sValue.fValue = sValue.fValue;
			break;
		default:
			return false;
		}
	}
	return bValid;
}

int Setting::Compare(const SettingData & x, const SettingData & y) const
{
	in_addr_t ipX, ipY;
	bool bValidX, bValidY;
	switch (m_eType & 0xF)
	{
	case SettingTypeText:
	case SettingTypeFile:
		return x.strValue.compare(y.strValue);
	case SettingTypeColor:
		return strcasecmp((const char *)x.strValue, (const char *)y.strValue);
	case SettingTypeIP:
		bValidX = StringToIPAddress(x.strValue, ipX);
		bValidY = StringToIPAddress(y.strValue, ipY);
		if (bValidX && bValidY)
		{
			ipX = htonl(ipX);
			ipY = htonl(ipY);
			return ipX - ipY;
		}
		else if (bValidX)
			return 1;
		else if (bValidY)
			return -1;
		else
			return 0;
	case SettingTypeBool:
		if (x.bValue)
			return y.bValue ? 0 : 1;
		else
			return y.bValue ? -1 : 0;
	case SettingTypeInt:
		return x.iValue - y.iValue;
	case SettingTypeFloat:
		if (x.fValue < y.fValue)
			return -1;
		else if (x.fValue == y.fValue)
			return 0;
		else
			return 1;
	default:
		return 0;
	}
}

QTableItem * Setting::CreateItem(QTable * pTable)
{
	unsigned int i;
	QSettingComboTableItem * pComboItem;
	QStringList listStrings;
	int iCurrentItem = -1;
	if ((m_eType & SettingTypeList) == SettingTypeList)
	{
		for (i = 0; i < m_iEnum; i++)
		{
			if (Compare(m_sValue, m_vecEnum[i]) == 0) {
				iCurrentItem = i;
				break;
			}
		}
		pComboItem = new QSettingComboTableItem(pTable, m_listEnumStrings, false, this);
		pComboItem->setCurrentItem(iCurrentItem);
		return pComboItem;
	}
	else
	{
		switch (m_eType & 0xF)
		{
		case SettingTypeText:
		case SettingTypeIP:
			return new QSettingTextTableItem(pTable, this);
		case SettingTypeFile:
			return new QSettingFileTableItem(pTable, this);
		case SettingTypeColor:
			return new QSettingColorTableItem(pTable, this);
		case SettingTypeBool:
			listStrings.push_back("True");
			listStrings.push_back("False");
			pComboItem = new QSettingComboTableItem(pTable, listStrings, false, this);
			pComboItem->setCurrentItem(m_sValue.bValue ? 0 : 1);
			return pComboItem;
		case SettingTypeInt:
		case SettingTypeFloat:
		default:
			return new QSettingTextTableItem(pTable, this);
		}
	}
}

void Setting::UpdateItem(QTableItem * pItem)
{
	unsigned int i;
	QSettingComboTableItem * pComboItem;
	QStringList listStrings;
	int iCurrentItem = -1;
	if ((m_eType & SettingTypeList) == SettingTypeList)
	{
		pComboItem = (QSettingComboTableItem *)pItem;
		for (i = 0; i < m_iEnum; i++)
		{
			if (Compare(m_sValue, m_vecEnum[i]) == 0) {
				iCurrentItem = i;
				break;
			}
		}
		pComboItem->setCurrentItem(iCurrentItem);
	}
	else
	{
		switch (m_eType & 0xF)
		{
		case SettingTypeBool:
			pComboItem = (QSettingComboTableItem *)pItem;
			pComboItem->setCurrentItem(m_sValue.bValue ? 0 : 1);
			break;
		case SettingTypeInt:
			pItem->setText(QString("%1").arg(m_sValue.iValue));
			break;
		case SettingTypeFloat:
			pItem->setText(QString("%1").arg(m_sValue.fValue));
			break;
		default:
			pItem->setText(m_sValue.strValue);
			break;
		}
	}
	pItem->table()->updateCell(pItem->row(), pItem->col());
}


Settings::Settings(int argc, char ** argv, QSettings * pAppSettings)
: m_pAppSettings(pAppSettings)
{
	Setting::SettingData sDefault, * vecEnum;
	QStringList listStrings;
	unsigned int i, iEnum;

	ExtractParamsFromCmdLine(argc, argv);

	if (m_pAppSettings)
		m_pAppSettings->setPath(SETTINGS_COMPANY, SETTINGS_PACKAGE);

	sDefault.bValue = SETTINGS_GENERAL_LOGGING_DEFAULT;
	m_sSettings[SETTINGS_GENERAL_LOGGING_NUM] = Setting(SETTINGS_GENERAL_LOGGING, Setting::SettingTypeBool, sDefault);

	sDefault.bValue = SETTINGS_GENERAL_LOADMAPS_DEFAULT;
	m_sSettings[SETTINGS_GENERAL_LOADMAPS_NUM] = Setting(SETTINGS_GENERAL_LOADMAPS, Setting::SettingTypeBool, sDefault);

	sDefault.bValue = SETTINGS_GENERAL_NETWORK_DEFAULT;
	m_sSettings[SETTINGS_GENERAL_NETWORK_NUM] = Setting(SETTINGS_GENERAL_NETWORK, Setting::SettingTypeBool, sDefault);

	sDefault.iValue = SETTINGS_GENERAL_ADDRESSES_DEFAULT;
	m_sSettings[SETTINGS_GENERAL_ADDRESSES_NUM] = Setting(SETTINGS_GENERAL_ADDRESSES, Setting::SettingTypeInt, sDefault);

	sDefault.bValue = SETTINGS_APPEARANCE_FILLWATER_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_FILLWATER_NUM] = Setting(SETTINGS_APPEARANCE_FILLWATER, Setting::SettingTypeBool, sDefault);

	sDefault.bValue = SETTINGS_APPEARANCE_SHOWCOMPASS_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_SHOWCOMPASS_NUM] = Setting(SETTINGS_APPEARANCE_SHOWCOMPASS, Setting::SettingTypeBool, sDefault);

	sDefault.bValue = SETTINGS_APPEARANCE_SHOWSCALE_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_SHOWSCALE_NUM] = Setting(SETTINGS_APPEARANCE_SHOWSCALE, Setting::SettingTypeBool, sDefault);

	sDefault.bValue = SETTINGS_APPEARANCE_SHOWMARKERS_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_SHOWMARKERS_NUM] = Setting(SETTINGS_APPEARANCE_SHOWMARKERS, Setting::SettingTypeBool, sDefault);

	sDefault.bValue = SETTINGS_APPEARANCE_SHOWTRACKS_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_SHOWTRACKS_NUM] = Setting(SETTINGS_APPEARANCE_SHOWTRACKS, Setting::SettingTypeBool, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_BG_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_BG_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_BG, Setting::SettingTypeColor, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_FG_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_FG_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_FG, Setting::SettingTypeColor, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_SMROAD_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_SMROAD_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_SMROAD, Setting::SettingTypeColor, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_LGROAD_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_LGROAD_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_LGROAD, Setting::SettingTypeColor, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_PRIMARY_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_PRIMARY_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_PRIMARY, Setting::SettingTypeColor, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_HIGHWAY_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_HIGHWAY_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_HIGHWAY, Setting::SettingTypeColor, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_RAIL_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_RAIL_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_RAIL, Setting::SettingTypeColor, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_WATER_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_WATER_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_WATER, Setting::SettingTypeColor, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_LANDBOUNDS_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_LANDBOUNDS_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_LANDBOUNDS, Setting::SettingTypeColor, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_WATERBOUNDS_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_WATERBOUNDS_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_WATERBOUNDS, Setting::SettingTypeColor, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_MISCBOUNDS_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_MISCBOUNDS_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_MISCBOUNDS, Setting::SettingTypeColor, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_LANDMARK_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_LANDMARK_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_LANDMARK, Setting::SettingTypeColor, sDefault);

	sDefault.strValue = SETTINGS_APPEARANCE_COLOR_PHYSICAL_DEFAULT;
	m_sSettings[SETTINGS_APPEARANCE_COLOR_PHYSICAL_NUM] = Setting(SETTINGS_APPEARANCE_COLOR_PHYSICAL, Setting::SettingTypeColor, sDefault);

	sDefault.bValue = SETTINGS_NETWORK_AUTOSERVER_DEFAULT;
	m_sSettings[SETTINGS_NETWORK_AUTOSERVER_NUM] = Setting(SETTINGS_NETWORK_AUTOSERVER, Setting::SettingTypeBool, sDefault);

	sDefault.strValue = SETTINGS_NETWORK_INITFILE_DEFAULT;
	m_sSettings[SETTINGS_NETWORK_INITFILE_NUM] = Setting(SETTINGS_NETWORK_INITFILE, Setting::SettingTypeFile, sDefault);

	sDefault.strValue = SETTINGS_NETWORK_INITARGS_DEFAULT;
	m_sSettings[SETTINGS_NETWORK_INITARGS_NUM] = Setting(SETTINGS_NETWORK_INITARGS, Setting::SettingTypeText, sDefault);

	sDefault.strValue = SETTINGS_NETWORK_CLOSEFILE_DEFAULT;
	m_sSettings[SETTINGS_NETWORK_CLOSEFILE_NUM] = Setting(SETTINGS_NETWORK_CLOSEFILE, Setting::SettingTypeFile, sDefault);

	sDefault.strValue = SETTINGS_NETWORK_CLOSEARGS_DEFAULT;
	m_sSettings[SETTINGS_NETWORK_CLOSEARGS_NUM] = Setting(SETTINGS_NETWORK_CLOSEARGS, Setting::SettingTypeText, sDefault);

	sDefault.strValue = SETTINGS_NETWORK_DEVICENAME_DEFAULT;
	m_sSettings[SETTINGS_NETWORK_DEVICENAME_NUM] = Setting(SETTINGS_NETWORK_DEVICENAME, Setting::SettingTypeText, sDefault);

	sDefault.strValue = SETTINGS_NETWORK_DEVICETYPE_DEFAULT;
	iEnum = SETTINGS_NETWORK_DEVICETYPE_LIST_SIZE;
	vecEnum = new Setting::SettingData[iEnum];
	for (i = 0; i < iEnum; i++) {
		vecEnum[i].strValue = SETTINGS_NETWORK_DEVICETYPES[i];
		listStrings.push_back(SETTINGS_NETWORK_DEVICETYPE_LIST[i]);
	}
	m_sSettings[SETTINGS_NETWORK_DEVICETYPE_NUM] = Setting(SETTINGS_NETWORK_DEVICETYPE, (Setting::SettingType)(Setting::SettingTypeText | Setting::SettingTypeList), sDefault, NULL, NULL, vecEnum, iEnum);
	m_sSettings[SETTINGS_NETWORK_DEVICETYPE_NUM].m_listEnumStrings = listStrings;
	delete[] vecEnum;
	listStrings.clear();

	sDefault.strValue = SETTINGS_NETWORK_IPADDRESS_DEFAULT;
	m_sSettings[SETTINGS_NETWORK_IPADDRESS_NUM] = Setting(SETTINGS_NETWORK_IPADDRESS, Setting::SettingTypeIP, sDefault);

	sDefault.strValue = SETTINGS_NETWORK_IPSUBNET_DEFAULT;
	m_sSettings[SETTINGS_NETWORK_IPSUBNET_NUM] = Setting(SETTINGS_NETWORK_IPSUBNET, Setting::SettingTypeIP, sDefault);

	sDefault.iValue = SETTINGS_NETWORK_TXPOWER_DEFAULT;
	m_sSettings[SETTINGS_NETWORK_TXPOWER_NUM] = Setting(SETTINGS_NETWORK_TXPOWER, Setting::SettingTypeInt, sDefault);

	sDefault.iValue = SETTINGS_NETWORK_TXRATE_DEFAULT;
	m_sSettings[SETTINGS_NETWORK_TXRATE_NUM] = Setting(SETTINGS_NETWORK_TXRATE, Setting::SettingTypeInt, sDefault);

	sDefault.iValue = SETTINGS_HYBRIDNETWORK_MODE_DEFAULT;
	iEnum = SETTINGS_HYBRIDNETWORK_MODE_LIST_SIZE;
	vecEnum = new Setting::SettingData[iEnum];
	for (i = 0; i < iEnum; i++) {
		vecEnum[i].iValue = SETTINGS_HYBRIDNETWORK_MODES[i];
		listStrings.push_back(SETTINGS_HYBRIDNETWORK_MODE_LIST[i]);
	}
	m_sSettings[SETTINGS_HYBRIDNETWORK_MODE_NUM] = Setting(SETTINGS_HYBRIDNETWORK_MODE, (Setting::SettingType)(Setting::SettingTypeInt | Setting::SettingTypeList), sDefault, NULL, NULL, vecEnum, iEnum);
	m_sSettings[SETTINGS_HYBRIDNETWORK_MODE_NUM].m_listEnumStrings = listStrings;
	delete[] vecEnum;
	listStrings.clear();

/*
	sDefault.iValue = SETTINGS_HYBRIDNETWORK_MODE_DEFAULT;
	m_sSettings[SETTINGS_HYBRIDNETWORK_MODE_NUM] = Setting(SETTINGS_HYBRIDNETWORK_MODE, Setting::SettingTypeInt, sDefault);
*/
	sDefault.strValue = SETTINGS_HYBRIDNETWORK_IP_DEFAULT;
	m_sSettings[SETTINGS_HYBRIDNETWORK_IP_NUM] = Setting(SETTINGS_HYBRIDNETWORK_IP, Setting::SettingTypeIP, sDefault);

fflush(stdout);
}

Settings::Settings(const Settings & copy)
: m_sSettings(copy.m_sSettings), m_listAddressHistory(copy.m_listAddressHistory), m_pAppSettings(copy.m_pAppSettings), m_mapCmdLineParams(copy.m_mapCmdLineParams)
{
}

Settings::~Settings()
{
}

Settings & Settings::operator = (const Settings & copy)
{
	unsigned int i;
	for (i = 0; i < SETTINGS_NUM; i++)
		m_sSettings[i] = copy.m_sSettings[i];
	m_listAddressHistory = copy.m_listAddressHistory;
	m_pAppSettings = copy.m_pAppSettings;
	m_mapCmdLineParams = copy.m_mapCmdLineParams;
	return *this;
}

void Settings::RestoreDefaults()
{
	unsigned int i;
	for (i = 0; i < SETTINGS_NUM; i++)
		m_sSettings[i].SetDefaultValue();
}

void Settings::ReadSettings()
{
	unsigned int i, iAddressHistorySize;
	Setting::SettingData sValue;

	for (i = 0; i < SETTINGS_NUM; i++)
	{
		switch (m_sSettings[i].GetType() & 0xF)
		{
		case Setting::SettingTypeText:
		case Setting::SettingTypeColor:
		case Setting::SettingTypeFile:
		case Setting::SettingTypeIP:
			sValue.strValue = m_pAppSettings->readEntry("/" SETTINGS_PACKAGE + m_sSettings[i].GetKey(), m_sSettings[i].GetDefault().strValue);
			break;
		case Setting::SettingTypeBool:
			sValue.bValue = m_pAppSettings->readBoolEntry("/" SETTINGS_PACKAGE + m_sSettings[i].GetKey(), m_sSettings[i].GetDefault().bValue);
			break;
		case Setting::SettingTypeInt:
			sValue.iValue = m_pAppSettings->readNumEntry("/" SETTINGS_PACKAGE + m_sSettings[i].GetKey(), m_sSettings[i].GetDefault().iValue);
			break;
		case Setting::SettingTypeFloat:
			sValue.fValue = m_pAppSettings->readDoubleEntry("/" SETTINGS_PACKAGE + m_sSettings[i].GetKey(), m_sSettings[i].GetDefault().fValue);
			break;
		default:
			break;
		}
		if (!m_sSettings[i].SetValue(sValue))
			m_sSettings[i].SetDefaultValue();
	}

	iAddressHistorySize = m_sSettings[SETTINGS_GENERAL_ADDRESSES_NUM].GetValue().iValue;
	m_listAddressHistory.clear();
	for (i = 0; i < iAddressHistorySize; i++)
	{
		m_listAddressHistory.push_back(m_pAppSettings->readEntry("/" SETTINGS_PACKAGE + QString("%1%2").arg(SETTINGS_ADDRESSES_ADDRESSBASE).arg(i+1), ""));
	}
}

void Settings::WriteSettings()
{
	unsigned int i, iAddressHistorySize;
	std::list<QString>::iterator iterAddress;
	for (i = 0; i < SETTINGS_NUM; i++)
	{
		switch (m_sSettings[i].GetType() & 0xF)
		{
		case Setting::SettingTypeText:
		case Setting::SettingTypeColor:
		case Setting::SettingTypeFile:
		case Setting::SettingTypeIP:
			m_pAppSettings->writeEntry("/" SETTINGS_PACKAGE + m_sSettings[i].GetKey(), m_sSettings[i].GetValue().strValue);
			break;
		case Setting::SettingTypeBool:
			m_pAppSettings->writeEntry("/" SETTINGS_PACKAGE + m_sSettings[i].GetKey(), m_sSettings[i].GetValue().bValue);
			break;
		case Setting::SettingTypeInt:
			m_pAppSettings->writeEntry("/" SETTINGS_PACKAGE + m_sSettings[i].GetKey(), (int)m_sSettings[i].GetValue().iValue);
			break;
		case Setting::SettingTypeFloat:
			m_pAppSettings->writeEntry("/" SETTINGS_PACKAGE + m_sSettings[i].GetKey(), m_sSettings[i].GetValue().fValue);
			break;
		default:
			break;
		}
	}

	iAddressHistorySize = m_sSettings[SETTINGS_GENERAL_ADDRESSES_NUM].GetValue().iValue;
	for (i = 0, iterAddress = m_listAddressHistory.begin(); i < iAddressHistorySize && iterAddress != m_listAddressHistory.end(); i++, ++iterAddress)
	{
		m_pAppSettings->writeEntry("/" SETTINGS_PACKAGE + QString("%1%2").arg(SETTINGS_ADDRESSES_ADDRESSBASE).arg(i+1), *iterAddress);
	}
}

void Settings::AddAddress(const QString & strAddress)
{
	unsigned int iAddressHistorySize = m_sSettings[SETTINGS_GENERAL_ADDRESSES_NUM].GetValue().iValue;
	m_listAddressHistory.remove(strAddress);
	m_listAddressHistory.push_front(strAddress);
	if (m_listAddressHistory.size() > iAddressHistorySize)
		m_listAddressHistory.resize(iAddressHistorySize);
}

void Settings::ExtractParamsFromCmdLine(int argc, char ** argv)
{
	int i, iSep;
	QString strArg, strKey, strValue;

	m_mapCmdLineParams.clear();
	for (i = 1; i < argc; i++)
	{
		strArg = QString(argv[i]).stripWhiteSpace();
		iSep = strArg.find('=');
		if (iSep < 0)
			m_mapCmdLineParams[strArg] = QString::null;
		else
			m_mapCmdLineParams[strArg.left(iSep)] = strArg.mid(iSep + 1);
	}
}

const QString & Settings::GetParam(const QString & strKey, const QString & strDefault, bool bAllowEmpty) const
{
	std::map<QString, QString>::const_iterator iterParam = m_mapCmdLineParams.find(strKey);
	return iterParam == m_mapCmdLineParams.end() || (!bAllowEmpty && iterParam->second.isEmpty()) ? strDefault : iterParam->second;
}

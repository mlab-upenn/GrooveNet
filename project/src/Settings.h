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

#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <qsettings.h>
#include <qtable.h>

#define SETTINGS_NUM_START 0

#define SETTINGS_GENERAL_LOGGING "/General/Enable Logging"
#define SETTINGS_GENERAL_LOGGING_DEFAULT true
#define SETTINGS_GENERAL_LOGGING_NUM SETTINGS_NUM_START

#define SETTINGS_GENERAL_LOADMAPS "/General/Autoload Maps"
#define SETTINGS_GENERAL_LOADMAPS_DEFAULT true
#define SETTINGS_GENERAL_LOADMAPS_NUM (SETTINGS_GENERAL_LOGGING_NUM+1)

#define SETTINGS_GENERAL_NETWORK "/General/Autostart Network"
#define SETTINGS_GENERAL_NETWORK_DEFAULT true
#define SETTINGS_GENERAL_NETWORK_NUM (SETTINGS_GENERAL_LOADMAPS_NUM+1)

#define SETTINGS_GENERAL_ADDRESSES "/General/Address History Size"
#define SETTINGS_GENERAL_ADDRESSES_DEFAULT 10
#define SETTINGS_GENERAL_ADDRESSES_NUM (SETTINGS_GENERAL_NETWORK_NUM+1)

#define SETTINGS_APPEARANCE_FILLWATER "/Appearance/Fill Water"
#define SETTINGS_APPEARANCE_FILLWATER_DEFAULT true
#define SETTINGS_APPEARANCE_FILLWATER_NUM (SETTINGS_GENERAL_ADDRESSES_NUM+1)

#define SETTINGS_APPEARANCE_SHOWCOMPASS "/Appearance/Show Compass"
#define SETTINGS_APPEARANCE_SHOWCOMPASS_DEFAULT true
#define SETTINGS_APPEARANCE_SHOWCOMPASS_NUM (SETTINGS_APPEARANCE_FILLWATER_NUM+1)

#define SETTINGS_APPEARANCE_SHOWSCALE "/Appearance/Show Scale"
#define SETTINGS_APPEARANCE_SHOWSCALE_DEFAULT true
#define SETTINGS_APPEARANCE_SHOWSCALE_NUM (SETTINGS_APPEARANCE_SHOWCOMPASS_NUM+1)

#define SETTINGS_APPEARANCE_SHOWMARKERS "/Appearance/Show Markers"
#define SETTINGS_APPEARANCE_SHOWMARKERS_DEFAULT true
#define SETTINGS_APPEARANCE_SHOWMARKERS_NUM (SETTINGS_APPEARANCE_SHOWSCALE_NUM+1)

#define SETTINGS_APPEARANCE_SHOWTRACKS "/Appearance/Show Tracks"
#define SETTINGS_APPEARANCE_SHOWTRACKS_DEFAULT true
#define SETTINGS_APPEARANCE_SHOWTRACKS_NUM (SETTINGS_APPEARANCE_SHOWMARKERS_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_BG "/Appearance/Color/Background"
#define SETTINGS_APPEARANCE_COLOR_BG_DEFAULT "#ffffff"
#define SETTINGS_APPEARANCE_COLOR_BG_NUM (SETTINGS_APPEARANCE_SHOWTRACKS_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_FG "/Appearance/Color/Foreground"
#define SETTINGS_APPEARANCE_COLOR_FG_DEFAULT "#c0c0c0"
#define SETTINGS_APPEARANCE_COLOR_FG_NUM (SETTINGS_APPEARANCE_COLOR_BG_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_SMROAD "/Appearance/Color/Small Roads"
#define SETTINGS_APPEARANCE_COLOR_SMROAD_DEFAULT "#a0a0a0"
#define SETTINGS_APPEARANCE_COLOR_SMROAD_NUM (SETTINGS_APPEARANCE_COLOR_FG_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_LGROAD "/Appearance/Color/Large Roads"
#define SETTINGS_APPEARANCE_COLOR_LGROAD_DEFAULT "#827000"
#define SETTINGS_APPEARANCE_COLOR_LGROAD_NUM (SETTINGS_APPEARANCE_COLOR_SMROAD_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_PRIMARY "/Appearance/Color/Primary Roads"
#define SETTINGS_APPEARANCE_COLOR_PRIMARY_DEFAULT "#b68628"
#define SETTINGS_APPEARANCE_COLOR_PRIMARY_NUM (SETTINGS_APPEARANCE_COLOR_LGROAD_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_HIGHWAY "/Appearance/Color/Highways"
#define SETTINGS_APPEARANCE_COLOR_HIGHWAY_DEFAULT "#ffaa00"
#define SETTINGS_APPEARANCE_COLOR_HIGHWAY_NUM (SETTINGS_APPEARANCE_COLOR_PRIMARY_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_RAIL "/Appearance/Color/Railways"
#define SETTINGS_APPEARANCE_COLOR_RAIL_DEFAULT "#808080"
#define SETTINGS_APPEARANCE_COLOR_RAIL_NUM (SETTINGS_APPEARANCE_COLOR_HIGHWAY_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_WATER "/Appearance/Color/Water"
#define SETTINGS_APPEARANCE_COLOR_WATER_DEFAULT "#0000ff"
#define SETTINGS_APPEARANCE_COLOR_WATER_NUM (SETTINGS_APPEARANCE_COLOR_RAIL_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_LANDBOUNDS "/Appearance/Color/Land Boundaries"
#define SETTINGS_APPEARANCE_COLOR_LANDBOUNDS_DEFAULT "#a0a0a0"
#define SETTINGS_APPEARANCE_COLOR_LANDBOUNDS_NUM (SETTINGS_APPEARANCE_COLOR_WATER_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_WATERBOUNDS "/Appearance/Color/Water Boundaries"
#define SETTINGS_APPEARANCE_COLOR_WATERBOUNDS_DEFAULT "#c0c0ff"
#define SETTINGS_APPEARANCE_COLOR_WATERBOUNDS_NUM (SETTINGS_APPEARANCE_COLOR_LANDBOUNDS_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_MISCBOUNDS "/Appearance/Color/Misc. Boundaries"
#define SETTINGS_APPEARANCE_COLOR_MISCBOUNDS_DEFAULT "#c0c0ff"
#define SETTINGS_APPEARANCE_COLOR_MISCBOUNDS_NUM (SETTINGS_APPEARANCE_COLOR_WATERBOUNDS_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_LANDMARK "/Appearance/Color/Landmarks"
#define SETTINGS_APPEARANCE_COLOR_LANDMARK_DEFAULT "#2faf41"
#define SETTINGS_APPEARANCE_COLOR_LANDMARK_NUM (SETTINGS_APPEARANCE_COLOR_MISCBOUNDS_NUM+1)

#define SETTINGS_APPEARANCE_COLOR_PHYSICAL "/Appearance/Color/Physical Features"
#define SETTINGS_APPEARANCE_COLOR_PHYSICAL_DEFAULT "#000000"
#define SETTINGS_APPEARANCE_COLOR_PHYSICAL_NUM (SETTINGS_APPEARANCE_COLOR_LANDMARK_NUM+1)

#define SETTINGS_NETWORK_AUTOSERVER "/Network/Autostart Server"
#define SETTINGS_NETWORK_AUTOSERVER_DEFAULT true
#define SETTINGS_NETWORK_AUTOSERVER_NUM (SETTINGS_APPEARANCE_COLOR_PHYSICAL_NUM+1)

#define SETTINGS_NETWORK_INITFILE "/Network/Startup Script"
#define SETTINGS_NETWORK_INITFILE_DEFAULT ""
#define SETTINGS_NETWORK_INITFILE_NUM (SETTINGS_NETWORK_AUTOSERVER_NUM+1)

#define SETTINGS_NETWORK_INITARGS "/Network/Startup Script Arguments"
#define SETTINGS_NETWORK_INITARGS_DEFAULT ""
#define SETTINGS_NETWORK_INITARGS_NUM (SETTINGS_NETWORK_INITFILE_NUM+1)

#define SETTINGS_NETWORK_CLOSEFILE "/Network/Shutdown Script"
#define SETTINGS_NETWORK_CLOSEFILE_DEFAULT ""
#define SETTINGS_NETWORK_CLOSEFILE_NUM (SETTINGS_NETWORK_INITARGS_NUM+1)

#define SETTINGS_NETWORK_CLOSEARGS "/Network/Shutdown Script Arguments"
#define SETTINGS_NETWORK_CLOSEARGS_DEFAULT ""
#define SETTINGS_NETWORK_CLOSEARGS_NUM (SETTINGS_NETWORK_CLOSEFILE_NUM+1)

#define SETTINGS_NETWORK_DEVICENAME "/Network/Adapter Name"
#define SETTINGS_NETWORK_DEVICENAME_DEFAULT "eth0"
#define SETTINGS_NETWORK_DEVICENAME_NUM (SETTINGS_NETWORK_CLOSEARGS_NUM+1)

#define SETTINGS_NETWORK_DEVICETYPE "/Network/Adapter Type"
#define SETTINGS_NETWORK_DEVICETYPE_LIST_SIZE 4
const QString SETTINGS_NETWORK_DEVICETYPES[SETTINGS_NETWORK_DEVICETYPE_LIST_SIZE] = {"atheros", "denso", "intel", ""};
const QString SETTINGS_NETWORK_DEVICETYPE_LIST[SETTINGS_NETWORK_DEVICETYPE_LIST_SIZE] = {"Atheros-compatible", "Denso DSRC Kit", "Intel-compatible", "Unknown/Unsupported"};
#define SETTINGS_NETWORK_DEVICETYPE_DEFAULT (SETTINGS_NETWORK_DEVICETYPES[3])
#define SETTINGS_NETWORK_DEVICETYPE_LABEL "Adapter Type"
#define SETTINGS_NETWORK_DEVICETYPE_NUM (SETTINGS_NETWORK_DEVICENAME_NUM+1)

#define SETTINGS_NETWORK_IPADDRESS "/Network/IP Address"
#define SETTINGS_NETWORK_IPADDRESS_DEFAULT "192.168.0.1"
#define SETTINGS_NETWORK_IPADDRESS_NUM (SETTINGS_NETWORK_DEVICETYPE_NUM+1)

#define SETTINGS_NETWORK_IPSUBNET "/Network/IP Subnet Mask"
#define SETTINGS_NETWORK_IPSUBNET_DEFAULT "255.255.0.0"
#define SETTINGS_NETWORK_IPSUBNET_NUM (SETTINGS_NETWORK_IPADDRESS_NUM+1)

#define SETTINGS_NETWORK_TXPOWER "/Network/Transmit Power"
#define SETTINGS_NETWORK_TXPOWER_DEFAULT 20
#define SETTINGS_NETWORK_TXPOWER_NUM (SETTINGS_NETWORK_IPSUBNET_NUM+1)

#define SETTINGS_NETWORK_TXRATE "/Network/Transmit Bandwidth"
#define SETTINGS_NETWORK_TXRATE_DEFAULT 0
#define SETTINGS_NETWORK_TXRATE_NUM (SETTINGS_NETWORK_TXPOWER_NUM+1)

// New settings for hybrid -MH
#define SETTINGS_HYBRIDNETWORK_MODE "/Hybrid Network/Mode"
#define SETTINGS_HYBRIDNETWORK_MODE_LIST_SIZE 3
const int SETTINGS_HYBRIDNETWORK_MODES[SETTINGS_HYBRIDNETWORK_MODE_LIST_SIZE] = {0, 1, 2};
const QString SETTINGS_HYBRIDNETWORK_MODE_LIST[SETTINGS_HYBRIDNETWORK_MODE_LIST_SIZE] = {"Off", "Gateway", "VOD"};
#define SETTINGS_HYBRIDNETWORK_MODE_DEFAULT (SETTINGS_HYBRIDNETWORK_MODES[0])
#define SETTINGS_HYBRIDNETWORK_MODE_LABEL "Hybrid Network Mode"
#define SETTINGS_HYBRIDNETWORK_MODE_NUM (SETTINGS_NETWORK_TXRATE_NUM+1)

#define SETTINGS_HYBRIDNETWORK_IP "/Hybrid Network/IP"
#define SETTINGS_HYBRIDNETWORK_IP_DEFAULT "128.2.134.51"
#define SETTINGS_HYBRIDNETWORK_IP_NUM (SETTINGS_HYBRIDNETWORK_MODE_NUM+1)
// end new hybrid settings

#define SETTINGS_NUM (SETTINGS_HYBRIDNETWORK_MODE_NUM+2)

#define SETTINGS_ADDRESSES_ADDRESSBASE "/AddressHistory/Address"

#define PARAMKEY_NETWORK_DEVNAME "--dev"
#define PARAMKEY_NETWORK_IP "--ip"
#define PARAMKEY_NETWORK_SUBNET "--subnet"

class Setting
{
public:
	typedef enum SettingTypeEnum
	{
		SettingTypeText = 0,
		SettingTypeBool,
		SettingTypeInt,
		SettingTypeFloat,
		SettingTypeIP,
		SettingTypeColor,
		SettingTypeFile,
	
		SettingTypeList = 16,
		SettingTypeRange = 32
	} SettingType;

	typedef struct SettingDataStruct
	{
		QString strValue;
		bool bValue;
		long iValue;
		double fValue;
	} SettingData;

	Setting();
	Setting(const QString & strKey, SettingType eType, const SettingData & sDefault, const SettingData * pMin = NULL, const SettingData * pMax = NULL, const SettingData * vecEnum = NULL, unsigned int iEnum = 0);
	Setting(const Setting & copy);
	~Setting();
	
	Setting & operator = (const Setting & copy);

	inline const QString & GetKey() const { return m_strKey; }
	inline SettingType GetType() const { return m_eType; }
	inline const SettingData & GetDefault() const { return m_sDefault; }
	bool IsValid(const SettingData & sValue) const;
	inline const SettingData & GetValue() const { return m_sValue; }
	bool SetValue(const SettingData & sValue);
	inline void SetDefaultValue() { m_sValue = m_sDefault; }
	int Compare(const SettingData & x, const SettingData & y) const;
	inline QString GetLabel() const { return m_strKey.mid(m_strKey.findRev('/')+1); }
	inline const SettingData & GetEnumValue(unsigned int i) { return m_vecEnum[i]; }
	
	QTableItem * CreateItem(QTable * pTable);
	void UpdateItem(QTableItem * pItem);

	QStringList m_listEnumStrings;

protected:
	QString m_strKey;
	SettingType m_eType;
	SettingData m_sDefault, m_sValue;
	SettingData * m_pMin, * m_pMax, * m_vecEnum;
	unsigned int m_iEnum;
};


class Settings
{
public:
	Settings(int argc, char ** argv, QSettings * pAppSettings = NULL);
	Settings(const Settings & copy);
	~Settings();

	Settings & operator = (const Settings & copy);

	// the settings
	Setting m_sSettings[SETTINGS_NUM];

	void RestoreDefaults();
	void ReadSettings();
	void WriteSettings();

	void AddAddress(const QString & strAddress);
	void ExtractParamsFromCmdLine(int argc, char ** argv);
	const QString & GetParam(const QString & strKey, const QString & strDefault, bool bAllowEmpty = false) const;

	std::list<QString> m_listAddressHistory;

protected:
	QSettings * m_pAppSettings;
	std::map<QString, QString> m_mapCmdLineParams;
};

extern Settings * g_pSettings;

#endif

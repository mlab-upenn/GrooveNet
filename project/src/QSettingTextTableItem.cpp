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

#include "QSettingTextTableItem.h"

QSettingTextTableItem::QSettingTextTableItem(QTable * parent, Setting * pSetting, EditType ed)
: QTableItem(parent, ed), m_pSetting(pSetting)
{
	setReplaceable(false);
}

QSettingTextTableItem::~QSettingTextTableItem()
{
}

QString QSettingTextTableItem::text() const
{
	if (m_pSetting == NULL)
		return QString::null;
	else
	{
		switch (m_pSetting->GetType() & 0xF)
		{
		case Setting::SettingTypeBool:
			return m_pSetting->GetValue().bValue ? "True" : "False";
		case Setting::SettingTypeInt:
			return QString("%1").arg(m_pSetting->GetValue().iValue);
		case Setting::SettingTypeFloat:
			return QString("%1").arg(m_pSetting->GetValue().fValue);
		default:
			return m_pSetting->GetValue().strValue;
		}
	}
}

void QSettingTextTableItem::setText(const QString & str)
{
	Setting::SettingData sValue;
	switch (m_pSetting->GetType() & 0xF)
	{
	case Setting::SettingTypeBool:
		sValue.bValue = (str.compare("True") == 0);
		break;
	case Setting::SettingTypeInt:
		sValue.iValue = str.toLong();
		break;
	case Setting::SettingTypeFloat:
		sValue.fValue = str.toDouble();
		break;
	default:
		sValue.strValue = str;
		break;
	}
	if (m_pSetting == NULL || m_pSetting->SetValue(sValue))
		QTableItem::setText(str);
	else
		table()->updateCell(row(), col());
}

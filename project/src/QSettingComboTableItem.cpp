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

#include "QSettingComboTableItem.h"

#include <qcombobox.h>

QSettingComboTableItem::QSettingComboTableItem(QTable * parent, const QStringList & listStrings, bool bEditable, Setting * pSetting)
: QComboTableItem(parent, listStrings, bEditable), m_pSetting(pSetting)
{
	setReplaceable(false);
}

QSettingComboTableItem::~QSettingComboTableItem()
{
}

void QSettingComboTableItem::setContentFromEditor(QWidget * w)
{
	if (w->inherits("QComboBox") && m_pSetting != NULL)
	{
		int iCurrent = ((QComboBox *)w)->currentItem();
		Setting::SettingData sValue;
		if (m_pSetting->m_listEnumStrings.empty())
		{
			switch (m_pSetting->GetType() & 0xF)
			{
			case Setting::SettingTypeBool:
				sValue.bValue = (iCurrent == 0);
				break;
			case Setting::SettingTypeInt:
				sValue.iValue = ((QComboBox *)w)->currentText().toLong();
				break;
			case Setting::SettingTypeFloat:
				sValue.fValue = ((QComboBox *)w)->currentText().toDouble();
				break;
			default:
				sValue.strValue = ((QComboBox *)w)->currentText();
				break;
			}
		} else
			sValue = m_pSetting->GetEnumValue(iCurrent);
		if (m_pSetting->SetValue(sValue))
			QComboTableItem::setContentFromEditor(w);
		else
			table()->updateCell(row(), col());
	} else
		QComboTableItem::setContentFromEditor(w);
}

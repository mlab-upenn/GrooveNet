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

#ifndef _QSETTINGCOLORTABLEITEM_H
#define _QSETTINGCOLORTABLEITEM_H

#include "QSettingTextTableItem.h"
#include "Settings.h"

#define SETTINGCOLORTABLEITEM_RTTI_VALUE 4211112

class QSettingColorTableItem : public QSettingTextTableItem
{
public:
	QSettingColorTableItem(QTable * parent, Setting * pSetting = NULL);
	virtual ~QSettingColorTableItem();

	inline virtual int rtti() const { return SETTINGCOLORTABLEITEM_RTTI_VALUE; }
	QPixmap CreatePixmap() const;
	virtual void setText(const QString & str);
	virtual QWidget * createEditor() const;
	virtual void setContentFromEditor(QWidget * w);

protected:
};

#endif

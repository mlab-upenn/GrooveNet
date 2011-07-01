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

#include <qlineedit.h>

#include "QSettingFileTableItem.h"
#include "QFilePushButton.h"

#include <qpainter.h>
#include <qlayout.h>

QSettingFileTableItem::QSettingFileTableItem(QTable * parent, Setting * pSetting)
: QSettingTextTableItem(parent, pSetting, OnTyping)
{
}

QSettingFileTableItem::~QSettingFileTableItem()
{
}

QWidget * QSettingFileTableItem::createEditor() const
{
	QWidget * pBox = new QWidget(table()->viewport());
	QHBoxLayout * pBoxLayout = new QHBoxLayout(pBox);
	QLineEdit * pEdit = new QLineEdit(pBox, "editor");
	QFilePushButton * pButton = new QFilePushButton(pEdit, "...", pBox, "filebutton");
	pBoxLayout->addWidget(pEdit, 1);
	pBoxLayout->addWidget(pButton);
	pBox->setFocusProxy(pEdit);
	pEdit->setText(text());
	return pBox;
}

void QSettingFileTableItem::setContentFromEditor(QWidget * w)
{
	QLineEdit * pEdit = (QLineEdit *)w->child("editor", "QLineEdit");
	if (pEdit != NULL)
		setText(pEdit->text());
	else
		QSettingTextTableItem::setContentFromEditor(w);
}

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

#include "QSettingColorTableItem.h"
#include "QColorPushButton.h"

#include <qpainter.h>
#include <qlayout.h>
#include <qpushbutton.h>

QSettingColorTableItem::QSettingColorTableItem(QTable * parent, Setting * pSetting)
: QSettingTextTableItem(parent, pSetting)
{
	setPixmap(CreatePixmap());
}

QSettingColorTableItem::~QSettingColorTableItem()
{
}

QPixmap QSettingColorTableItem::CreatePixmap() const
{
	if (m_pSetting == NULL || (m_pSetting->GetType() & 0xF) != Setting::SettingTypeColor)
		return QPixmap();
	else
	{
		QPixmap bmpImage(sizeHint().height() - 4, sizeHint().height() - 4);
		QPainter dcImage;
		QColor clrBackground(m_pSetting->GetValue().strValue);
		dcImage.begin(&bmpImage, table()->viewport());
		dcImage.setBackgroundColor(clrBackground.isValid() ? clrBackground : QColor(0, 0, 0));
		dcImage.eraseRect(bmpImage.rect());
		dcImage.drawRect(bmpImage.rect());
		dcImage.end();
		return bmpImage;
	}
}

void QSettingColorTableItem::setText(const QString & str)
{
	QSettingTextTableItem::setText(str);
	setPixmap(CreatePixmap());
}

QWidget * QSettingColorTableItem::createEditor() const
{
	QWidget * pBox = new QWidget(table()->viewport());
	QHBoxLayout * pBoxLayout = new QHBoxLayout(pBox);
	QLineEdit * pEdit = new QLineEdit(pBox, "editor");
	QColorPushButton * pButton = new QColorPushButton(pEdit, "...", pBox, "colorbutton");
	pBoxLayout->addWidget(pEdit, 1);
	pBoxLayout->addWidget(pButton);
	pBox->setFocusProxy(pEdit);
	pEdit->setText(pButton->m_strColor = text());
	return pBox;
}

void QSettingColorTableItem::setContentFromEditor(QWidget * w)
{
	QLineEdit * pEdit = (QLineEdit *)w->child("editor", "QLineEdit");
	if (pEdit != NULL)
		setText(pEdit->text());
	else
		QSettingTextTableItem::setContentFromEditor(w);
}

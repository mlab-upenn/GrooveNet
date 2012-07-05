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

#include "QMapWidget.h"

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <qlayout.h>

#include "app16x16.xpm"

#include "QBoundingRegionConfDialog.h"
#include "StringHelp.h"

QBoundingRegionConfDialog::QBoundingRegionConfDialog(const SafetyPacket::BoundingRegion & sBoundingRegion, QWidget * parent, const char * name, WFlags f)
: QDialog(parent, name, f), m_sBoundingRegion(sBoundingRegion)
{
	QWidget * pParamBox = new QWidget(this);
	QWidget * pButtonBox = new QWidget(this);

	QVBoxLayout * pLayout = new QVBoxLayout(this, 8, 8);
	QHBoxLayout * pParamBoxLayout = new QHBoxLayout(pParamBox, 0, 8);
	QHBoxLayout * pButtonBoxLayout = new QHBoxLayout(pButtonBox, 0, 8);

	setCaption("GrooveNet - Configure Message Bounding Region...");
	setIcon(app16x16_xpm);

	m_pMap = new QMapWidget(this);
	m_labelRegionInfo = new QLabel("", this);
	m_labelParameter = new QLabel("", pParamBox);
	m_txtParameter = new QLineEdit("", pParamBox);
	m_buttonOK = new QPushButton("&OK", pButtonBox);
	m_buttonCancel = new QPushButton("&Cancel", pButtonBox);

	connect(m_pMap, SIGNAL(selectionChanged()), this, SLOT(slotMapSelectionChanged()));
	connect(m_txtParameter, SIGNAL(textChanged(const QString& )), this, SLOT(slotParamChanged(const QString& )));
	connect(m_buttonOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

	InitializeBoundingRegionInfo();
	m_buttonOK->setAutoDefault(false);
	m_buttonCancel->setAutoDefault(false);

	slotMapSelectionChanged();
	slotParamChanged(m_txtParameter->text());

	pLayout->addWidget(m_pMap, 1);
	pLayout->addWidget(m_labelRegionInfo, 0, Qt::AlignLeft);
	pLayout->addWidget(pParamBox);
	pLayout->addWidget(pButtonBox);
	pParamBoxLayout->addWidget(m_labelParameter, 0, Qt::AlignLeft);
	pParamBoxLayout->addWidget(m_txtParameter, 1);
	pButtonBoxLayout->addStretch(1);
	pButtonBoxLayout->addWidget(m_buttonOK, 0, Qt::AlignCenter);
	pButtonBoxLayout->addWidget(m_buttonCancel, 0, Qt::AlignCenter);
}

QBoundingRegionConfDialog::~QBoundingRegionConfDialog()
{
}

void QBoundingRegionConfDialog::InitializeBoundingRegionInfo()
{
	unsigned int i;

	switch (m_sBoundingRegion.eRegionType)
	{
	case SafetyPacket::BoundingRegionTypeBBox:
	{
		Rect rBox(m_sBoundingRegion.vecCoords[0].m_iLong, m_sBoundingRegion.vecCoords[0].m_iLat, m_sBoundingRegion.vecCoords[1].m_iLong, m_sBoundingRegion.vecCoords[1].m_iLat);
		m_pMap->SetSelectionMode(QMapWidget::SelectionModeRect);
		m_pMap->SetSelectionRect(rBox.normalize());
		m_pMap->recenter(rBox.GetCenter());
		m_labelParameter->hide();
		m_txtParameter->hide();
		break;
	}
	case SafetyPacket::BoundingRegionTypeWaypoint:
	{
		std::list<Coords> listPoints;
		for (i = 0; i < BOUNDINGREGIONCOORDSMAX; i++)
		{
			if (m_sBoundingRegion.vecCoords[i].m_iLong != 0 || m_sBoundingRegion.vecCoords[i].m_iLat != 0) {
				listPoints.push_back(m_sBoundingRegion.vecCoords[i]);
			}
			else
				break;
		}
		m_pMap->SetSelectionMode(QMapWidget::SelectionModePoints);
		m_pMap->SetSelectionPoints(listPoints);
		m_pMap->recenter(listPoints.empty() ? Coords() : listPoints.front());
		m_labelRegionInfo->hide();
		m_labelParameter->setText("Radius (meters):");
		m_txtParameter->setText(QString("%1").arg(m_sBoundingRegion.fParam));
		slotParamChanged(m_txtParameter->text());
		break;
	}
	case SafetyPacket::BoundingRegionTypeCircle:
	{
		m_pMap->SetSelectionMode(QMapWidget::SelectionModePoint);
		m_pMap->SetSelectionPoint(m_sBoundingRegion.vecCoords[0]);
		m_pMap->recenter(m_sBoundingRegion.vecCoords[0]);
		m_labelParameter->setText("Radius (meters):");
		m_txtParameter->setText(QString("%1").arg(m_sBoundingRegion.fParam));
		slotParamChanged(m_txtParameter->text());
		break;
	}
	case SafetyPacket::BoundingRegionTypeDirection:
	{
		m_pMap->SetSelectionMode(QMapWidget::SelectionModePoint);
		m_pMap->SetSelectionPoint(m_sBoundingRegion.vecCoords[0]);
		m_pMap->recenter(m_sBoundingRegion.vecCoords[0]);
		m_labelRegionInfo->hide();
		m_labelParameter->setText("Direction (degrees relative to true North):");
		m_txtParameter->setText(QString("%1").arg(m_sBoundingRegion.fParam));
		slotParamChanged(m_txtParameter->text());
		break;
	}
	default:
		m_pMap->SetSelectionMode(QMapWidget::SelectionModeNone);
		m_labelParameter->hide();
		m_txtParameter->hide();
		break;
	}
}

void QBoundingRegionConfDialog::slotMapSelectionChanged()
{
	unsigned int i;

	switch (m_sBoundingRegion.eRegionType)
	{
	case SafetyPacket::BoundingRegionTypeBBox:
	{
		Rect rBox = m_pMap->GetSelectionRect();
		m_labelRegionInfo->setText(QString("Bounding box area: %1 sq. km.").arg(rBox.GetArea() * KILOMETERSPERMILE * KILOMETERSPERMILE));
		m_sBoundingRegion.vecCoords[0].m_iLong = rBox.m_iLeft;
		m_sBoundingRegion.vecCoords[0].m_iLat = rBox.m_iTop;
		m_sBoundingRegion.vecCoords[1].m_iLong = rBox.m_iRight;
		m_sBoundingRegion.vecCoords[1].m_iLat = rBox.m_iBottom;
		m_buttonOK->setEnabled(rBox.m_iRight > rBox.m_iLeft && rBox.m_iTop > rBox.m_iBottom);
		break;
	}
	case SafetyPacket::BoundingRegionTypeWaypoint:
	{
		std::list<Coords> listPoints = m_pMap->GetSelectionPoints();
		std::list<Coords>::iterator iterPoint;
		for (i = 0, iterPoint = listPoints.begin(); i < BOUNDINGREGIONCOORDSMAX && iterPoint != listPoints.end(); i++, ++iterPoint)
			m_sBoundingRegion.vecCoords[i] = *iterPoint;
		m_sBoundingRegion.vecCoords[i].Set(0, 0);
		m_buttonOK->setEnabled((m_sBoundingRegion.vecCoords[0].m_iLong != 0 || m_sBoundingRegion.vecCoords[0].m_iLat != 0) && m_sBoundingRegion.fParam > 0.);
		break;
	}
	case SafetyPacket::BoundingRegionTypeCircle:
	{
		m_sBoundingRegion.vecCoords[0] = m_pMap->GetSelectionPoint();
		m_buttonOK->setEnabled((m_sBoundingRegion.vecCoords[0].m_iLong != 0 || m_sBoundingRegion.vecCoords[0].m_iLat != 0) && m_sBoundingRegion.fParam > 0.);
		break;
	}
	case SafetyPacket::BoundingRegionTypeDirection:
	{
		m_sBoundingRegion.vecCoords[0] = m_pMap->GetSelectionPoint();
		m_buttonOK->setEnabled((m_sBoundingRegion.vecCoords[0].m_iLong != 0 || m_sBoundingRegion.vecCoords[0].m_iLat != 0) && m_sBoundingRegion.fParam > 0.);
		break;
	}
	default:
		m_buttonOK->setEnabled(false);
		break;
	}
}

void QBoundingRegionConfDialog::slotParamChanged(const QString & strText)
{
	switch (m_sBoundingRegion.eRegionType)
	{
	case SafetyPacket::BoundingRegionTypeWaypoint:
		m_sBoundingRegion.fParam = StringToNumber(m_txtParameter->text());
		m_buttonOK->setEnabled((m_sBoundingRegion.vecCoords[0].m_iLong != 0 || m_sBoundingRegion.vecCoords[0].m_iLat != 0) && m_sBoundingRegion.fParam > 0.);
		break;
	case SafetyPacket::BoundingRegionTypeCircle:
		m_sBoundingRegion.fParam = StringToNumber(m_txtParameter->text());
		m_labelRegionInfo->setText(QString("Bounding region area: %1 sq. km.").arg(m_sBoundingRegion.fParam * m_sBoundingRegion.fParam * M_PI * 1e-6));
		m_buttonOK->setEnabled((m_sBoundingRegion.vecCoords[0].m_iLong != 0 || m_sBoundingRegion.vecCoords[0].m_iLat != 0) && m_sBoundingRegion.fParam > 0.);
		break;
	case SafetyPacket::BoundingRegionTypeDirection:
		m_sBoundingRegion.fParam = StringToNumber(m_txtParameter->text());
		m_buttonOK->setEnabled((m_sBoundingRegion.vecCoords[0].m_iLong != 0 || m_sBoundingRegion.vecCoords[0].m_iLat != 0) && m_sBoundingRegion.fParam > 0.);
		break;
	default:
		break;
	}
}

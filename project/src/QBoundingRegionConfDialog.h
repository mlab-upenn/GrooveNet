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

#ifndef _QBOUNDINGREGIONCONFDIALOG_H
#define _QBOUNDINGREGIONCONFDIALOG_H

#include <qdialog.h>

class QMapWidget;
class QPushButton;
class QLabel;
class QLineEdit;

#include "Model.h"
#include "Coords.h"
#include "Message.h"

class QBoundingRegionConfDialog : public QDialog
{
Q_OBJECT
public:
	QBoundingRegionConfDialog(const SafetyPacket::BoundingRegion & sBoundingRegion, QWidget * parent = NULL, const char * name = 0, WFlags f = 0);
	virtual ~QBoundingRegionConfDialog();

	void InitializeBoundingRegionInfo();

	SafetyPacket::BoundingRegion m_sBoundingRegion;

protected slots:
	virtual void slotMapSelectionChanged();
	virtual void slotParamChanged(const QString & strText);

protected:

	QMapWidget * m_pMap;
	QLabel * m_labelRegionInfo, * m_labelParameter;
	QPushButton * m_buttonOK, * m_buttonCancel;
	QLineEdit * m_txtParameter;
};

#endif

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
#ifndef _CARLISTVISUAL_H
#define _CARLISTVISUAL_H

#include "TableVisualizer.h"

#include <qdragobject.h>
#include <arpa/inet.h>

#define CARLISTVISUAL_NAME "CarListVisual"

class QPopupMenu;

class CarListVisual : public TableVisualizer
{
public:
	inline virtual QString GetModelType() const
	{
		return CARLISTVISUAL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(CARLISTVISUAL_NAME) == 0 || (bDescendSufficient && TableVisualizer::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	CarListVisual(const QString & strModelName = QString::null);
	CarListVisual(const CarListVisual & copy);
	virtual ~CarListVisual();

	virtual CarListVisual & operator = (const CarListVisual & copy);

	virtual int Init(const std::map<QString, QString> & mapParams);
	virtual int ProcessEvent(SimEvent & event);
	virtual int PostRun();
	virtual int Save(std::map<QString, QString> & mapParams);
	virtual int Cleanup();

	virtual void tableContextMenuRequested(int row, int col, const QPoint & pos);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	virtual void UpdateTable();

	std::map<in_addr_t, int> m_mapObjectsToRows;

protected:
	QPopupMenu * m_pRightClickMenu;
};

QDragObject * CarListVisualDragObjectCreator(TableVisualizer * pTableVisualizer);

inline Model * CarListVisualCreator(const QString & strModelName)
{
	return new CarListVisual(strModelName);
}

#endif

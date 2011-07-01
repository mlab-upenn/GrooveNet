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

#ifndef _TABLEVISUALIZER_H
#define _TABLEVISUALIZER_H

#include "Visualizer.h"

#define TABLEVISUALIZER_NAME "TableVisualizer"

class TableVisualizer : public Visualizer
{
public:
	inline virtual QString GetModelType() const
	{
		return TABLEVISUALIZER_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(TABLEVISUALIZER_NAME) == 0 || (bDescendSufficient && Visualizer::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	TableVisualizer(const QString & strModelName = QString::null);
	TableVisualizer(const TableVisualizer & copy);
	virtual ~TableVisualizer();

	virtual TableVisualizer & operator = (const TableVisualizer & copy);

	virtual int Cleanup();

	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	inline virtual void tableCurrentChanged(int row __attribute__ ((unused)) , int col __attribute__ ((unused)) ) {}
	inline virtual void tableClicked(int row __attribute__ ((unused)) , int col __attribute__ ((unused)) , int button __attribute__ ((unused)) , const QPoint & mousePos __attribute__ ((unused)) ) {}
	inline virtual void tableDoubleClicked(int row __attribute__ ((unused)) , int col __attribute__ ((unused)) , int button __attribute__ ((unused)) , const QPoint & mousePos __attribute__ ((unused)) ) {}
	inline virtual void tablePressed(int row __attribute__ ((unused)) , int col __attribute__ ((unused)) , int button __attribute__ ((unused)) , const QPoint & mousePos __attribute__ ((unused)) ) {}
	inline virtual void tableSelectionChanged() {}
	inline virtual void tableValueChanged(int row __attribute__ ((unused)) , int col __attribute__ ((unused)) ) {}
	inline virtual void tableContextMenuRequested(int row __attribute__ ((unused)) , int col __attribute__ ((unused)) , const QPoint & pos __attribute__ ((unused)) ) {}
	inline virtual void tableDropped(QDropEvent * e __attribute__ ((unused)) ) {}

protected:
	virtual QWidget * CreateWidget();
};

#endif


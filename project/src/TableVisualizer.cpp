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

#include "TableVisualizer.h"
#include "QTableVisualizer.h"
#include "MainWindow.h"
#include "StringHelp.h"

#include <qapplication.h>

TableVisualizer::TableVisualizer(const QString & strModelName)
: Visualizer(strModelName)
{
}

TableVisualizer::TableVisualizer(const TableVisualizer & copy)
: Visualizer(copy)
{
}

TableVisualizer::~TableVisualizer()
{
}

TableVisualizer & TableVisualizer::operator = (const TableVisualizer & copy)
{
	Visualizer::operator =(copy);

	return *this;
}

void TableVisualizer::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	Visualizer::GetParams(mapParams);
}

int TableVisualizer::Cleanup()
{
	if (m_pWidget != NULL && ((QTableVisualizer*)m_pWidget)->m_pTable != NULL)
	{
		((QTableVisualizer*)m_pWidget)->m_pTable->setNumRows(0);
		((QTableVisualizer*)m_pWidget)->m_pTable->close(true);
		((QTableVisualizer*)m_pWidget)->m_pTable = NULL;
	}

	if (Visualizer::Cleanup())
		return 1;

	return 0;
}

QWidget * TableVisualizer::CreateWidget()
{
	return new QTableVisualizer(this, g_pMainWindow->centralWidget(), m_strModelName, Qt::WNoAutoErase | Qt::WDestructiveClose);
}

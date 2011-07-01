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
#include "QVisualizer.h"

#include <qpixmap.h>

#include "app16x16.xpm"

QVisualizer::QVisualizer(Visualizer * pVisualizer, QWidget *parent, const char *name, Qt::WFlags f)
: QWidget(parent, name, f), m_pVisualizer(pVisualizer)
{
	setIcon(app16x16_xpm);
}

QVisualizer::~QVisualizer()
{
	if (m_pVisualizer) /* unbind widget */
		m_pVisualizer->SetWidget(NULL);
}

void QVisualizer::closeEvent(QCloseEvent * e)
{
	if (m_pVisualizer) {
		m_pVisualizer->m_mutexUpdate.lock();
		m_pVisualizer->closeEvent(e);
		m_pVisualizer->m_mutexUpdate.unlock();
	}
	e->accept();
}

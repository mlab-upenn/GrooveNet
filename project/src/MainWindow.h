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

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include <qmainwindow.h>
#include <qworkspace.h>
#include <qaction.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include "QNetworkManager.h"
#include "QMessageList.h"

class MainWindow: public QMainWindow
{
Q_OBJECT
public:
	MainWindow();
	~MainWindow();

	QPopupMenu * m_pFileMenu, * m_pSimMenu, * m_pNetMenu, * m_pWindowMenu, * m_pNetServerMenu;
	QAction * m_pFileNew, * m_pFileEdit, * m_pFileOpen, * m_pFileSave, * m_pFileConfig, * m_pFileExit, * m_pSimRun, * m_pSimPause, * m_pSimSkip, * m_pSimStop, * m_pNetInit, * m_pNetClose, * m_pNetServer,  * m_pWindowCascade, * m_pWindowTile, * m_pWindowCloseAll;
	QLabel * m_pLblStatus;
	QNetworkManager * m_pNetworkManager;
	int m_iNetServerMenuID;



	QToolBar *tabsContainer;
	QTabWidget *tabsWidget;

	QWorkspace * m_pCentralWidget;

protected slots:
	void OnFileNew();
	void OnFileEdit();
	void OnFileOpen();
	void OnFileSave();
	void OnFileConfig();
	void OnFileExit();
	void OnSimRun();
	void OnSimPause();
	void OnSimSkip();
	void OnSimStop();
	void OnNetInit();
	void OnNetClose();
	void OnNetServer();
	void OnNetServer(int id);

protected:
	void closeEvent(QCloseEvent * e);
};

extern MainWindow * g_pMainWindow;

#endif

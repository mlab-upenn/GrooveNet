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

#include "MainWindow.h"

#include "Global.h"
#include "Simulator.h"
#include "Logger.h"
#include "QSimCreateDialog.h"
#include "QConfigureDialog.h"
#include "QSimRunDialog.h"

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qaccel.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qcursor.h>
#include <qstatusbar.h>
#include <qlayout.h>

#include "app32x32.xpm"
#include "fileopen.xpm"
#include "filesave.xpm"

MainWindow::MainWindow()
    : QMainWindow( 0, "MainWindow", WDestructiveClose )
{
	QDockWindow * pNMWindow;
	std::map<QString, ServerCreator>::iterator iterCreator;

	m_pCentralWidget = new QWorkspace(this, "CENTRALWIDGET");
	m_pCentralWidget->setScrollBarsEnabled(true);

	setIcon(app32x32_xpm);

	setCentralWidget(m_pCentralWidget);

	// create menus
	m_pFileMenu = new QPopupMenu(this, "File Menu");
	m_pFileNew = new QAction("&New...", QAccel::stringToKey("Ctrl+N"), this, "file.new");
	m_pFileNew->setToolTip("Create a new simulation file, ending the current simulation");
	connect(m_pFileNew, SIGNAL(activated()), this, SLOT(OnFileNew()));
	m_pFileNew->addTo(m_pFileMenu);
	m_pFileEdit = new QAction("&Edit...", QAccel::stringToKey("Ctrl+E"), this, "file.edit");
	m_pFileEdit->setToolTip("Edit the current simulation file, ending the current simulation");
	connect(m_pFileEdit, SIGNAL(activated()), this, SLOT(OnFileEdit()));
	m_pFileEdit->setEnabled(false);
	m_pFileEdit->addTo(m_pFileMenu);
	m_pFileOpen = new QAction(QPixmap(fileopen), "&Open...", QAccel::stringToKey("Ctrl+O"), this, "file.open");
	m_pFileOpen->setToolTip("Open a simulation file, ending the current simulation");
	connect(m_pFileOpen, SIGNAL(activated()), this, SLOT(OnFileOpen()));
	m_pFileOpen->addTo(m_pFileMenu);
	m_pFileSave = new QAction(QPixmap(filesave), "&Save As...", QAccel::stringToKey("Ctrl+S"), this, "file.save");
	m_pFileSave->setToolTip("Save the current setup to a simulation file (this ends any running simulation)");
	connect(m_pFileSave, SIGNAL(activated()), this, SLOT(OnFileSave()));
	m_pFileSave->setEnabled(false);
	m_pFileSave->addTo(m_pFileMenu);
	m_pFileMenu->insertSeparator();
	m_pFileConfig = new QAction("&Configure...", QKeySequence(), this, "file.config");
	m_pFileConfig->setToolTip("Edit the current configuration");
	connect(m_pFileConfig, SIGNAL(activated()), this, SLOT(OnFileConfig()));
	m_pFileConfig->addTo(m_pFileMenu);
	m_pFileExit = new QAction("E&xit...", QAccel::stringToKey("Ctrl+Q"), this, "file.exit");
	m_pFileExit->setToolTip("Exit the program");
	connect(m_pFileExit, SIGNAL(activated()), this, SLOT(OnFileExit()));
	m_pFileExit->addTo(m_pFileMenu);

	m_pSimMenu = new QPopupMenu(this, "Sim Menu");
	m_pSimRun = new QAction("&Run...", QKeySequence(Qt::Key_F2), this, "sim.run");
	m_pSimRun->setToolTip("Start a new simulation");
	connect(m_pSimRun, SIGNAL(activated()), this, SLOT(OnSimRun()));
	m_pSimRun->setEnabled(false);
	m_pSimRun->addTo(m_pSimMenu);
	m_pSimPause = new QAction("Pause", QKeySequence(Qt::Key_F3), this, "sim.pause");
	m_pSimPause->setToolTip("Pause the currently running simulation");
	m_pSimPause->setEnabled(false);
	connect(m_pSimPause, SIGNAL(activated()), this, SLOT(OnSimPause()));
	m_pSimPause->addTo(m_pSimMenu);
	m_pSimSkip = new QAction("Skip to next", QKeySequence(), this, "sim.skip");
	m_pSimSkip->setToolTip("Skip to the next Monte Carlo iteration of the simulation");
	m_pSimSkip->setEnabled(false);
	connect(m_pSimSkip, SIGNAL(activated()), this, SLOT(OnSimSkip()));
	m_pSimSkip->addTo(m_pSimMenu);
	m_pSimStop = new QAction("&Stop", QKeySequence(Qt::SHIFT | Qt::Key_F2), this, "sim.stop");
	m_pSimStop->setToolTip("Stop the currently running simulation");
	m_pSimStop->setEnabled(false);
	connect(m_pSimStop, SIGNAL(activated()), this, SLOT(OnSimStop()));
	m_pSimStop->addTo(m_pSimMenu);

	m_pNetMenu = new QPopupMenu(this, "Net Menu");
	m_pNetInit = new QAction("&Initialize", QKeySequence(), this, "net.init");
	m_pNetInit->setToolTip("Initialize network devices and connections");
	m_pNetInit->setEnabled(true);
	connect(m_pNetInit, SIGNAL(activated()), this, SLOT(OnNetInit()));
	m_pNetInit->addTo(m_pNetMenu);
	m_pNetClose = new QAction("&Close", QKeySequence(), this, "net.close");
	m_pNetClose->setToolTip("Close network devices and connections");
	m_pNetClose->setEnabled(false);
	connect(m_pNetClose, SIGNAL(activated()), this, SLOT(OnNetClose()));
	m_pNetClose->addTo(m_pNetMenu);
	m_pNetServer = new QAction("&Stop Server", QKeySequence(), this, "net.server");
	m_pNetServer->setToolTip("Stop GrooveNet server");
	connect(m_pNetServer, SIGNAL(activated()), this, SLOT(OnNetServer()));
	m_pNetServerMenu = new QPopupMenu(this, "startserver");
	connect(m_pNetServerMenu, SIGNAL(activated(int)), this, SLOT(OnNetServer(int)));
	for (iterCreator = g_mapServerCreators.begin(); iterCreator != g_mapServerCreators.end(); ++iterCreator)
		m_pNetServerMenu->insertItem(iterCreator->first);
	m_iNetServerMenuID = m_pNetMenu->insertItem("&Start Server", m_pNetServerMenu);

	m_pWindowMenu = new QPopupMenu(this, "Window Menu");
	m_pWindowCascade = new QAction("&Cascade", QKeySequence(), this, "window.cascade");
	m_pWindowCascade->setToolTip("Cascade all open windows");
	connect(m_pWindowCascade, SIGNAL(activated()), m_pCentralWidget, SLOT(cascade()));
	m_pWindowCascade->addTo(m_pWindowMenu);
	m_pWindowTile = new QAction("&Tile", QKeySequence(), this, "window.tile");
	m_pWindowTile->setToolTip("Tile all open windows");
	connect(m_pWindowTile, SIGNAL(activated()), m_pCentralWidget, SLOT(tile()));
	m_pWindowTile->addTo(m_pWindowMenu);
	m_pWindowCloseAll = new QAction("Close &All Windows", QKeySequence(), this, "window.closeall");
	m_pWindowCloseAll->setToolTip("Close all open windows");
	connect(m_pWindowCloseAll, SIGNAL(activated()), m_pCentralWidget, SLOT(closeAllWindows()));
	m_pWindowCloseAll->addTo(m_pWindowMenu);

	menuBar()->insertItem("&File", m_pFileMenu);
	menuBar()->insertItem("&Simulator", m_pSimMenu);
	menuBar()->insertItem("&Network", m_pNetMenu);
	menuBar()->insertItem("&Window", m_pWindowMenu);

	statusBar()->addWidget(m_pLblStatus = new QLabel("Ready", this, "statusbar.label"), 1);
	statusBar()->setSizeGripEnabled(true);

/*	pNMWindow = new QDockWindow(this);
	pNMWindow->setCaption("Network Manager");
	pNMWindow->setHorizontallyStretchable(true);
	pNMWindow->setVerticallyStretchable(false);
	pNMWindow->setResizeEnabled(true);
	pNMWindow->setMovingEnabled(true);
	pNMWindow->setCloseMode(QDockWindow::Always);
	m_pNetworkManager = new QNetworkManager(pNMWindow, "network manager");
	pNMWindow->setWidget(m_pNetworkManager);
	addDockWindow(pNMWindow, Qt::DockBottom, true);
*/
        tabsContainer = new QToolBar(this);
        tabsWidget = new QTabWidget(tabsContainer, "tabs");
        tabsWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        tabsWidget->setMargin(8);

        tabsContainer->setStretchableWidget(tabsWidget);
        tabsContainer->setLabel(tr("GrooveNet - Tooltabs"));
        addDockWindow(tabsContainer, Qt::DockBottom, TRUE);
        m_pNetworkManager = new QNetworkManager(tabsContainer, "network manager");
        tabsWidget->addTab(m_pNetworkManager, QString("Network Manager"));
	m_pMessageList = new QMessageList(tabsContainer, "message list");
        tabsWidget->addTab(m_pMessageList, QString("Messages"));
        setAppropriate(tabsContainer, false);
        tabsWidget->addTab(NULL, QString("Congestion Info"));
        tabsWidget->addTab(NULL, QString("Playback"));
}

MainWindow::~MainWindow()
{
	// perform cleanup
	g_pSimulator->Unload();
}

void MainWindow::OnFileNew()
{
	QSimCreateDialog * pDialog = new QSimCreateDialog(this);

	if (pDialog->exec() == QDialog::Accepted)
		m_pLblStatus->setText("Ready");
	delete pDialog;
}

void MainWindow::OnFileEdit()
{
	QSimCreateDialog * pDialog = new QSimCreateDialog(this);

	pDialog->AddExistingModels();
	if (pDialog->exec() == QDialog::Accepted)
		m_pLblStatus->setText("Ready");
	delete pDialog;
}

void MainWindow::OnFileOpen()
{
	QString strSimFile;

	strSimFile = QFileDialog::getOpenFileName(QString::null, "Simulation Files (*.sim)", g_pMainWindow, "choose simulator dialog", "Open Simulation Configuration File...");
	if (!strSimFile.isEmpty())
	{
		qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
		g_pSimulator->Load(strSimFile);
		m_pLblStatus->setText("Ready");
		qApp->restoreOverrideCursor();
	}
}

void MainWindow::OnFileSave()
{
	QString strSimFile;

	if (g_pSimulator->running())
	{
		if (QMessageBox::question(this, "GrooveNet", "Saving the configuration requires that the currently running simulation be halted. Continue?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::NoButton) == 0) {
			while (!g_pSimulator->wait(MAX_DEADLOCK))
				g_pSimulator->terminate();
		} else
			return;
	}

	strSimFile = QFileDialog::getSaveFileName(QString::null, "Simulation Files (*.sim);;All Files (*)", g_pMainWindow, "write simulator dialog", "Save Simulation Configuration File...");
	if (!strSimFile.isEmpty())
	{
		qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
		g_pSimulator->Save(strSimFile);
		qApp->restoreOverrideCursor();
	}
}

void MainWindow::OnFileConfig()
{
	QConfigureDialog * pDialog = new QConfigureDialog(this, "config");
	if (pDialog->exec() == QDialog::Accepted && g_pSimulator != NULL)
		g_pSimulator->TriggerSettingsChanged();
	delete pDialog;
}

void MainWindow::OnFileExit()
{
	close();
}

void MainWindow::OnSimRun()
{
	QSimRunDialog * pDialog = new QSimRunDialog(this);

	if (pDialog->exec() == QDialog::Accepted)
	{
		std::vector<QString> vecLogFilenames;
		g_pSimulator->m_sSimSettings.iTrials = pDialog->m_iTrials;
		g_pSimulator->m_sSimSettings.tDuration = pDialog->m_tDuration;
		g_pSimulator->m_sSimSettings.tIncrement = pDialog->m_tIncrement;
		g_pSimulator->m_sSimSettings.bSimulationTime = (pDialog->m_tIncrement != timeval0);
		g_pSimulator->m_sSimSettings.vecMessages = pDialog->m_vecMessages;
		g_pSimulator->m_sSimSettings.bProfile = pDialog->m_bProfile;
		pDialog->GetLogFilePaths(vecLogFilenames);
		g_pSimulator->start(vecLogFilenames);
	}

	delete pDialog;
}

void MainWindow::OnSimPause()
{
	if (g_pSimulator->running())
	{
		if (g_pSimulator->isPaused())
			g_pSimulator->resume();
		else
			g_pSimulator->pause();
	}
}

void MainWindow::OnSimSkip()
{
	if (g_pSimulator->running())
	{
		g_pSimulator->skip();
		if (g_pSimulator->isPaused())
			g_pSimulator->resume();
	}
}

void MainWindow::OnSimStop()
{
	qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	while (!g_pSimulator->wait(MAX_DEADLOCK))
		g_pSimulator->terminate();
	g_pSimulator->m_EventQueue.Clear();
	g_pLogger->CloseLogFiles();
	m_pLblStatus->setText("Ready");
	qApp->restoreOverrideCursor();
}

void MainWindow::OnNetInit()
{
	InitNetwork();
}

void MainWindow::OnNetClose()
{
	CloseNetwork();
}

void MainWindow::OnNetServer()
{
	if (IsServerRunning())
		StopServer();
}

void MainWindow::OnNetServer(int id)
{
	StartServer(m_pNetServerMenu->text(id));
}

void MainWindow::closeEvent(QCloseEvent * e)
{
	int ret;
	ret = QMessageBox::question(this, "GrooveNet", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::Escape, QMessageBox::No | QMessageBox::Default, QMessageBox::NoButton);
	switch (ret) {
	case 0:
		e->accept();
		break;
	default:
		e->ignore();
		break;
	}
}

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

#include "StringHelp.h"
#include "MapDB.h"
#include "Simulator.h"
#include "MainWindow.h"
#include "Logger.h"
#include "Settings.h"
#include "CarRegistry.h"
#include "InfrastructureNodeRegistry.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qstatusbar.h>

Simulator::Simulator()
: m_tCurrent(timeval0), m_tStart(timeval0), m_bLoaded(false), m_bCancelled(false), m_bNextTrial(false), m_iPaused(0), m_pMutexPause(new QMutex(true))
{
	m_sSimSettings.tDuration = timeval0;
	m_sSimSettings.tIncrement = timeval0;
	m_sSimSettings.bSimulationTime = false;
	m_sSimSettings.iTrials = 0;
}

Simulator::~Simulator()
{
	if (m_bLoaded)
		Unload();
	delete m_pMutexPause;
	m_pMutexPause = NULL;
}

bool Simulator::New(const std::map<QString, std::map<QString, QString> > & mapModels)
{
	std::map<QString, std::map<QString, QString> >::const_iterator iterModel;
	std::set<QString> setModelsAdded, setModelsAdding;
	std::vector<std::pair<QString, QString> > vecDepends;
	std::map<QString, QString>::const_iterator iterParam;
	unsigned int i, iProgress = (unsigned)-1;
	bool bSuccess = true;

	if (m_bLoaded)
		Unload();

	if (g_pMainWindow != NULL && g_pMainWindow->m_pLblStatus != NULL)
		g_pMainWindow->m_pLblStatus->setText("Creating simulation...");

	for (iterModel = mapModels.begin(), i = 0; iterModel != mapModels.end(); ++iterModel, i++)
	{
		if (i*100/mapModels.size() != iProgress && g_pMainWindow != NULL && g_pMainWindow->m_pLblStatus != NULL)
			g_pMainWindow->m_pLblStatus->setText(QString("Creating simulation... (%1%)").arg(iProgress = i*100/mapModels.size()));
		qApp->processEvents();
		if (setModelsAdded.find(iterModel->first) == setModelsAdded.end() && !internalAddNew(mapModels, iterModel, setModelsAdded, setModelsAdding)) {
			bSuccess = false;
			break;
		}

		if (setModelsAdded.find(iterModel->first) != setModelsAdded.end())
		{
			iterParam = iterModel->second.find(PARAM_DEPENDS);
			if (iterParam == iterModel->second.end())
				vecDepends.push_back(std::pair<QString, QString>(iterModel->first, ""));
			else
				vecDepends.push_back(std::pair<QString, QString>(iterModel->first, iterParam->second));
		}
	}

	if (g_pMainWindow != NULL && g_pMainWindow->m_pLblStatus != NULL)
		g_pMainWindow->m_pLblStatus->setText("Creating simulation... (100%)");
	qApp->processEvents();
	if (bSuccess)
		bSuccess = m_ModelMgr.BuildModelTree(vecDepends) > 0;

	qApp->processEvents();
	if (bSuccess)
		m_bLoaded = true;
	else
		Unload();

	if (g_pMainWindow != NULL) {
		g_pMainWindow->m_pFileEdit->setEnabled(m_bLoaded);
		g_pMainWindow->m_pFileSave->setEnabled(m_bLoaded);
		g_pMainWindow->m_pSimRun->setEnabled(m_bLoaded);
		g_pMainWindow->m_pCentralWidget->tile();
		g_pMainWindow->m_pLblStatus->setText("Ready");
	}
	qApp->processEvents();
	return bSuccess;
}

bool Simulator::internalAddNew(const std::map<QString, std::map<QString, QString> > & mapModels, std::map<QString, std::map<QString, QString> >::const_iterator iterModel, std::set<QString> & setAdded, std::set<QString> & setAdding)
{
	if (!setAdding.insert(iterModel->first).second)
		return false;

	bool bSuccess = true;
	std::map<QString, QString>::const_iterator iterParam = iterModel->second.find(PARAM_DEPENDS);
	QString strType, strDepends = iterParam == iterModel->second.end() ? "" : iterParam->second;
	QStringList listDepends = QStringList::split(';', strDepends, false);
	QStringList::iterator iterDepend;
	for (iterDepend = listDepends.begin(); iterDepend != listDepends.end(); ++iterDepend)
	{
		if ((*iterDepend).compare("NULL") != 0 && setAdded.find(*iterDepend) == setAdded.end())
		{
			std::map<QString, std::map<QString, QString> >::const_iterator iterDependModel = mapModels.find(*iterDepend);
			if (iterDependModel == mapModels.end() || !internalAddNew(mapModels, iterDependModel, setAdded, setAdding)) {
				bSuccess = false;
				break;
			}
		}
	}

	if (bSuccess) {
		iterParam = iterModel->second.find(PARAM_TYPE);
		strType = iterParam == iterModel->second.end() ? "" : iterParam->second;
		bSuccess = m_ModelMgr.AddModel(iterModel->first, strType, iterModel->second);
	}
	if (bSuccess)
		bSuccess = setAdded.insert(iterModel->first).second;
	if (bSuccess)
		setAdding.erase(iterModel->first);
	else
		bSuccess = (QMessageBox::warning(NULL, "GrooveNet", QString("Warning: Failed to create model \"%1\". Continue?").arg(iterModel->first), QMessageBox::Yes | QMessageBox::Escape, QMessageBox::No | QMessageBox::Default, QMessageBox::NoButton) == 0);
	return bSuccess;
}

int Simulator::Load(const QString & strFilename)
{
	QFile file(strFilename);
	QTextStream reader;
	QString line;
	std::vector<std::pair<QString, QString> > vecPairs, vecDepends;
	std::map<QString, QString> mapParams;
	unsigned int i, fsm_state, iProgress = (unsigned)-1;
	int model_index, type_index, depends_index;
	int result;

	if (!file.open(IO_ReadOnly | IO_Translate))
	{
		errno = ENOENT;
		return 0;
	}

	if (m_bLoaded)
		Unload();

	if (g_pMainWindow != NULL && g_pMainWindow->m_pLblStatus != NULL)
		g_pMainWindow->m_pLblStatus->setText("Loading simulation...");
	qApp->processEvents();

	reader.setDevice(&file);

	while (!(line = reader.readLine()).isNull())
	{
		if (line.isEmpty() || line[0] == '%')
			continue;

		/* extract each line's parameters */
		if (!ExtractParams(line, vecPairs))
		{
			/* malformed file? */
			reader.unsetDevice();
			file.close();
			errno = EINVAL;
			return 0;
		}
	}

	reader.unsetDevice();
	file.close();
	qApp->processEvents();

	/* now we parse the parameters -- using a simple FSM */
#define FSM_STATE_SEARCHMODEL 0
#define FSM_STATE_GETPARAMSTYPE 1
#define FSM_STATE_GETPARAMSNOTYPE 2

	fsm_state = FSM_STATE_SEARCHMODEL;
	model_index = depends_index = type_index = -1;
	mapParams.clear();
	for (i = 0; i < vecPairs.size(); i++)
	{
		if (i*100/vecPairs.size() != iProgress && g_pMainWindow != NULL && g_pMainWindow->m_pLblStatus != NULL)
			g_pMainWindow->m_pLblStatus->setText(QString("Loading simulation... (%1%)").arg(iProgress = i * 100 / vecPairs.size()));
		qApp->processEvents();
		switch (fsm_state)
		{
			case FSM_STATE_SEARCHMODEL:
				/* if searching for model */
				if (!vecPairs[i].first.upper().compare(PARAM_MODEL))
				{
					mapParams.clear();
					model_index = i;
					depends_index = type_index = -1;
					fsm_state = FSM_STATE_GETPARAMSTYPE;
				}
				break;
			case FSM_STATE_GETPARAMSTYPE:
				/* if searching for params, type */
				if (!vecPairs[i].first.upper().compare(PARAM_MODEL))
				{
					mapParams.clear();
					model_index = i;
					depends_index = type_index = -1;
				}
				else if (!vecPairs[i].first.upper().compare(PARAM_TYPE))
				{
					type_index = i;
					fsm_state = FSM_STATE_GETPARAMSNOTYPE;
					mapParams[vecPairs[i].first] = vecPairs[i].second;
				}
				else if (!vecPairs[i].first.upper().compare(PARAM_DEPENDS)) {
					depends_index = i;
					mapParams[vecPairs[i].first] = vecPairs[i].second;
				} else
					mapParams[vecPairs[i].first] = vecPairs[i].second;
				break;
			case FSM_STATE_GETPARAMSNOTYPE:
				/* if searching for params, type */
				if (!vecPairs[i].first.upper().compare(PARAM_MODEL))
				{
					if (m_ModelMgr.AddModel(vecPairs[model_index].second, vecPairs[type_index].second, mapParams))
					{
						if (depends_index > -1)
							vecDepends.push_back(std::pair<QString, QString>(vecPairs[model_index].second, vecPairs[depends_index].second));
						else
							vecDepends.push_back(std::pair<QString, QString>(vecPairs[model_index].second, ""));
					}
					mapParams.clear();
					model_index = i;
					depends_index = type_index = -1;
					fsm_state = FSM_STATE_GETPARAMSTYPE;
				}
				else if (!vecPairs[i].first.upper().compare(PARAM_TYPE))
				{
				}
				else if (!vecPairs[i].first.upper().compare(PARAM_DEPENDS)) {
					depends_index = i;
					mapParams[vecPairs[i].first] = vecPairs[i].second;
				} else
					mapParams[vecPairs[i].first] = vecPairs[i].second;
				break;
		}
	}

	if (g_pMainWindow != NULL && g_pMainWindow->m_pLblStatus != NULL)
		g_pMainWindow->m_pLblStatus->setText("Loading simulation... (100%)");
	qApp->processEvents();
	/* take care of last model if there is one */
	if (model_index > -1 && type_index > -1)
	{
		if (m_ModelMgr.AddModel(vecPairs[model_index].second, vecPairs[type_index].second, mapParams))
		{
			if (depends_index > -1)
				vecDepends.push_back(std::pair<QString, QString>(vecPairs[model_index].second, vecPairs[depends_index].second));
			else
				vecDepends.push_back(std::pair<QString, QString>(vecPairs[model_index].second, ""));
		}
	}
	if ((result = m_ModelMgr.BuildModelTree(vecDepends)) > 0)
		m_bLoaded = true;
	else
		Unload();

	if (g_pMainWindow != NULL) {
		g_pMainWindow->m_pFileEdit->setEnabled(m_bLoaded);
		g_pMainWindow->m_pFileSave->setEnabled(m_bLoaded);
		g_pMainWindow->m_pSimRun->setEnabled(m_bLoaded);
		g_pMainWindow->m_pCentralWidget->tile();
		g_pMainWindow->m_pLblStatus->setText("Ready");
	}
	qApp->processEvents();
	return result;
}

void Simulator::Save(const QString & strFilename)
{
	QFile file(strFilename);
	QTextStream writer;
	unsigned int i;

	if (!file.open(IO_WriteOnly | IO_Truncate))
		return;

	writer.setDevice(&file);

	writer << "% Created by GrooveNet Hybrid Simulator on " << QDate::currentDate(Qt::LocalTime).toString("M/dd/yyyy") << endl;
	writer << endl;

	m_ModelMgr.m_modelsMutex.lock();
	m_ModelMgr.MarkAllModelsDirty();

	for (i = 0; i < m_ModelMgr.m_nModelTreeNodes; i++)
	{
		if (MODELTREENODE_ISVALID(m_ModelMgr.m_pModelTreeNodes[i]) && !MODELTREENODE_ISERROR(m_ModelMgr.m_pModelTreeNodes[i]))
		{
			if (MODELTREENODE_ISDIRTY(m_ModelMgr.m_pModelTreeNodes[i]) && !internalSave(writer, &m_ModelMgr.m_pModelTreeNodes[i]))
				m_ModelMgr.m_pModelTreeNodes[i].iBits |= MODELTREEBITS_ERROR;
			else
				m_ModelMgr.m_pModelTreeNodes[i].iBits &= ~MODELTREEBITS_DIRTY;
		}
	}
	m_ModelMgr.ClearAllModelsError();

	m_ModelMgr.m_modelsMutex.unlock();
	writer.unsetDevice();
	file.close();
}

bool Simulator::internalSave(QTextStream & writer, ModelTreeNode * pModelNode)
{
	std::list<ModelTreeNode *>::iterator iterReqModel;
	QString strDepends;
	bool bSuccess = true;
	// initialize models that this model depends on first
	for (iterReqModel = pModelNode->listDepends.begin(); iterReqModel != pModelNode->listDepends.end(); ++iterReqModel)
	{
		if (strDepends.isEmpty())
			strDepends = (*iterReqModel)->strModelName;
		else
			strDepends += (';' + (*iterReqModel)->strModelName);
		if (MODELTREENODE_ISVALID(**iterReqModel) && !MODELTREENODE_ISERROR(**iterReqModel))
		{
			if (MODELTREENODE_ISDIRTY(**iterReqModel) && !internalSave(writer, *iterReqModel)) {
				(*iterReqModel)->iBits |= MODELTREEBITS_ERROR;
				bSuccess = false;
			} else
				(*iterReqModel)->iBits &= ~MODELTREEBITS_DIRTY;
		} else
			bSuccess = false;
	}

	// if we successfully saved the required models, save this model
	if (pModelNode->pModel != NULL && bSuccess) {
		std::map<QString, QString> mapParams;
		std::map<QString, QString>::iterator iterParam;
		pModelNode->pModel->m_mutexUpdate.lock();
		bSuccess = (pModelNode->pModel->Save(mapParams) == 0);
		pModelNode->pModel->m_mutexUpdate.unlock();
		if (bSuccess)
		{
			QString strWrite;

			// write model name, type, and dependencies
			if (strDepends.isEmpty())
				strWrite = QString("MODEL=\"%1\" TYPE=\"%2\"").arg(pModelNode->strModelName).arg(pModelNode->pModel->GetModelType());
			else
				strWrite = QString("MODEL=\"%1\" TYPE=\"%2\" DEPENDS=\"%3\"").arg(pModelNode->strModelName).arg(pModelNode->pModel->GetModelType()).arg(strDepends);
			writer << strWrite << endl;

			// write parameters
			strWrite = QString::null;
			for (iterParam = mapParams.begin(); iterParam != mapParams.end(); ++iterParam)
			{
				if (strWrite.isEmpty())
					strWrite = QString("%1=\"%2\"").arg(iterParam->first).arg(iterParam->second);
				else
					strWrite += QString(" %1=\"%2\"").arg(iterParam->first).arg(iterParam->second);
			}
			if (!strWrite.isEmpty())
				writer << strWrite << endl;
			writer << endl;
		}
	}
	return bSuccess;
}

void Simulator::Unload()
{
	if (g_pMainWindow != NULL && g_pMainWindow->m_pLblStatus != NULL)
		g_pMainWindow->m_pLblStatus->setText("Closing simulation...");
	qApp->processEvents();

	while (!wait(MAX_DEADLOCK));
		terminate();
	g_pLogger->CloseLogFiles();

	m_EventQueue.Clear();
	m_msgCurrentTrack.iSeqNumber = (unsigned)-1;
	m_msgCurrentTrack.ipCar = (unsigned)-1;

	/* erase all models */
	m_ModelMgr.DestroyAllModels();
	g_pMapObjects->clear();
	m_ModelMgr.AddModel("NULL", NULLMODEL_NAME, "", std::map<QString, QString>());
	m_bLoaded = false;

	if (g_pMainWindow != NULL) {
		g_pMainWindow->m_pFileEdit->setEnabled(false);
		g_pMainWindow->m_pFileSave->setEnabled(false);
		g_pMainWindow->m_pSimRun->setEnabled(false);
		g_pMainWindow->m_pLblStatus->setText("Ready");
	}
	qApp->processEvents();
}

void Simulator::start(const std::vector<QString> & vecLogFilenames, Priority priority)
{
	if (m_bLoaded)
	{
		while (!wait(MAX_DEADLOCK))
			terminate();
		m_bNextTrial = false;
		m_bCancelled = false;
		m_tProfileStart = m_tProfileEnd = timeval0;
		if (g_pMainWindow != NULL) {
			g_pMainWindow->m_pSimRun->setEnabled(false);
			g_pMainWindow->m_pSimPause->setEnabled(true);
			g_pMainWindow->m_pSimSkip->setEnabled(m_sSimSettings.iTrials > 0);
			g_pMainWindow->m_pSimStop->setEnabled(true);
		}
		if (g_pSettings->m_sSettings[SETTINGS_GENERAL_LOGGING_NUM].GetValue().bValue)
		{
			if (vecLogFilenames.empty())
				g_pLogger->CreateLogFiles();
			else
				g_pLogger->CreateLogFiles(vecLogFilenames);
		}
		QThread::start(priority);
	}
}

void Simulator::pause()
{
	m_pMutexPause->lock();
	m_iPaused++;
	if (m_iPaused == 1 && g_pMainWindow != NULL && g_pMainWindow->m_pSimPause != NULL) {
		g_pMainWindow->m_pSimPause->setMenuText("Resume");
		g_pMainWindow->m_pSimPause->setToolTip("Resume the currently paused simulation");
	}
	if (m_iPaused == 1 && g_pMainWindow != NULL && g_pMainWindow->m_pLblStatus != NULL)
		g_pMainWindow->m_pLblStatus->setText("Paused");
}

void Simulator::resume()
{
	m_iPaused--;
	if (m_iPaused == 0 && g_pMainWindow != NULL && g_pMainWindow->m_pSimPause != NULL) {
		g_pMainWindow->m_pSimPause->setMenuText("Pause");
		g_pMainWindow->m_pSimPause->setToolTip("Pause the currently running simulation");
	}
	if (m_iPaused == 0 && g_pMainWindow != NULL && g_pMainWindow->m_pLblStatus != NULL)
	{
		if (running())
			g_pMainWindow->m_pLblStatus->setText("Running...");
		else
			g_pMainWindow->m_pLblStatus->setText("Ready");
	}
	m_pMutexPause->unlock();
}

void Simulator::skip()
{
	m_bNextTrial = true;
}

bool Simulator::wait(unsigned long time)
{
	bool bReturn;
	m_bCancelled = true;
	bReturn = QThread::wait(time);
	if (bReturn)
	{
		m_tCurrent = timeval0;
		m_EventQueue.Clear();
		if (g_pMainWindow != NULL) {
			g_pMainWindow->m_pSimRun->setEnabled(true);
			g_pMainWindow->m_pSimPause->setEnabled(false);
			g_pMainWindow->m_pSimSkip->setEnabled(false);
			g_pMainWindow->m_pSimStop->setEnabled(false);
		}
		SetTimeMode(false);
		if (isPaused())
			resume();
	}
	return bReturn;
}

void Simulator::TriggerSettingsChanged()
{
	std::map<QString, Model *>::iterator iterModel;

	m_ModelMgr.m_modelsMutex.lock();
	for (iterModel = m_ModelMgr.m_mapModels.begin(); iterModel != m_ModelMgr.m_mapModels.end(); ++iterModel)
	{
		if (iterModel->second != NULL)
			iterModel->second->SettingsChanged();
	}
	m_ModelMgr.m_modelsMutex.unlock();
}

void Simulator::SendMessage(const EventMessage & event)
{
	SafetyPacket msg;
	std::map<in_addr_t, CarModel *> * pCarRegistry = g_pCarRegistry->acquireLock();
	std::map<in_addr_t, CarModel *>::iterator iterCar = pCarRegistry->find(event.ipSource);
	CarModel * pCar = (iterCar == pCarRegistry->end() ? NULL : iterCar->second);

	if (pCar != NULL)
	{
		Event1Message msgEvent;
		Address sDest;
		pCar->CreateMessage(&msg);
//		msg.m_eType = event.eType;
		msg.m_sBoundingRegion = event.sBoundingRegion;
		msg.m_pData = (unsigned char *)strdup(event.strMessage);
		msg.m_iDataLength = event.strMessage.length() + 1;

		g_pSimulator->m_mutexEvent1Log.lock();
		msgEvent.ID = msg.m_ID.srcID;
		msgEvent.tMessage = event.tTransmit;
		msgEvent.tLifetime = msg.m_tLifetime = event.tLifetime;
		msgEvent.ptOrigin = pCar->GetCurrentPosition();
		if (!event.strDest.isEmpty() && StringToAddress(event.strDest, &sDest))
			msgEvent.ptDest = sDest.ptCoordinates;
		else
			msgEvent.ptDest.Set(0, 0);
		msgEvent.fDistance = 0.f;
		msgEvent.fOriginatorDistance = 0.f;
		msgEvent.iCars = 1;
		g_pSimulator->m_mapEvent1Log.insert(std::pair<PacketSequence, Event1Message>(msgEvent.ID, msgEvent));
		g_pSimulator->m_mutexEvent1Log.unlock();

		g_pSimulator->m_msgCurrentTrack = msg.m_ID.srcID;
		if (pCar->IsLoggingEnabled())
			g_pLogger->WriteMessage(LOGFILE_MESSAGES, &msg);
		pCar->TransmitPacket(&msg);
		if (pCar->m_pCommModel != NULL)
			pCar->m_pCommModel->AddMessageToRebroadcastQueue(msg);
	}
	g_pCarRegistry->releaseLock();
}

void Simulator::run()
{
	unsigned int i, iTrial;
	bool bMonteCarlo;

	if (g_pMainWindow != NULL && g_pMainWindow->m_pLblStatus != NULL)
		g_pMainWindow->m_pLblStatus->setText("Running...");

	bMonteCarlo = (m_sSimSettings.iTrials > 0);
	if (!bMonteCarlo)
		m_sSimSettings.iTrials = 1;

	for (iTrial = 0; iTrial < m_sSimSettings.iTrials && !m_bCancelled; iTrial++)
	{
		SetTimeMode(m_sSimSettings.bSimulationTime, m_sSimSettings.tIncrement);
		m_tCurrent = m_tStart = GetCurrentTime();
	
		m_mapEvent1Log.clear();
		m_msgCurrentTrack.iSeqNumber = (unsigned)-1;
		m_msgCurrentTrack.ipCar = (unsigned)-1;
		qApp->wakeUpGuiThread();

		// perform simulation setup
		for (i = 0; i < m_sSimSettings.vecMessages.size(); i++)
		{
			EventMessage * pEvent = new EventMessage(m_sSimSettings.vecMessages[i]);
			pEvent->tTransmit = pEvent->tTransmit + m_tStart;
			m_EventQueue.AddEvent(SimEvent(pEvent->tTransmit, EVENT_PRIORITY_LOWEST, QString::null, QString::null, EVENT_EVENTMESSAGE_OCCUR, pEvent, DestroyEventMessage));
		}

		// perform pre-run initialization of all models
		m_ModelMgr.m_modelsMutex.lock();
		m_ModelMgr.MarkAllModelsDirty();
		for (i = 0; i < m_ModelMgr.m_nModelTreeNodes; i++)
		{
			if (g_pMainWindow != NULL && g_pMainWindow->m_pLblStatus != NULL && (i == 0 || ((i * 100) / m_ModelMgr.m_nModelTreeNodes > ((i-1) * 100) / m_ModelMgr.m_nModelTreeNodes)))
				g_pMainWindow->m_pLblStatus->setText(QString("Initializing Models...%1%").arg((i * 100) / m_ModelMgr.m_nModelTreeNodes));
	
			if (MODELTREENODE_ISVALID(m_ModelMgr.m_pModelTreeNodes[i]) && !MODELTREENODE_ISERROR(m_ModelMgr.m_pModelTreeNodes[i]))
			{
				if (MODELTREENODE_ISDIRTY(m_ModelMgr.m_pModelTreeNodes[i]) && !prerun(&m_ModelMgr.m_pModelTreeNodes[i]))
					m_ModelMgr.m_pModelTreeNodes[i].iBits |= MODELTREEBITS_ERROR;
				else
					m_ModelMgr.m_pModelTreeNodes[i].iBits &= ~MODELTREEBITS_DIRTY;
			}
		}
		m_ModelMgr.m_modelsMutex.unlock();
		qApp->wakeUpGuiThread();

		if (bMonteCarlo)
		{
			for (i = 0; i < LOGFILES; i++)
				g_pLogger->WriteComment(i, QString("Monte Carlo: Starting Run #%1").arg(iTrial+1));
		}

		m_tProfileStart = GetRealTime();
		while (!m_bCancelled && !m_bNextTrial)
		{
			sleep(1);
			if (!qApp->tryLock()) {
				qApp->wakeUpGuiThread();
				continue;
			}
			if (!m_pMutexPause->tryLock()) {
				qApp->unlock();
				continue;
			}
	
			// get current time
			m_tCurrent = GetCurrentTime();
			if ((bMonteCarlo || m_sSimSettings.tDuration > timeval0) && m_tCurrent - m_tStart >= m_sSimSettings.tDuration)
			{
				m_pMutexPause->unlock();
				qApp->unlock();
				break;
			}

			if (g_pMainWindow != NULL && g_pMainWindow->m_pLblStatus != NULL)
				g_pMainWindow->m_pLblStatus->setText("Running... (" + FormatTime(ToDouble(m_tCurrent - m_tStart), 0) + ")");
	
			std::map<in_addr_t, CarModel *> * pCarRegistry = g_pCarRegistry->acquireLock();
			std::map<in_addr_t, InfrastructureNodeModel *> * pNodeRegistry __attribute__((unused)) = g_pInfrastructureNodeRegistry->acquireLock();
	
			while (!m_EventQueue.IsEmpty() && m_EventQueue.TopEvent().GetTimestamp() <= m_tCurrent)
			{
				SimEvent event(m_EventQueue.TopEvent());
				Model * pDestModel = NULL;
				m_EventQueue.PopEvent();
				if (event.GetDestModel().isEmpty())
				{
					switch (event.GetEventID())
					{
					case EVENT_EVENTMESSAGE_OCCUR:
					{
						if (event.GetEventData() != NULL)
						{
							SendMessage(*(EventMessage *)event.GetEventData());
							delete (EventMessage *)event.GetEventData();
						}
						break;
					}
					default:
						break;
					}
				}
				else
				{
					if (m_ModelMgr.GetModel(event.GetDestModel(), pDestModel) && pDestModel != NULL)
						pDestModel->ProcessEvent(event);
				}
			}
	
			// write events to log file
			std::map<PacketSequence, Event1Message>::iterator iterMessage;
			std::map<in_addr_t, CarModel *>::iterator iterCar;
			float fDistance;
			m_mutexEvent1Log.lock();
			iterMessage = m_mapEvent1Log.begin();
			while (iterMessage != m_mapEvent1Log.end())
			{
				if (iterMessage->second.tMessage + iterMessage->second.tLifetime < m_tCurrent)
				{
					if (iterMessage->first == m_msgCurrentTrack) {
						m_msgCurrentTrack.iSeqNumber = (unsigned)-1;
						m_msgCurrentTrack.ipCar = (unsigned)-1;
					}
					if (iterMessage->second.ptDest.m_iLong != 0 || iterMessage->second.ptDest.m_iLat != 0)
						g_pLogger->LogInfo(QString("[t = %1s] Message %2 from %3 never reached its destination!\n").arg(ToDouble(m_tCurrent - iterMessage->second.tMessage), 0, 'f', 6).arg(iterMessage->second.ID.iSeqNumber).arg(IPAddressToString(iterMessage->second.ID.ipCar)), WARNING_LEVEL_NONE);

					std::map<PacketSequence, Event1Message>::iterator iterTempMessage = iterMessage;
					++iterMessage;
					m_mapEvent1Log.erase(iterTempMessage);
				}
				else
				{
					struct timeval tTemp = iterMessage->second.tMessage;
					for (iterCar = pCarRegistry->begin(); iterCar != pCarRegistry->end(); ++iterCar)
					{
						if (iterCar->second != NULL && (iterCar->first == iterMessage->second.ID.ipCar || iterCar->second->HasMessage(iterMessage->second.ID)))
						{
							fDistance = Distance(iterMessage->second.ptOrigin, iterCar->second->GetCurrentPosition()) * METERSPERMILE;
							if (fDistance > iterMessage->second.fDistance)
								iterMessage->second.fDistance = fDistance;
							if (iterCar->first == iterMessage->second.ID.ipCar && fDistance > iterMessage->second.fOriginatorDistance)
								iterMessage->second.fOriginatorDistance = fDistance;
							if (iterMessage->second.ptDest.m_iLong != 0 || iterMessage->second.ptDest.m_iLat != 0)
							{
								if (iterCar->second->IsActive() && iterCar->second->m_pPhysModel != NULL && iterCar->second->m_pPhysModel->IsCarInRange(iterCar->second->GetCurrentPosition(), iterMessage->second.ptDest))
								{
									iterMessage->second.ptDest.Set(0, 0);
									g_pLogger->LogInfo(QString("[t = %1s] Message %2 from %3 reached its destination!\n").arg(ToDouble(m_tCurrent - iterMessage->second.tMessage), 0, 'f', 6).arg(iterMessage->second.ID.iSeqNumber).arg(IPAddressToString(iterMessage->second.ID.ipCar)), WARNING_LEVEL_NONE);
								}
							}
						}
					}
	
					// change message time to current time for the log file, then change back
					iterMessage->second.tMessage = m_tCurrent - tTemp;
					g_pLogger->WriteMessage(LOGFILE_EVENT1, &iterMessage->second);
					iterMessage->second.tMessage = tTemp;
					++iterMessage;
				}
			}
			m_mutexEvent1Log.unlock();
			g_pInfrastructureNodeRegistry->releaseLock();
			g_pCarRegistry->releaseLock();
	
	//		m_ModelMgr.m_modelsMutex.unlock();
			m_pMutexPause->unlock();
			qApp->unlock();
	
			if (GetTimeMode())
				IncrementTime();
		}
		m_tProfileEnd = GetRealTime();
		if (m_sSimSettings.bProfile)
			g_pLogger->LogInfo(QString("Profiler: Completed %1 seconds in %2 seconds.\n").arg(ToDouble(m_tCurrent) - ToDouble(m_tStart), 0, 'f', 6).arg(ToDouble(m_tProfileEnd) - ToDouble(m_tProfileStart), 0, 'f', 6), WARNING_LEVEL_NONE);
	
		// perform post-run finalization of models
		m_ModelMgr.m_modelsMutex.lock();
		m_ModelMgr.ClearAllModelsError();
		m_ModelMgr.MarkAllModelsDirty();
		for (i = 0; i < m_ModelMgr.m_nModelTreeNodes; i++)
		{
			if (MODELTREENODE_ISVALID(m_ModelMgr.m_pModelTreeNodes[i]) && !MODELTREENODE_ISERROR(m_ModelMgr.m_pModelTreeNodes[i]))
			{
				if (MODELTREENODE_ISDIRTY(m_ModelMgr.m_pModelTreeNodes[i]) && !postrun(&m_ModelMgr.m_pModelTreeNodes[i]))
					m_ModelMgr.m_pModelTreeNodes[i].iBits |= MODELTREEBITS_ERROR;
				else
					m_ModelMgr.m_pModelTreeNodes[i].iBits &= ~MODELTREEBITS_DIRTY;
			}
		}
		m_ModelMgr.m_modelsMutex.unlock();

		m_EventQueue.Clear();
		m_bNextTrial = false;
	}

	if (g_pMainWindow != NULL) {
		g_pMainWindow->m_pLblStatus->setText("Ready");
		g_pMainWindow->m_pSimRun->setEnabled(true);
		g_pMainWindow->m_pSimPause->setEnabled(false);
		g_pMainWindow->m_pSimSkip->setEnabled(false);
		g_pMainWindow->m_pSimStop->setEnabled(false);
	}
}

bool Simulator::prerun(ModelTreeNode * pModelNode)
{
	std::list<ModelTreeNode *>::iterator iterReqModel;
	bool bSuccess = true;
	// initialize models that this model depends on first
	for (iterReqModel = pModelNode->listDepends.begin(); iterReqModel != pModelNode->listDepends.end(); ++iterReqModel)
	{
		if (MODELTREENODE_ISVALID(**iterReqModel) && !MODELTREENODE_ISERROR(**iterReqModel))
		{
			if (MODELTREENODE_ISDIRTY(**iterReqModel) && !prerun(*iterReqModel)) {
				(*iterReqModel)->iBits |= MODELTREEBITS_ERROR;
				bSuccess = false;
			} else
				(*iterReqModel)->iBits &= ~MODELTREEBITS_DIRTY;
		} else
			bSuccess = false;
	}

	// if we successfully pre-ran the required models, pre-run this model
	if (pModelNode->pModel != NULL && bSuccess) {
		pModelNode->pModel->m_mutexUpdate.lock();
		bSuccess = (pModelNode->pModel->PreRun() == 0);
		pModelNode->pModel->m_mutexUpdate.unlock();
	}
	return bSuccess;
}
/*
bool Simulator::iteration(ModelTreeNode * pModelNode, struct timeval tCurrent)
{
	std::list<ModelTreeNode *>::iterator iterReqModel;
	bool bSuccess = true;
	// initialize models that this model depends on first
	for (iterReqModel = pModelNode->listDepends.begin(); iterReqModel != pModelNode->listDepends.end(); ++iterReqModel)
	{
		if (MODELTREENODE_ISVALID(**iterReqModel) && !MODELTREENODE_ISERROR(**iterReqModel))
		{
			if (MODELTREENODE_ISDIRTY(**iterReqModel) && !iteration(*iterReqModel, tCurrent)) {
				(*iterReqModel)->iBits |= MODELTREEBITS_ERROR;
				bSuccess = false;
			} else
				(*iterReqModel)->iBits &= ~MODELTREEBITS_DIRTY;
		} else
			bSuccess = false;
	}

	// if we successfully updated the required models, update this model
	if (pModelNode->pModel != NULL && bSuccess) {
		pModelNode->pModel->m_mutexUpdate.lock();
		bSuccess = (pModelNode->pModel->Iteration(tCurrent) == 0);
		pModelNode->pModel->m_mutexUpdate.unlock();
	}
	// if successful, a post-iteration step is necessary for this model
	if (bSuccess)
		pModelNode->iBits |= MODELTREEBITS_POSTITER;
	return bSuccess;
}

bool Simulator::postiteration(ModelTreeNode * pModelNode)
{
	std::list<ModelTreeNode *>::iterator iterReqModel;
	bool bSuccess = true;
	// initialize models that this model depends on first
	for (iterReqModel = pModelNode->listDepends.begin(); iterReqModel != pModelNode->listDepends.end(); ++iterReqModel)
	{
		if (MODELTREENODE_ISVALID(**iterReqModel) && !MODELTREENODE_ISERROR(**iterReqModel))
		{
			if (((*iterReqModel)->iBits & MODELTREEBITS_POSTITER) && !postiteration(*iterReqModel)) {
				(*iterReqModel)->iBits |= MODELTREEBITS_ERROR;
				bSuccess = false;
			}
			(*iterReqModel)->iBits &= ~MODELTREEBITS_POSTITER;
		} else
			bSuccess = false;
	}

	// if we successfully post-updated the required models, post-update this model
	if (pModelNode->pModel != NULL && bSuccess) {
		pModelNode->pModel->m_mutexUpdate.lock();
		bSuccess = (pModelNode->pModel->PostIteration() == 0);
		pModelNode->pModel->m_mutexUpdate.unlock();
	}
	return bSuccess;
}
*/
bool Simulator::postrun(ModelTreeNode * pModelNode)
{
	std::list<ModelTreeNode *>::iterator iterReqModel;
	bool bSuccess = true;
	// initialize models that this model depends on first
	for (iterReqModel = pModelNode->listDepends.begin(); iterReqModel != pModelNode->listDepends.end(); ++iterReqModel)
	{
		if (MODELTREENODE_ISVALID(**iterReqModel) && !MODELTREENODE_ISERROR(**iterReqModel))
		{
			if (MODELTREENODE_ISDIRTY(**iterReqModel) && !postrun(*iterReqModel)) {
				(*iterReqModel)->iBits |= MODELTREEBITS_ERROR;
				bSuccess = false;
			} else
				(*iterReqModel)->iBits &= ~MODELTREEBITS_DIRTY;
		} else
			bSuccess = false;
	}

	// if we successfully post-ran the required models, post-run this model
	if (pModelNode->pModel != NULL && bSuccess) {
		pModelNode->pModel->m_mutexUpdate.lock();
		bSuccess = (pModelNode->pModel->PostRun() == 0);
		pModelNode->pModel->m_mutexUpdate.unlock();
	}
	return bSuccess;
}

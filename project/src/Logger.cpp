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

#include "Global.h"
#include "Logger.h"
#include "StringHelp.h"
#include "MainWindow.h"

#include <qmessagebox.h>
#include <qapplication.h>
#include <qfiledialog.h>

const char * g_strLogFileNames[LOGFILES] = {"GrooveNet Message Log", "Event Log 1", "Vehicle Neighbor Log"};

Logger::Logger()
{
	unsigned int i;
	for (i = 0; i < LOGFILES; i++)
		m_pLogFiles[i] = NULL;

#ifdef DEBUG
	i = 1;
	QFileInfo fi;
	QString strDebug;
	do
	{
		strDebug = QString("debug%1.txt").arg(i);
		fi.setFile(strDebug);
		i++;
	}
	while (fi.exists());
	m_pLogDebug = fopen(strDebug, "w");
#else
	m_pLogDebug = NULL;
#endif

	LogInfo(QString("[Logging started at %1]\n").arg(QDateTime::currentDateTime(Qt::LocalTime).toString(Qt::LocalDate)));
}

Logger::~Logger()
{
	CloseLogFiles();

	LogInfo("[Logging stopped]\n");

	if (m_pLogDebug != NULL) {
		m_mutexLogDebug.lock();
		fflush(m_pLogDebug);
		fclose(m_pLogDebug);
		m_pLogDebug = NULL;
		m_mutexLogDebug.unlock();
	}
}

bool Logger::PromptLogWarning(const QString & strTitle, const QString & strError, const int iWarningLevel)
{
	if (m_pLogDebug != NULL) {
		m_mutexLogDebug.lock();
		fprintf(m_pLogDebug, "Warning [level %d]: %s\n", iWarningLevel, (const char *)strError);
		fflush(m_pLogDebug);
		m_mutexLogDebug.unlock();
	}
	if (iWarningLevel <= CURRENT_WARNING_LEVEL) {
		// display message box
		int ret = QMessageBox::warning(NULL, strTitle, strError, QMessageBox::Abort, QMessageBox::Retry, QMessageBox::Ignore);
		// if fatal error, terminate application
		switch (ret)
		{
		case 0: // abort
			QApplication::beep();
			exit(-1);
			return false;
		case 1: // retry
			return true;
		default: // ignore
			return false;
		}
	} else
		return false;
}

void Logger::LogWarning(const QString & strTitle, const QString & strWarning, const int iWarningLevel)
{
	if (m_pLogDebug != NULL) {
		m_mutexLogDebug.lock();
		fprintf(m_pLogDebug, "Warning [level %d]: %s\n", iWarningLevel, (const char *)strWarning);
		fflush(m_pLogDebug);
		m_mutexLogDebug.unlock();
	}
	if (iWarningLevel <= CURRENT_WARNING_LEVEL)
	{
		// display message box
		QMessageBox::warning(NULL, strTitle, strWarning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	}
}

bool Logger::PromptLogError(const QString & strTitle, const QString & strError, const bool bFatal)
{
	// display message box
	if (bFatal)
	{
		if (m_pLogDebug != NULL) {
			m_mutexLogDebug.lock();
			fprintf(m_pLogDebug, "Fatal Error: %s\n", (const char *)strError);
			fflush(m_pLogDebug);
			m_mutexLogDebug.unlock();
		}
		int ret = QMessageBox::critical(NULL, strTitle, strError, QMessageBox::Retry, QMessageBox::Cancel);
		switch (ret)
		{
		case 0: // retry
			return true;
		default: // cancel
			QApplication::beep();
			exit(-1);
			return false;
		}
	}
	else
	{
		if (m_pLogDebug != NULL) {
			m_mutexLogDebug.lock();
			fprintf(m_pLogDebug, "Error: %s\n", (const char *)strError);
			fflush(m_pLogDebug);
			m_mutexLogDebug.unlock();
		}
		int ret = QMessageBox::critical(NULL, strTitle, strError, QMessageBox::Abort, QMessageBox::Retry, QMessageBox::Ignore);
		// TODO: write error to log
		switch (ret)
		{
		case 0: // abort
			QApplication::beep();
			exit(-1);
			return false;
		case 1: // retry
			return true;
		default: // ignore
			return false;
		}
	}
}

void Logger::LogError(const QString & strTitle, const QString & strError, const bool bFatal)
{
	if (m_pLogDebug != NULL) {
		m_mutexLogDebug.lock();
		if (bFatal)
			fprintf(m_pLogDebug, "Fatal Error: %s\n", (const char *)strError);
		else
			fprintf(m_pLogDebug, "Error: %s\n", (const char *)strError);
		fflush(m_pLogDebug);
		m_mutexLogDebug.unlock();
	}

	// display message box
	QMessageBox::critical(NULL, strTitle, strError, bFatal ? QMessageBox::Abort : QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	// if fatal error, terminate application
	if (bFatal)
	{
		QApplication::beep();
		exit(-1);
	}
}

void Logger::LogInfo(const QString & strInfo, const int iWarningLevel)
{
	if (iWarningLevel <= CURRENT_WARNING_LEVEL)
	{
		if (m_pLogDebug != NULL) {
			m_mutexLogDebug.lock();
			fprintf(m_pLogDebug, "%s", (const char *)strInfo);
			fflush(m_pLogDebug);
			m_mutexLogDebug.unlock();
		}
	
		// write info to console
		fprintf(stdout, "%s", (const char *)strInfo);
		fflush(stdout);
	}
}

void Logger::CreateLogFiles()
{
	unsigned int i = 0;

	CloseLogFiles();

	for (i = 0; i < LOGFILES; i++)
	{
		QString strLogFile;

		while (true)
		{
			strLogFile = QFileDialog::getSaveFileName(QString::null, "Log Files (*.txt)", g_pMainWindow, "choose log file dialog", QString("Specify %1...").arg(g_strLogFileNames[i]));
			if (!strLogFile.isEmpty())
			{
				if ((m_pLogFiles[i] = fopen(strLogFile, "w")) == NULL)
				{
					if (!PromptLogError(PACKAGE_TITLE, QString("Could not open %1 for writing").arg(strLogFile), false))
						break; // ignore
				} else {
					WriteHeader(i);
					break;
				}
			} else
				break;
		}
	}
}

void Logger::CreateLogFiles(const std::vector<QString> & vecFilenames)
{
	unsigned int i = 0;

	CloseLogFiles();

	for (i = 0; i < LOGFILES; i++)
	{
		while (!vecFilenames[i].isEmpty())
		{
			if ((m_pLogFiles[i] = fopen(vecFilenames[i], "w")) == NULL)
			{
				if (!PromptLogError(PACKAGE_TITLE, QString("Could not open %1 for writing").arg(vecFilenames[i]), false))
					break; // ignore
			} else {
				WriteHeader(i);
				break;
			}
		}
	}
}

void Logger::CloseLogFiles()
{
	unsigned int i;
	for (i = 0; i < LOGFILES; i++)
	{
		if (m_pLogFiles[i] != NULL) {
			m_mutexLogFiles[i].lock();
			fflush(m_pLogFiles[i]);
			fclose(m_pLogFiles[i]);
			m_pLogFiles[i] = NULL;
			m_mutexLogFiles[i].unlock();
		}
	}
}

void Logger::WriteHeader(unsigned int iLogFile)
{
	m_mutexLogFiles[iLogFile].lock();
	if (m_pLogFiles[iLogFile] != NULL)
	{
		switch (iLogFile)
		{
		case LOGFILE_MESSAGES:
			fprintf(m_pLogFiles[iLogFile], "%% #      Time                Vehicle         Longitude   Latitude    Speed Heading IP (TX)         IP (RX)         TX Time             RX Time             RSSI SNR  Hops Data\n");
			break;
		case LOGFILE_EVENT1:
			fprintf(m_pLogFiles[iLogFile], "%% Time              #        Vehicle         Max. Distance  Originator Distance  Cars Reached\n");
			break;
		case LOGFILE_NEIGHBORS:
	#ifdef MULTILANETEST
			fprintf(m_pLogFiles[iLogFile], "%% Time              Vehicle         Neighbors Messages  Collisions Lane\n");
	#else
			fprintf(m_pLogFiles[iLogFile], "%% Time              Vehicle         Neighbors Messages  Collisions\n");
	#endif
			break;
		default:
			break;
		}
		fflush(m_pLogFiles[iLogFile]);
	}
	m_mutexLogFiles[iLogFile].unlock();
}

void Logger::WriteComment(unsigned int iLogFile, const QString & strComment)
{
	const char * pPrint;
	QStringList listLines = QStringList::split('\n', strComment, true);
	QStringList::iterator iterLine;
	m_mutexLogFiles[iLogFile].lock();

	if (m_pLogFiles[iLogFile] != NULL)
	{
		for (iterLine = listLines.begin(); iterLine != listLines.end(); ++iterLine)
		{
			pPrint = (*iterLine).isEmpty() ? NULL : (*iterLine).ascii();
			if (pPrint == NULL)
				fprintf(m_pLogFiles[iLogFile], "%%\n");
			else
				fprintf(m_pLogFiles[iLogFile], "%% %s\n", pPrint);
		}
		fflush(m_pLogFiles[iLogFile]);
	}
	m_mutexLogFiles[iLogFile].unlock();
}

void Logger::WriteMessage(unsigned int iLogFile, const void * pMessage)
{
	m_mutexLogFiles[iLogFile].lock();
	if (m_pLogFiles[iLogFile] != NULL)
	{
		switch (iLogFile)
		{
		case LOGFILE_MESSAGES:
		{
			const SafetyPacket & msg = *(const SafetyPacket *)pMessage;
#ifdef LOGEVENTSONLY
			if (msg.m_ePacketType != ptSafety)
				break;
#endif
			if (msg.m_ipRX == 0)
			{
				fprintf(m_pLogFiles[iLogFile],
					"%-8u %-19.6f %-15s %-11.6f %-11.6f %-5hd %-7.2f %-15s                 %-19.6f                                    %s\n",
					msg.m_ID.srcID.iSeqNumber,
					ToDouble(msg.m_tTime),
					(const char *)IPAddressToString(msg.m_ID.srcID.ipCar),
					msg.m_ptPosition.m_iLong * 1e-6,
					msg.m_ptPosition.m_iLat * 1e-6,
					msg.m_iSpeed,
					msg.m_iHeading * 1e-2,
					(const char *)IPAddressToString(msg.m_ipTX),
					ToDouble(msg.m_tTX),
					msg.m_iDataLength > 0 ? (const char *)msg.m_pData : "");
			}
			else
			{
				fprintf(m_pLogFiles[iLogFile],
					"%-8u %-19.6f %-15s %-11.6f %-11.6f %-5hd %-7.2f %-15s %-15s %-19.6f %-19.6f %-4hd %-4hd %-4hd %s\n",
					msg.m_ID.srcID.iSeqNumber,
					ToDouble(msg.m_tTime),
					(const char *)IPAddressToString(msg.m_ID.srcID.ipCar),
					msg.m_ptPosition.m_iLong * 1e-6,
					msg.m_ptPosition.m_iLat * 1e-6,
					msg.m_iSpeed,
					msg.m_iHeading * 1e-2,
					(const char *)IPAddressToString(msg.m_ipTX),
					(const char *)IPAddressToString(msg.m_ipRX),
					ToDouble(msg.m_tTX),
					ToDouble(msg.m_tRX),
					msg.m_iRSSI,
					msg.m_iSNR,
					msg.m_iTTL,
					msg.m_iDataLength > 0 ? (const char *)msg.m_pData : "");
			}
			break;
		}
		case LOGFILE_EVENT1:
		{
			const Event1Message & msg = *(const Event1Message *)pMessage;
			fprintf(m_pLogFiles[iLogFile], "%-19.6f %-8u %-15s %-11.3f    %-11.3f          %-8u\n",
			  ToDouble(msg.tMessage),
			  msg.ID.iSeqNumber,
			  (const char *)IPAddressToString(msg.ID.ipCar),
			  msg.fDistance,
			  msg.fOriginatorDistance,
			  msg.iCars);
			break;
		}
		case LOGFILE_NEIGHBORS:
		{
			const NeighborMessage & msg = *(const NeighborMessage *)pMessage;
#ifdef MULTILANETEST
			fprintf(m_pLogFiles[iLogFile], "%-19.6f %-15s %-9u %-9u %-9u %-1u\n",
			  ToDouble(msg.tMessage),
			  (const char *)IPAddressToString(msg.ipCar),
			  msg.iNeighbors,
			  msg.iMessages,
			  msg.iCollisionCount,
			  msg.iLane);
#else
			fprintf(m_pLogFiles[iLogFile], "%-19.6f %-15s %-9u %-9u %-9u\n",
			  ToDouble(msg.tMessage),
			  (const char *)IPAddressToString(msg.ipCar),
			  msg.iNeighbors,
			  msg.iMessages,
			  msg.iCollisionCount);
#endif
			break;
		}
		default:
			break;
		}
		fflush(m_pLogFiles[iLogFile]);
	}
	m_mutexLogFiles[iLogFile].unlock();
}

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

#ifndef _LOGGER_H
#define _LOGGER_H

#define WARNING_LEVEL_NONE 0
#define WARNING_LEVEL_SEVERE 1
#define WARNING_LEVEL_MAJOR 2
#define WARNING_LEVEL_MINOR 3
#define WARNING_LEVEL_DEBUG 4

#ifdef DEBUG
#define CURRENT_WARNING_LEVEL WARNING_LEVEL_DEBUG
#else
#define CURRENT_WARNING_LEVEL WARNING_LEVEL_SEVERE
#endif

#include <qstring.h>
#include <qmutex.h>
#include <stdio.h>

#include "Network.h"

#define LOGFILES 3
#define LOGFILE_MESSAGES 0
#define LOGFILE_EVENT1 1
#define LOGFILE_NEIGHBORS 2

#define LOGEVENTSONLY

class Logger
{
public:
	Logger();
	~Logger();

	bool PromptLogWarning(const QString & strTitle, const QString & strWarning, const int iWarningLevel); // true if retry, false if ignore, doesn't return if abort
	void LogWarning(const QString & strTitle, const QString & strWarning, const int iWarningLevel);
	bool PromptLogError(const QString & strTitle, const QString & strError, const bool bFatal = false); // true if retry, false if ignore, doesn't return if abort
	void LogError(const QString & strTitle, const QString & strError, const bool bFatal = false);
	void LogInfo(const QString & strInfo, const int iWarningLevel = WARNING_LEVEL_NONE);

	void WriteHeader(unsigned int iLogFile);
	void WriteComment(unsigned int iLogFile, const QString & strComment);
	void WriteMessage(unsigned int iLogFile, const void * pMessage);
	void CreateLogFiles();
	void CreateLogFiles(const std::vector<QString> & vecFilenames);
	void CloseLogFiles();

protected:
	FILE * m_pLogFiles[LOGFILES];
	QMutex m_mutexLogFiles[LOGFILES];
	FILE * m_pLogDebug;
	QMutex m_mutexLogDebug;
};

extern Logger * g_pLogger;
extern const char * g_strLogFileNames[LOGFILES];

#endif

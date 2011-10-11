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

#ifndef _SIMULATOR_H
#define _SIMULATOR_H

#include "ModelMgr.h"
#include "Message.h"
#include "SimBase.h"

#include <qthread.h>
#include <set>

#ifndef MAX_DEADLOCK
#define MAX_DEADLOCK 30000
#endif

#define PARAM_MODEL "MODEL"
#define PARAM_TYPE "TYPE"
#define PARAM_DEPENDS "DEPENDS"

class QTextStream;

#define EVENT_EVENTMESSAGE_OCCUR 348756

typedef struct EventMessageStruct
{
	//Message::MessageType eType;
	QString strMessage;
	struct timeval tTransmit;
	struct timeval tLifetime;
	in_addr_t ipSource;
	SafetyPacket::BoundingRegion sBoundingRegion;
	QString strDest; // when message is in range of destination location, will notify the user
} EventMessage;

inline bool CompareEventMessages(const EventMessage & x, const EventMessage & y)
{
	return x.tTransmit > y.tTransmit;
}

inline void DestroyEventMessage(void * ptr)
{
	delete (EventMessage *)ptr;
}

typedef struct SimulatorSettingsStruct
{
	struct timeval tDuration;
	unsigned int iTrials;
	std::vector<EventMessage> vecMessages;
	bool bSimulationTime;
	struct timeval tIncrement;
	bool bProfile;
} SimulatorSettings;

class Simulator : public QThread
{
public:
	Simulator();
	~Simulator();

	bool New(const std::map<QString, std::map<QString, QString> > & mapModels);
	int Load(const QString & strFilename);
	inline bool IsLoaded() const
	{
		return m_bLoaded;
	}
	void Save(const QString & strFilename);
	void Unload();

	virtual void start(const std::vector<QString> & vecLogFilenames, Priority priority = InheritPriority);
	virtual bool wait(unsigned long time = ULONG_MAX);
	void pause();
	void resume();
	void skip();
	inline bool isPaused() const
	{
		return m_iPaused > 0;
	}
	void SendMessage(const EventMessage & event);

	SimEventQueue m_EventQueue;
	ModelMgr m_ModelMgr;
	SimulatorSettings m_sSimSettings;
	struct timeval m_tCurrent, m_tStart, m_tProfileStart, m_tProfileEnd;

	std::map<PacketSequence, Event1Message> m_mapEvent1Log;
	PacketSequence m_msgCurrentTrack;
	QMutex m_mutexEvent1Log;

	void TriggerSettingsChanged();

protected:
	bool internalAddNew(const std::map<QString, std::map<QString, QString> > & mapModels, std::map<QString, std::map<QString, QString> >::const_iterator iterModel, std::set<QString> & setAdded, std::set<QString> & setAdding);
	bool internalSave(QTextStream & writer, ModelTreeNode * pModelNode);

	void run();

	bool prerun(ModelTreeNode * pModelNode);
	bool iteration(ModelTreeNode * pModelNode, struct timeval tCurrent);
	bool postiteration(ModelTreeNode * pModelNode);
	bool postrun(ModelTreeNode * pModelNode);

	bool m_bLoaded;
	bool m_bCancelled, m_bNextTrial;
	unsigned int m_iPaused;
	QMutex * m_pMutexPause;
};

extern Simulator * g_pSimulator;

#endif

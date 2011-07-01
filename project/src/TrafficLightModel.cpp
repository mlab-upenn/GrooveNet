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

#include "TrafficLightModel.h"
#include "StringHelp.h"
#include "MapDB.h"
#include "Simulator.h"

#define TRAFFICLIGHTMODEL_PARAM_GREENLIGHT "GREENLIGHTTIME"
#define TRAFFICLIGHTMODEL_PARAM_GREENLIGHT_DEFAULT "30"
#define TRAFFICLIGHTMODEL_PARAM_GREENLIGHT_DESC "GREENLIGHTTIME (seconds) -- The duration of a single green light."

// divide total group into 100 vertex groups
#define VERTEX_COUNT_MAX 100

TrafficLightModel::TrafficLightModel(const QString & strModelName)
: Model(strModelName), m_tGreenLightTime(timeval0)
{
}

TrafficLightModel::TrafficLightModel(const TrafficLightModel & copy)
: Model(copy), m_tGreenLightTime(copy.m_tGreenLightTime)
{
}

TrafficLightModel::~TrafficLightModel()
{
}

TrafficLightModel & TrafficLightModel::operator = (const TrafficLightModel & copy)
{
	Model::operator = (copy);

	m_tGreenLightTime = copy.m_tGreenLightTime;
	return *this;
}

int TrafficLightModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;

	if (Model::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, TRAFFICLIGHTMODEL_PARAM_GREENLIGHT, TRAFFICLIGHTMODEL_PARAM_GREENLIGHT_DEFAULT);
	m_tGreenLightTime = MakeTime(ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL));
	g_pMapDB->UseTrafficLights(true);

	return 0; // successful
}

int TrafficLightModel::PreRun()
{
	if (Model::PreRun())
		return 1;

	double fDelay = ToDouble(m_tGreenLightTime) / VERTEX_COUNT_MAX, fStart = ToDouble(g_pSimulator->m_tCurrent);

	for (unsigned int i = 0; i < VERTEX_COUNT_MAX; i++, fStart += fDelay)
		g_pSimulator->m_EventQueue.AddEvent(SimEvent(MakeTime(fStart), EVENT_PRIORITY_HIGHEST, m_strModelName, m_strModelName, EVENT_TRAFFICLIGHTMODEL_SIGNAL, (void *)i));

	g_pMapDB->ResetTrafficLights();

	return 0;
}

int TrafficLightModel::ProcessEvent(SimEvent & event)
{
	unsigned int i;
	
	if (Model::ProcessEvent(event))
		return 1;

	switch (event.GetEventID())
	{
	case EVENT_TRAFFICLIGHTMODEL_SIGNAL:
		for (i = (unsigned int)event.GetEventData(); i < g_pMapDB->GetVertexCount(); i += VERTEX_COUNT_MAX)
		{
			Vertex & vertex = g_pMapDB->GetVertex(i);
			vertex.iRoadPermitted++;
			if (vertex.iRoadPermitted >= vertex.vecRoads.size())
				vertex.iRoadPermitted = 0;
		}
		event.SetTimestamp(event.GetTimestamp() + m_tGreenLightTime);
		g_pSimulator->m_EventQueue.AddEvent(event);
		break;
	default:
		break;
	}
	return 0;
}

/*
int TrafficLightModel::Iteration(struct timeval tCurrent)
{
	struct timeval tElapsed = tCurrent - m_tLastIteration;
	unsigned int iCount = (unsigned int)floor((ToDouble(tElapsed) * VERTEX_COUNT_MAX) / m_iGreenLightTime), i, j;

	if (iCount == 0)
		return 2;

	if (Model::Iteration(tCurrent))
		return 1;

	for (i = 0; i < iCount; i++)
	{
		for (j = 0; j < g_pMapDB->GetVertexCount(); j += VERTEX_COUNT_MAX)
		{
			if (i + j + m_iVertexCounter >= g_pMapDB->GetVertexCount())
				break;
			Vertex & vertex = g_pMapDB->GetVertex(i + j + m_iVertexCounter);
			vertex.iRoadPermitted++;
			if (vertex.iRoadPermitted >= vertex.vecRoads.size())
				vertex.iRoadPermitted = 0;
		}
		m_iVertexCounter++;
		if (m_iVertexCounter == VERTEX_COUNT_MAX)
			m_iVertexCounter = 0;
	}
	return 0;
}
*/
int TrafficLightModel::Save(std::map<QString, QString> & mapParams)
{
	if (Model::Save(mapParams))
		return 1;

	mapParams[TRAFFICLIGHTMODEL_PARAM_GREENLIGHT] = QString("%1").arg(ToDouble(m_tGreenLightTime));

	return 0; // successful
}

int TrafficLightModel::Cleanup()
{
	g_pMapDB->UseTrafficLights(false);

	if (Model::Cleanup())
		return 1;

	return 0;
}

void TrafficLightModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	Model::GetParams(mapParams);

	mapParams[TRAFFICLIGHTMODEL_PARAM_GREENLIGHT].strValue = TRAFFICLIGHTMODEL_PARAM_GREENLIGHT_DEFAULT;
	mapParams[TRAFFICLIGHTMODEL_PARAM_GREENLIGHT].strDesc = TRAFFICLIGHTMODEL_PARAM_GREENLIGHT_DESC;
	mapParams[TRAFFICLIGHTMODEL_PARAM_GREENLIGHT].eType = (ModelParameterType)(ModelParameterTypeFloat | ModelParameterFixed);
	mapParams[TRAFFICLIGHTMODEL_PARAM_GREENLIGHT].strAuxData = QString("%1:").arg(0.f);
}

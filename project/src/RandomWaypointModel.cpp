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

#include "RandomWaypointModel.h"
#include "StringHelp.h"

#define RANDOMWAYPOINTMODEL_START_PARAM "START"
#define RANDOMWAYPOINTMODEL_START_PARAM_DEFAULT "0,0"
#define RANDOMWAYPOINTMODEL_START_PARAM_DESC "START (TIGER coordinates) -- The longitude and latitute to start the vehicle's trip from."
#define RANDOMWAYPOINTMODEL_LOWSPEED_PARAM "LOWSPEED"
#define RANDOMWAYPOINTMODEL_LOWSPEED_PARAM_DEFAULT "0"
#define RANDOMWAYPOINTMODEL_LOWSPEED_PARAM_DESC "LOWSPEED (mph) -- The slowest possible speed of vehicle"
#define RANDOMWAYPOINTMODEL_HIGHSPEED_PARAM "HIGHSPEED"
#define RANDOMWAYPOINTMODEL_HIGHSPEED_PARAM_DEFAULT "0"
#define RANDOMWAYPOINTMODEL_HIGHSPEED_PARAM_DESC "HIGHSPEED (mph) -- The fastest possible speed of vehicle"
#define RANDOMWAYPOINTMODEL_INTERVAL_PARAM "INTERVAL"
#define RANDOMWAYPOINTMODEL_INTERVAL_PARAM_DEFAULT "10"
#define RANDOMWAYPOINTMODEL_INTERVAL_PARAM_DESC "INTERVAL (seconds) -- The time between decisions"

RandomWaypointModel::RandomWaypointModel(const QString & strModelName)
: SimUnconstrainedMobilityModel(strModelName), m_iLowSpeed(0), m_iHighSpeed(0), m_tInterval(MakeTime(10, 0)), m_fProgress(0.f)
{
}

RandomWaypointModel::RandomWaypointModel(const RandomWaypointModel & copy)
: SimUnconstrainedMobilityModel(copy), m_ptStart(copy.m_ptStart), m_iLowSpeed(copy.m_iLowSpeed), m_iHighSpeed(copy.m_iHighSpeed), m_tInterval(copy.m_tInterval), m_fProgress(copy.m_fProgress)
{
}

RandomWaypointModel::~RandomWaypointModel()
{
}

RandomWaypointModel & RandomWaypointModel::operator = (const RandomWaypointModel & copy)
{
	SimUnconstrainedMobilityModel::operator = (copy);

	m_ptStart = copy.m_ptStart;
	m_iLowSpeed = copy.m_iLowSpeed;
	m_iHighSpeed = copy.m_iHighSpeed;
	m_tInterval = copy.m_tInterval;
	m_fProgress = copy.m_fProgress;

	return *this;
}

int RandomWaypointModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;

	if (SimUnconstrainedMobilityModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, RANDOMWAYPOINTMODEL_START_PARAM, RANDOMWAYPOINTMODEL_START_PARAM_DEFAULT);
	if (!m_ptStart.FromString(strValue)) {
		m_ptStart.Set(0, 0);
		return 2;
	}

	strValue = GetParam(mapParams, RANDOMWAYPOINTMODEL_LOWSPEED_PARAM, RANDOMWAYPOINTMODEL_LOWSPEED_PARAM_DEFAULT);
	m_iLowSpeed = (short)ValidateNumber(StringToNumber(strValue), 0, 1000);

	strValue = GetParam(mapParams, RANDOMWAYPOINTMODEL_HIGHSPEED_PARAM, RANDOMWAYPOINTMODEL_HIGHSPEED_PARAM_DEFAULT);
	m_iHighSpeed = (short)ValidateNumber(StringToNumber(strValue), m_iLowSpeed, 1000);

	strValue = GetParam(mapParams, RANDOMWAYPOINTMODEL_INTERVAL_PARAM, RANDOMWAYPOINTMODEL_INTERVAL_PARAM_DEFAULT);
	m_tInterval = MakeTime(ValidateNumber(StringToNumber(strValue), 0, 1000));
	return 0;
}

int RandomWaypointModel::Save(std::map<QString, QString> & mapParams)
{
	QString strValue;
	if (SimUnconstrainedMobilityModel::Save(mapParams))
		return 1;

	mapParams[RANDOMWAYPOINTMODEL_START_PARAM] = m_ptStart.ToString();
	mapParams[RANDOMWAYPOINTMODEL_LOWSPEED_PARAM] = QString("%1").arg(m_iLowSpeed);
	mapParams[RANDOMWAYPOINTMODEL_HIGHSPEED_PARAM] = QString("%1").arg(m_iHighSpeed);
	mapParams[RANDOMWAYPOINTMODEL_INTERVAL_PARAM] = QString("%1").arg(ToDouble(m_tInterval));

	return 0;
}

void RandomWaypointModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	SimUnconstrainedMobilityModel::GetParams(mapParams);

	mapParams[RANDOMWAYPOINTMODEL_START_PARAM].strValue = RANDOMWAYPOINTMODEL_START_PARAM_DEFAULT;
	mapParams[RANDOMWAYPOINTMODEL_START_PARAM].strDesc = RANDOMWAYPOINTMODEL_START_PARAM_DESC;
	mapParams[RANDOMWAYPOINTMODEL_START_PARAM].eType = ModelParameterTypeCoords;

	mapParams[RANDOMWAYPOINTMODEL_LOWSPEED_PARAM].strValue = RANDOMWAYPOINTMODEL_LOWSPEED_PARAM_DEFAULT;
	mapParams[RANDOMWAYPOINTMODEL_LOWSPEED_PARAM].strDesc = RANDOMWAYPOINTMODEL_LOWSPEED_PARAM_DESC;
	mapParams[RANDOMWAYPOINTMODEL_LOWSPEED_PARAM].eType = ModelParameterTypeInt;

	mapParams[RANDOMWAYPOINTMODEL_HIGHSPEED_PARAM].strValue = RANDOMWAYPOINTMODEL_HIGHSPEED_PARAM_DEFAULT;
	mapParams[RANDOMWAYPOINTMODEL_HIGHSPEED_PARAM].strDesc = RANDOMWAYPOINTMODEL_HIGHSPEED_PARAM_DESC;
	mapParams[RANDOMWAYPOINTMODEL_HIGHSPEED_PARAM].eType = ModelParameterTypeInt;

	mapParams[RANDOMWAYPOINTMODEL_INTERVAL_PARAM].strValue = RANDOMWAYPOINTMODEL_INTERVAL_PARAM_DEFAULT;
	mapParams[RANDOMWAYPOINTMODEL_INTERVAL_PARAM].strDesc = RANDOMWAYPOINTMODEL_INTERVAL_PARAM_DESC;
	mapParams[RANDOMWAYPOINTMODEL_INTERVAL_PARAM].eType = ModelParameterTypeFloat;
}

bool RandomWaypointModel::GetInitialConditions(Coords & ptPosition, short & iSpeed, short & iHeading)
{
	// get initial position from trip model
	ptPosition = m_ptStart;
	iHeading = ChooseDirection(0);
	iSpeed = ChooseSpeed(0);
	return ptPosition.m_iLat != 0 || ptPosition.m_iLong != 0;
}

int RandomWaypointModel::PreRun()
{
	if (SimUnconstrainedMobilityModel::PreRun())
		return 1;

	m_fProgress = 0.f;

	return 0;
}

bool RandomWaypointModel::DoIteration(float fElapsed, Coords & ptPosition, short & iSpeed, short & iHeading)
{
	bool bMore = false;

	do
	{
		bMore = SetProgress(fElapsed, ptPosition, iSpeed, iHeading);
		if (bMore)
		{
			iSpeed = ChooseSpeed(iSpeed);
			iHeading = ChooseDirection(iHeading);
		}
	} while (bMore);

	return true;
}

bool RandomWaypointModel::SetProgress(float & fTime, Coords & ptPosition, short iSpeed, short iHeading)
{
	// find distance to next shape point
	float fRemaining = (1 - m_fProgress) * ToFloat(m_tInterval), fDistance = (fTime * iSpeed) / SECSPERHOUR, fHeading = iHeading * RADIANSPERCENTIDEGREE;

	if (fTime < fRemaining)
	{
		// we're still on the same "segment"
		ptPosition.m_iLat += (int)round(normangle((fDistance * cosf(fHeading)) / EARTHRADIUS) * TIGERDEGREESPERRADIAN);
		if (abs(ptPosition.m_iLat) < 90000000)
			ptPosition.m_iLong += (int)round(normangle((fDistance * sinf(fHeading)) / (EARTHRADIUS * cosf(ptPosition.m_iLat * RADIANSPERTIGERDEGREE))) * TIGERDEGREESPERRADIAN);
		
		m_fProgress += fTime / ToFloat(m_tInterval);
		fTime = 0.f;
		return false;
	}
	else
	{
		// we've passed the end of this "segment"
		fDistance = (fRemaining * iSpeed) / SECSPERHOUR;
		ptPosition.m_iLat += (int)round(normangle((fDistance * cosf(fHeading)) / EARTHRADIUS) * TIGERDEGREESPERRADIAN);
		if (abs(ptPosition.m_iLat) < 90000000)
			ptPosition.m_iLong += (int)round(normangle((fDistance * sinf(fHeading)) / (EARTHRADIUS * cosf(ptPosition.m_iLat * RADIANSPERTIGERDEGREE))) * TIGERDEGREESPERRADIAN);
		
		m_fProgress = 0.f;
		fTime -= fRemaining;
		return true;
	}
}

short RandomWaypointModel::ChooseSpeed(short iSpeed) const
{
	return (short)RandInt(m_iLowSpeed, m_iHighSpeed+1);
}

short RandomWaypointModel::ChooseDirection(short iHeading) const
{
	return (short)RandInt(-18000, 18000);
}

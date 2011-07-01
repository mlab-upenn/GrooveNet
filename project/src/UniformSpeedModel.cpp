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

#include "UniformSpeedModel.h"

#include "Simulator.h"
#include "MapDB.h"
#include "StringHelp.h"

#define UNIFORMSPEEDMODEL_LOW_PARAM "LOW"
#define UNIFORMSPEEDMODEL_LOW_PARAM_DEFAULT "0"
#define UNIFORMSPEEDMODEL_LOW_PARAM_DESC "LOW (mph or %) -- The lower bound relative to the road speed for the uniform distribution of the vehicle speed. If specified as a percentage, the lower bound deviation from the road speed is computed as a fraction of the road speed; otherwise, it is an absolute difference."
#define UNIFORMSPEEDMODEL_HIGH_PARAM "HIGH"
#define UNIFORMSPEEDMODEL_HIGH_PARAM_DEFAULT "0"
#define UNIFORMSPEEDMODEL_HIGH_PARAM_DESC "HIGH (mph or %) -- The upper bound relative to the road speed for the uniform distribution of the vehicle speed. If specified as a percentage, the upper bound deviation from the road speed is computed as a fraction of the road speed; otherwise, it is an absolute difference."

UniformSpeedModel::UniformSpeedModel(const QString & strModelName)
: StreetSpeedModel(strModelName)
{
}

UniformSpeedModel::UniformSpeedModel(const UniformSpeedModel & copy)
: StreetSpeedModel(copy), m_sLow(copy.m_sLow), m_sHigh(copy.m_sHigh)
{
}

UniformSpeedModel::~UniformSpeedModel()
{
}

UniformSpeedModel & UniformSpeedModel::operator = (const UniformSpeedModel & copy)
{
	StreetSpeedModel::operator = (copy);

	m_sLow = copy.m_sLow;
	m_sHigh = copy.m_sHigh;

	return *this;
}

short UniformSpeedModel::ChooseRandomSpeed(short iSpeed) const
{
	short iLow, iHigh;
	switch(m_sLow.eType)
	{
	case UniformSpeedRange::UniformSpeedRangeAbsolute:
		iLow = iSpeed + m_sLow.iAbsolute;
		break;
	case UniformSpeedRange::UniformSpeedRangeRelative:
		iLow = (short)round(iSpeed * (1 + m_sLow.fRelative));
		break;
	default:
		iLow = iSpeed;
		break;
	}
	switch(m_sHigh.eType)
	{
	case UniformSpeedRange::UniformSpeedRangeAbsolute:
		iHigh = iSpeed + m_sHigh.iAbsolute;
		break;
	case UniformSpeedRange::UniformSpeedRangeRelative:
		iHigh = (short)round(iSpeed * (1 + m_sHigh.fRelative));
		break;
	default:
		iHigh = iSpeed;
		break;
	}
	return (short)RandInt(iLow, iHigh + 1);
}

int UniformSpeedModel::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;

	if (StreetSpeedModel::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, UNIFORMSPEEDMODEL_LOW_PARAM, UNIFORMSPEEDMODEL_LOW_PARAM_DEFAULT);
	if (strValue.find('%') > -1) {
		m_sLow.fRelative = (float)StringToNumber(strValue);
		m_sLow.eType = UniformSpeedRange::UniformSpeedRangeRelative;
	} else {
		m_sLow.iAbsolute = (int)round(StringToNumber(strValue));
		m_sLow.eType = UniformSpeedRange::UniformSpeedRangeAbsolute;
	}
	strValue = GetParam(mapParams, UNIFORMSPEEDMODEL_HIGH_PARAM, UNIFORMSPEEDMODEL_HIGH_PARAM_DEFAULT);
	if (strValue.find('%') > -1) {
		m_sHigh.fRelative = (float)StringToNumber(strValue);
		m_sHigh.eType = UniformSpeedRange::UniformSpeedRangeRelative;
	} else {
		m_sHigh.iAbsolute = (int)round(StringToNumber(strValue));
		m_sHigh.eType = UniformSpeedRange::UniformSpeedRangeAbsolute;
	}
	return 0;
}

int UniformSpeedModel::Save(std::map<QString, QString> & mapParams)
{
	if (StreetSpeedModel::Save(mapParams))
		return 1;

	switch (m_sLow.eType)
	{
	case UniformSpeedRange::UniformSpeedRangeRelative:
		mapParams[UNIFORMSPEEDMODEL_LOW_PARAM] = QString("%1").arg(m_sLow.fRelative*100) + '%';
		break;
	case UniformSpeedRange::UniformSpeedRangeAbsolute:
		mapParams[UNIFORMSPEEDMODEL_LOW_PARAM] = QString("%1").arg(m_sLow.iAbsolute);
		break;
	default:
		break;
	}

	switch (m_sHigh.eType)
	{
	case UniformSpeedRange::UniformSpeedRangeRelative:
		mapParams[UNIFORMSPEEDMODEL_HIGH_PARAM] = QString("%1").arg(m_sHigh.fRelative*100) + '%';
		break;
	case UniformSpeedRange::UniformSpeedRangeAbsolute:
		mapParams[UNIFORMSPEEDMODEL_HIGH_PARAM] = QString("%1").arg(m_sHigh.iAbsolute);
		break;
	default:
		break;
	}

	return 0;
}

void UniformSpeedModel::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	StreetSpeedModel::GetParams(mapParams);

	mapParams[UNIFORMSPEEDMODEL_LOW_PARAM].strValue = UNIFORMSPEEDMODEL_LOW_PARAM_DEFAULT;
	mapParams[UNIFORMSPEEDMODEL_LOW_PARAM].strDesc = UNIFORMSPEEDMODEL_LOW_PARAM_DESC;
	mapParams[UNIFORMSPEEDMODEL_LOW_PARAM].eType = ModelParameterTypeFloat;

	mapParams[UNIFORMSPEEDMODEL_HIGH_PARAM].strValue = UNIFORMSPEEDMODEL_HIGH_PARAM_DEFAULT;
	mapParams[UNIFORMSPEEDMODEL_HIGH_PARAM].strDesc = UNIFORMSPEEDMODEL_HIGH_PARAM_DESC;
	mapParams[UNIFORMSPEEDMODEL_HIGH_PARAM].eType = ModelParameterTypeFloat;
}

short UniformSpeedModel::ChooseSpeed(unsigned int iRecord) const
{
	if (iRecord == (unsigned)-1)
		return 0;
	else
		return ChooseRandomSpeed(StreetSpeedModel::ChooseSpeed(iRecord));
}

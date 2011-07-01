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

#include "Model.h"
#include "StringHelp.h"

//#define MODEL_PARAM_DELAY "DELAY"
//#define MODEL_PARAM_DELAY_DEFAULT "0.2"

Model::Model(const QString & strModelName)
: /*m_tDelay(timeval0), */m_strModelName(strModelName), m_tLastEvent(timeval0)
{
}

Model::Model(const Model & copy)
: /*m_tDelay(copy.m_tDelay), */m_strModelName(copy.m_strModelName), m_tLastEvent(copy.m_tLastEvent)
{
}

Model::~ Model()
{
}

Model & Model::operator =(const Model & copy)
{
//	m_tDelay = copy.m_tDelay;

	m_strModelName = copy.m_strModelName;
	m_tLastEvent = copy.m_tLastEvent;
	return *this;
}

int Model::Init(const std::map<QString, QString> & mapParams)
{
//	QString strValue;

//	strValue = GetParam(mapParams, MODEL_PARAM_DELAY, MODEL_PARAM_DELAY_DEFAULT);
//	m_tDelay = MakeTime(ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL));

	return 0; // successful
}

int Model::Save(std::map<QString, QString> & mapParams)
{
//	mapParams[MODEL_PARAM_DELAY] = QString("%1").arg(ToDouble(m_tDelay));

	return 0; // successful
}

void Model::GetParams(std::map<QString, ModelParameter> & mapParams)
{
//	mapParams[MODEL_PARAM_DELAY].strValue = MODEL_PARAM_DELAY_DEFAULT;
//	mapParams[MODEL_PARAM_DELAY].eType = (ModelParameterType)(ModelParameterTypeFloat | ModelParameterFixed);
//	mapParams[MODEL_PARAM_DELAY].strAuxData = QString("%1:").arg(5e-2);
}





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

#include <qapplication.h>

#include "Visualizer.h"
#include "MainWindow.h"
#include "QVisualizer.h"
#include "StringHelp.h"
#include "Simulator.h"

#define VISUALIZER_PARAM_DELAY "DELAY"
#define VISUALIZER_PARAM_DELAY_DEFAULT "0.2"
#define VISUALIZER_PARAM_DELAY_DESC "DELAY (seconds) -- The time interval for updating this display window."

Visualizer::Visualizer(const QString & strModelName)
: Model(strModelName), m_pWidget(NULL), m_tDelay(MakeTime(0.2))
{
}

Visualizer::Visualizer(const Visualizer & copy)
: Model(copy), m_pWidget(copy.m_pWidget), m_tDelay(copy.m_tDelay)
{
}

Visualizer::~Visualizer()
{
}

Visualizer & Visualizer::operator =(const Visualizer & copy)
{
	Model::operator =(copy);

	m_pWidget = copy.m_pWidget;
	m_tDelay = copy.m_tDelay;
	return *this;
}

int Visualizer::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;
	int iWidth, iHeight;

	if (Model::Init(mapParams))
		return 1;

	strValue = GetParam(mapParams, VISUALIZER_PARAM_DELAY, VISUALIZER_PARAM_DELAY_DEFAULT);
	m_tDelay = MakeTime(ValidateNumber(StringToNumber(strValue), 0., HUGE_VAL));

	// create associated widget
	m_pWidget = CreateWidget();
	if (m_pWidget == NULL)
		return 2;
	strValue = GetParam(mapParams, VISUALIZER_PARAM_CAPTION, VISUALIZER_PARAM_CAPTION_DEFAULT);
	m_pWidget->setCaption(strValue);
	strValue = GetParam(mapParams, VISUALIZER_PARAM_WIDTH, VISUALIZER_PARAM_WIDTH_DEFAULT);
	iWidth = (int)ValidateNumber(round(StringToNumber(strValue)), QApplication::globalStrut().width(), HUGE_VAL);
	strValue = GetParam(mapParams, VISUALIZER_PARAM_HEIGHT, VISUALIZER_PARAM_HEIGHT_DEFAULT);
	iHeight = (int)ValidateNumber(round(StringToNumber(strValue)), QApplication::globalStrut().height(), HUGE_VAL);
	m_pWidget->resize(iWidth, iHeight); // set initial size

	m_pWidget->show();

	return 0;
}

int Visualizer::PreRun()
{
	if (Model::PreRun())
		return 1;

	// push initial event
	g_pSimulator->m_EventQueue.AddEvent(SimEvent(g_pSimulator->m_tCurrent, EVENT_PRIORITY_UPDATE, m_strModelName, m_strModelName, EVENT_VISUALIZER_UPDATE));

	return 0;
}

int Visualizer::ProcessEvent(SimEvent & event)
{
	if (Model::ProcessEvent(event))
		return 1;

	switch (event.GetEventID())
	{
	case EVENT_VISUALIZER_UPDATE:
		// push initial event
		event.SetTimestamp(g_pSimulator->m_tCurrent + m_tDelay);
		g_pSimulator->m_EventQueue.AddEvent(event);
		break;
	default:
		break;
	}
	return 0;
}

int Visualizer::PostRun()
{
	if (Model::PostRun())
		return 1;

	if (m_pWidget)
		m_pWidget->update();

	return 0;
}

int Visualizer::Save(std::map<QString, QString> & mapParams)
{
	if (Model::Save(mapParams))
		return 1;

	mapParams[VISUALIZER_PARAM_DELAY] = QString("%1").arg(ToDouble(m_tDelay));
	if (m_pWidget != NULL)
	{
		mapParams[VISUALIZER_PARAM_CAPTION] = m_pWidget->caption();
		mapParams[VISUALIZER_PARAM_WIDTH] = QString("%1").arg(m_pWidget->width());
		mapParams[VISUALIZER_PARAM_HEIGHT] = QString("%1").arg(m_pWidget->height());
	}

	return 0;
}

void Visualizer::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	Model::GetParams(mapParams);

	mapParams[VISUALIZER_PARAM_DELAY].strValue = VISUALIZER_PARAM_DELAY_DEFAULT;
	mapParams[VISUALIZER_PARAM_DELAY].strDesc = VISUALIZER_PARAM_DELAY_DESC;
	mapParams[VISUALIZER_PARAM_DELAY].eType = (ModelParameterType)(ModelParameterTypeFloat | ModelParameterFixed);
	mapParams[VISUALIZER_PARAM_DELAY].strAuxData = QString("%1:").arg(5e-2);

	mapParams[VISUALIZER_PARAM_CAPTION].strValue = VISUALIZER_PARAM_CAPTION_DEFAULT;
	mapParams[VISUALIZER_PARAM_CAPTION].strDesc = VISUALIZER_PARAM_CAPTION_DESC;
	mapParams[VISUALIZER_PARAM_CAPTION].eType = (ModelParameterType)(ModelParameterTypeText | ModelParameterFixed);

	mapParams[VISUALIZER_PARAM_WIDTH].strValue = VISUALIZER_PARAM_WIDTH_DEFAULT;
	mapParams[VISUALIZER_PARAM_WIDTH].strDesc = VISUALIZER_PARAM_WIDTH_DESC;
	mapParams[VISUALIZER_PARAM_WIDTH].eType = (ModelParameterType)(ModelParameterTypeInt | ModelParameterFixed);
	mapParams[VISUALIZER_PARAM_WIDTH].strAuxData = QString("%1:").arg(QApplication::globalStrut().width());

	mapParams[VISUALIZER_PARAM_HEIGHT].strValue = VISUALIZER_PARAM_HEIGHT_DEFAULT;
	mapParams[VISUALIZER_PARAM_HEIGHT].strDesc = VISUALIZER_PARAM_HEIGHT_DESC;
	mapParams[VISUALIZER_PARAM_HEIGHT].eType = (ModelParameterType)(ModelParameterTypeInt | ModelParameterFixed);
	mapParams[VISUALIZER_PARAM_HEIGHT].strAuxData = QString("%1:").arg(QApplication::globalStrut().height());
}

int Visualizer::Cleanup()
{
	if (Model::Cleanup())
		return 1;

	if (m_pWidget != NULL)
	{
		m_pWidget->close(true);
		m_pWidget = NULL;
	}
	return 0;
}

QWidget * Visualizer::CreateWidget()
{
	return new QVisualizer(this, g_pMainWindow->centralWidget(), m_strModelName, Qt::WNoAutoErase | Qt::WDestructiveClose);
}

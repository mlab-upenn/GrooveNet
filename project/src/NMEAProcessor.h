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

#ifndef _NMEAPROCESSOR_H
#define _NMEAPROCESSOR_H

#include "GPSModel.h"

#define NMEAPROCESSOR_NAME "NMEA"

class NMEAProcessor : public GPSProcessor
{
public:
	inline virtual QString GetGPSProcessorType() const
	{
		return NMEAPROCESSOR_NAME;
	}

	NMEAProcessor();
	virtual ~NMEAProcessor();

	virtual bool Init(const std::map<QString, QString> & mapParams);
	virtual void Save(std::map<QString, QString> & mapParams);
	static void GetParams(std::map<QString, ModelParameter> & mapParams);

	bool Open(const QString & strFilename, int iBaudRate, int iDataBits, int iStopBits, int iParity);
	void Close();

protected:
	virtual void run();

	int m_iFD;

	QString m_strFilename;
	int m_iBaudRate, m_iDataBits, m_iStopBits, m_iParity;
};

#define NMEAPROCESSOR_FILENAME_PARAM "FILENAME"
#define NMEAPROCESSOR_FILENAME_PARAM_DEFAULT "/dev/ttyS0"
#define NMEAPROCESSOR_FILENAME_PARAM_DESC "FILENAME (string) -- The path to the terminal I/O file to use for communication with the GPS device."
#define NMEAPROCESSOR_BAUDRATE_PARAM "BAUDRATE"
#define NMEAPROCESSOR_BAUDRATE_PARAM_DEFAULT "13"
#define NMEAPROCESSOR_BAUDRATE_PARAM_DESC "BAUDRATE (integer) -- The baud rate setting. Specific values are: 0=0; 1=50; 2=75; 3=110; 4=134; 5=150; 6=200; 7=300; 8=600; 9=1,200; 10=2,400; 11=4,800; 12=9,600; 13=19,200; 14=38,400; 15=57,600; 16=115,200; 17=230,400; 18=460,800; 19=500,000; 20=576,000; 21=921,600; 22=1,000,000; 23=1,152,000; 24=1,500,000; 25=2,000,000; 26=2,500,000; 27=3,000,000; 28=3,500,000; 29=4,000,000."
#define NMEAPROCESSOR_DATABITS_PARAM "DATABITS"
#define NMEAPROCESSOR_DATABITS_PARAM_DEFAULT "8"
#define NMEAPROCESSOR_DATABITS_PARAM_DESC "DATABITS (integer) -- The number of data bits per serial frame. Can be either 5, 6, 7 or 8."
#define NMEAPROCESSOR_STOPBITS_PARAM "STOPBITS"
#define NMEAPROCESSOR_STOPBITS_PARAM_DEFAULT "1"
#define NMEAPROCESSOR_STOPBITS_PARAM_DESC "STOPBITS (integer) -- The number of stop bits per serial frame. Can either be 1 or 2."
#define NMEAPROCESSOR_PARITY_PARAM "PARITY"
#define NMEAPROCESSOR_PARITY_PARAM_DEFAULT "0"
#define NMEAPROCESSOR_PARITY_PARAM_DESC "PARITY (integer) -- The parity bit setting for this serial connection. Specific values are: 0=no parity; 1=odd parity; 2=even parity."

#endif


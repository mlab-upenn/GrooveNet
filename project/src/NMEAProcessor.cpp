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

#include "NMEAProcessor.h"
#include "StringHelp.h"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define NMEAPROCESSOR_MSG_DELIMITER "\r\n"
#define NMEAPROCESSOR_DELIMITER ','

typedef enum NMEAMessageTypeEnum
{
	NMEAMessageTypeUnknown = 0,
	NMEAMessageTypeGPGGA,
	NMEAMessageTypeGPGLL,
	NMEAMessageTypeGPGSA,
	NMEAMessageTypeGPGST,
	NMEAMessageTypeGPGSV,
	NMEAMessageTypeGPRMC,
	NMEAMessageTypeGPRRE,
	NMEAMessageTypeGPVTG,
	NMEAMessageTypeGPZDA,
} NMEAMessageType;

static const QString g_strNMEAMessageTypeNames[9] = {"$GPGGA", "$GPGLL", "$GPGSA", "$GPGST", "$GPGSV", "$GPRMC", "$GPRRE", "$GPVTG", "$GPZDA"};

static const unsigned int g_iNMEAMessageTypeArgs[10] = {0, 14, 6, 17, 8, 19, 11, 27, 8, 6};

typedef struct GPGGAMessageStruct {
	double fTimestamp; // hhmmss.ss
	long iLatitude; // TIGER
	long iLongitude; // TIGER
	double fAltitude; // meters
	unsigned char iQuality; // 0 = none, 1 = no differential correction, 2 = differential correction, 9 = almanac
	unsigned char nSatellites;
	float fHDOP; // 0.0 -> 9.9
	float fGeoidalSep; // meters
	int iDiffCorrectionAge; // seconds
	int iRefID;
} GPGGAMessage;

typedef struct GPGLLMessageStruct {
	long iLatitude; // TIGER
	long iLongitude; // TIGER
	double fTimestamp; // hhmmss.ss
	char iValid; // A = valid, V = invalid
} GPGLLMessage;

typedef struct GPGSAMessageStruct {
	char iAcquisitionMode; // M = manual 2D or 3D, A = automatic swap
	unsigned char iPositionMode; // 1 = N/A, 2 = 2D fix, 3 = 3D fix
	unsigned char pSatellites[12];
	float fPDOP; // 0.0 -> 9.9
	float fHDOP; // 0.0 -> 9.9
	float fVDOP; // 0.0 -> 9.9
} GPGSAMessage;

typedef struct GPGSTMessageStruct {
	double fTimestamp; // hhmmss.ss
	float fRMSStdDev;
	float fStdDevSemiMajor; // meters
	float fStdDevSemiMinor; // meters
	float fOrientSemiMajor; // meters
	float fStdDevLat; // meters
	float fStdDevLong; // meters
	float fStdDevAlt; // meters
} GPGSTMessage;

typedef struct GPGSVMessageStruct {
	unsigned char nMessages; // 1-3
	unsigned char iMsg; // 1-3
	unsigned char nSatellites;
	unsigned char pSatellites[4];
	unsigned char pElevations[4]; // degrees (0 to 90)
	int pAzimuths[4]; // degrees (0 to 359)
	unsigned char pSNRs[4]; // (dB) + 30, from 0 to 99
} GPGSVMessage;

typedef struct GPRMCMessageStruct {
	double fTimestamp; // hhmmss.ss
	char iValid; // A = valid, V = invalid
	long iLatitude; // TIGER
	long iLongitude; // TIGER
	float fSpeed; // knots
	float fHeading; // degrees, rel. to true north
	long iDate; // ddmmyy
	float fMagVariation; // degrees
	char iVariationSense; // E = East, W = West
} GPRMCMessage;

typedef struct GPRREMessageStruct {
	unsigned char nSatellites;
	unsigned char pSatellites[12];
	int pRangeResiduals[12]; // meters
	float fHorizontalError; // meters
	float fVerticalError; // meters
} GPRREMessage;

typedef struct GPVTGMessageStruct {
	float fTrueCourse; // centidegrees (0 to 35999)
	float fMagCourse; // centidegrees (0 to 35999)
	float fSpeedKnots; // knots (0 to 999)
	float fSpeedKMH; // km/h (0 to 999)
} GPVTGMessage;

typedef struct GPZDAMessageStruct {
	double fTimestamp; // hhmmss.ss
	unsigned char iDay; // 0 to 31
	unsigned char iMonth; // 1 to 12
	int iYear; // yyyy
	char iLocalHours; // -13 to 13
	unsigned char iLocalMinutes; // 0 to 59
} GPZDAMessage;

typedef struct NMEAMessageStruct {
	NMEAMessageType eType;
	union { // 69 bytes!
		GPGGAMessage sGPGGA;
		GPGLLMessage sGPGLL;
		GPGSAMessage sGPGSA;
		GPGSTMessage sGPGST;
		GPGSVMessage sGPGSV;
		GPRMCMessage sGPRMC;
		GPRREMessage sGPRRE;
		GPVTGMessage sGPVTG;
		GPZDAMessage sGPZDA;
	};
} NMEAMessage;

NMEAMessageType GetNMEAMessageType(const QString & msgType)
{
	int i;

	for (i = 0; i < 9; i++)
	{
		if (msgType.compare(g_strNMEAMessageTypeNames[i]) == 0)
			return (NMEAMessageType)(i + 1);
	}
	return NMEAMessageTypeUnknown;
}

long NMEAToTIGER(double coord)
{
	return (long)(coord * 1e-2) * 1000000 + (long)(fmod(coord, 100) * 16666.6666667);
}

struct timeval NMEAToTime(double hhmmss, int day, int month, int year)
{
	double hhmmss_int, hhmmss_frac;
	time_t tTime;
	struct tm sTime;

	hhmmss_frac = modf(hhmmss, &hhmmss_int);
	sTime.tm_hour = (int)hhmmss_int / 10000;
	sTime.tm_min = ((int)hhmmss_int / 100) % 100;
	sTime.tm_sec = (int)hhmmss_int % 100;

	sTime.tm_mday = day; // ordinal value!
	sTime.tm_mon = month - 1; // zero-indexed!
	sTime.tm_year = year - 1900;

	tTime = mktime(&sTime);

	return MakeTime(tTime, (long int)(hhmmss_frac * 100000));
}

struct timeval NMEAToTime(double hhmmss)
{
	double hhmmss_int, hhmmss_frac;
	time_t tTime;
	struct tm sTime;

	hhmmss_frac = modf(hhmmss, &hhmmss_int);
	time(&tTime);
	gmtime_r(&tTime, &sTime);
	sTime.tm_hour = (int)hhmmss_int / 10000;
	sTime.tm_min = ((int)hhmmss_int / 100) % 100;
	sTime.tm_sec = (int)hhmmss_int % 100;

	tTime = mktime(&sTime);

	return MakeTime(tTime, (long int)(hhmmss_frac * 100000));
}

bool ParseNMEAMessage(const QString & strMsg, NMEAMessage * pMsg)
{
	QStringList listArgs = QStringList::split(NMEAPROCESSOR_DELIMITER, strMsg, true);
	NMEAMessageType eType;
	QStringList::iterator iterArg = listArgs.begin();
	unsigned int nArgs;
	int i;
	bool bValid = true;

	if (listArgs.size() == 0 || (eType = GetNMEAMessageType(*iterArg)) == NMEAMessageTypeUnknown)
		return false;

	nArgs = g_iNMEAMessageTypeArgs[eType];
	if (listArgs.size() < nArgs + 1) // not enough arguments - not valid!
		return false;

	// fill new message structure with parsed data
	pMsg->eType = eType;
	switch (eType) {
		case NMEAMessageTypeGPGGA:
			pMsg->sGPGGA.fTimestamp = (*(++iterArg)).toDouble();
			pMsg->sGPGGA.iLatitude = NMEAToTIGER((*(++iterArg)).toDouble());
			if ((*(++iterArg)).compare("S") == 0)
				pMsg->sGPGGA.iLatitude *= -1;
			pMsg->sGPGGA.iLongitude = NMEAToTIGER((*(++iterArg)).toDouble());
			if ((*(++iterArg)).compare("W") == 0)
				pMsg->sGPGGA.iLongitude *= -1;
			pMsg->sGPGGA.iQuality = (unsigned char)(*(++iterArg)).toUShort();
			pMsg->sGPGGA.nSatellites = (unsigned char)(*(++iterArg)).toUShort();
			pMsg->sGPGGA.fHDOP = (*(++iterArg)).toFloat();
			pMsg->sGPGGA.fAltitude = (*(++iterArg)).toDouble();
			++iterArg; // skip altitude units (always meters)
			pMsg->sGPGGA.fGeoidalSep = (*(++iterArg)).toFloat();
			++iterArg; // skip geoidal separation units (always meters)
			pMsg->sGPGGA.iDiffCorrectionAge = (*(++iterArg)).toInt();
			pMsg->sGPGGA.iRefID = (*(++iterArg)).toInt();
			break;
		case NMEAMessageTypeGPGLL:
			pMsg->sGPGLL.iLatitude = NMEAToTIGER((*(++iterArg)).toDouble());
			if ((*(++iterArg)).compare("S") == 0)
				pMsg->sGPGLL.iLatitude *= -1;
			pMsg->sGPGLL.iLongitude = NMEAToTIGER((*(++iterArg)).toDouble());
			if ((*(++iterArg)).compare("W") == 0)
				pMsg->sGPGLL.iLongitude *= -1;
			pMsg->sGPGLL.fTimestamp = (*(++iterArg)).toDouble();
			pMsg->sGPGLL.iValid = (QChar)(*(++iterArg))[0];
			break;
		case NMEAMessageTypeGPGSA:
			pMsg->sGPGSA.iAcquisitionMode = (QChar)(*(++iterArg))[0];
			pMsg->sGPGSA.iPositionMode = (unsigned char)(*(++iterArg)).toUShort();
			for (i = 0; i < 12; i++)
				pMsg->sGPGSA.pSatellites[i] = (unsigned char)(*(++iterArg)).toUShort();
			pMsg->sGPGSA.fPDOP = (*(++iterArg)).toFloat();
			pMsg->sGPGSA.fHDOP = (*(++iterArg)).toFloat();
			pMsg->sGPGSA.fVDOP = (*(++iterArg)).toFloat();
			break;
		case NMEAMessageTypeGPGST:
			pMsg->sGPGST.fTimestamp = (*(++iterArg)).toDouble();
			pMsg->sGPGST.fRMSStdDev = (*(++iterArg)).toFloat();
			pMsg->sGPGST.fStdDevSemiMajor = (*(++iterArg)).toFloat();
			pMsg->sGPGST.fStdDevSemiMinor = (*(++iterArg)).toFloat();
			pMsg->sGPGST.fOrientSemiMajor = (*(++iterArg)).toFloat();
			pMsg->sGPGST.fStdDevLat = (*(++iterArg)).toFloat();
			pMsg->sGPGST.fStdDevLong = (*(++iterArg)).toFloat();
			pMsg->sGPGST.fStdDevAlt = (*(++iterArg)).toFloat();
			break;
		case NMEAMessageTypeGPGSV:
			pMsg->sGPGSV.nMessages = (unsigned char)(*(++iterArg)).toUShort();
			pMsg->sGPGSV.iMsg = (unsigned char)(*(++iterArg)).toUShort();
			pMsg->sGPGSV.nSatellites = (unsigned char)(*(++iterArg)).toUShort();
			for (i = 0; i < 4; i++) {
				pMsg->sGPGSV.pSatellites[i] = (unsigned char)(*(++iterArg)).toUShort();
				pMsg->sGPGSV.pElevations[i] = (unsigned char)(*(++iterArg)).toUShort();
				pMsg->sGPGSV.pAzimuths[i] = (unsigned char)(*(++iterArg)).toInt();
				pMsg->sGPGSV.pSNRs[i] = (unsigned char)(*(++iterArg)).toUShort();
			}
			break;
		case NMEAMessageTypeGPRMC:
			pMsg->sGPRMC.fTimestamp = (*(++iterArg)).toDouble();
			pMsg->sGPRMC.iValid = (QChar)(*(++iterArg))[0];
			pMsg->sGPRMC.iLatitude = NMEAToTIGER((*(++iterArg)).toDouble());
			if ((*(++iterArg)).compare("S") == 0)
				pMsg->sGPRMC.iLatitude *= -1;
			pMsg->sGPRMC.iLongitude = NMEAToTIGER((*(++iterArg)).toDouble());
			if ((*(++iterArg)).compare("W") == 0)
				pMsg->sGPRMC.iLongitude *= -1;
			pMsg->sGPRMC.fSpeed = (*(++iterArg)).toFloat();
			pMsg->sGPRMC.fHeading = (*(++iterArg)).toFloat();
			pMsg->sGPRMC.iDate = (*(++iterArg)).toLong();
			pMsg->sGPRMC.fMagVariation = (*(++iterArg)).toFloat();
			pMsg->sGPRMC.iVariationSense = (QChar)(*(++iterArg))[0];
			break;
		case NMEAMessageTypeGPRRE:
			pMsg->sGPRRE.nSatellites = (unsigned char)(*(++iterArg)).toUShort();
			for (i = 0; i < 12; i++) {
				pMsg->sGPRRE.pSatellites[i] = (unsigned char)(*(++iterArg)).toUShort();
				pMsg->sGPRRE.pRangeResiduals[i] = (*(++iterArg)).toInt();
			}
			pMsg->sGPRRE.fHorizontalError = (*(++iterArg)).toFloat();
			pMsg->sGPRRE.fVerticalError = (*(++iterArg)).toFloat();
			break;
		case NMEAMessageTypeGPVTG:
			pMsg->sGPVTG.fTrueCourse = (*(++iterArg)).toFloat();
			++iterArg; // skip over c because it always is T
			pMsg->sGPVTG.fMagCourse = (*(++iterArg)).toFloat();
			++iterArg; // skip over c because it always is M
			pMsg->sGPVTG.fSpeedKnots = (*(++iterArg)).toFloat();
			++iterArg; // skip over u because it always is N
			pMsg->sGPVTG.fSpeedKMH = (*(++iterArg)).toFloat();
			++iterArg; // skip over u because it always is K
			break;
		case NMEAMessageTypeGPZDA:
			pMsg->sGPZDA.fTimestamp = (*(++iterArg)).toDouble();
			pMsg->sGPZDA.iDay = (unsigned char)(*(++iterArg)).toUShort();
			pMsg->sGPZDA.iMonth = (unsigned char)(*(++iterArg)).toUShort();
			pMsg->sGPZDA.iYear = (*(++iterArg)).toInt();
			pMsg->sGPZDA.iLocalHours = (char)(*(++iterArg)).toShort();
			pMsg->sGPZDA.iLocalMinutes = (unsigned char)(*(++iterArg)).toUShort();
			break;
		default:
			break;
	}

	switch (eType) {
		case NMEAMessageTypeGPGGA:
			if (pMsg->sGPGGA.iQuality == 0) bValid = false;
			break;
		case NMEAMessageTypeGPGLL:
			if (toupper(pMsg->sGPGLL.iValid) != 'A') bValid = false;
			break;
		case NMEAMessageTypeGPGSA:
			if (pMsg->sGPGSA.iPositionMode == 1) bValid = false;
			break;
		case NMEAMessageTypeGPGST:
			break;
		case NMEAMessageTypeGPGSV:
			break;
		case NMEAMessageTypeGPRMC:
			if (toupper(pMsg->sGPRMC.iValid) != 'A') bValid = false;
			break;
		case NMEAMessageTypeGPRRE:
			break;
		case NMEAMessageTypeGPVTG:
			break;
		case NMEAMessageTypeGPZDA:
			break;
		default:
			bValid = false;
			break;
	}
	return bValid;
}

void ParseNMEAMessages(const QString & strText, std::vector<NMEAMessage> & vecMessages)
{
	QStringList listMessages = QStringList::split(NMEAPROCESSOR_MSG_DELIMITER, strText, false);
	QStringList::iterator iter;
	NMEAMessage sMsg;

	for (iter = listMessages.begin(); iter != listMessages.end(); ++iter)
	{
		if (ParseNMEAMessage(*iter, &sMsg))
			vecMessages.push_back(sMsg);
	}
}


NMEAProcessor::NMEAProcessor()
: GPSProcessor(), m_iFD(-1), m_strFilename(NMEAPROCESSOR_FILENAME_PARAM_DEFAULT), m_iBaudRate(13), m_iDataBits(8), m_iStopBits(0), m_iParity(0)
{
}

NMEAProcessor::~NMEAProcessor()
{
	Close();
}

bool NMEAProcessor::Init(const std::map<QString, QString> & mapParams)
{
	QString strValue;

	m_strFilename = GetParam(mapParams, NMEAPROCESSOR_FILENAME_PARAM, NMEAPROCESSOR_FILENAME_PARAM_DEFAULT);

	strValue = GetParam(mapParams, NMEAPROCESSOR_BAUDRATE_PARAM, NMEAPROCESSOR_BAUDRATE_PARAM_DEFAULT);
	m_iBaudRate = (int)StringToNumber(strValue);

	strValue = GetParam(mapParams, NMEAPROCESSOR_DATABITS_PARAM, NMEAPROCESSOR_DATABITS_PARAM_DEFAULT);
	m_iDataBits = (int)StringToNumber(strValue);

	strValue = GetParam(mapParams, NMEAPROCESSOR_STOPBITS_PARAM, NMEAPROCESSOR_STOPBITS_PARAM_DEFAULT);
	m_iStopBits = (int)StringToNumber(strValue);

	strValue = GetParam(mapParams, NMEAPROCESSOR_PARITY_PARAM, NMEAPROCESSOR_PARITY_PARAM_DEFAULT);
	m_iParity = (int)StringToNumber(strValue);

	return Open(m_strFilename, m_iBaudRate, m_iDataBits, m_iStopBits, m_iParity);
}

void NMEAProcessor::Save(std::map<QString, QString> & mapParams)
{
	mapParams[NMEAPROCESSOR_FILENAME_PARAM] = m_strFilename;
	mapParams[NMEAPROCESSOR_BAUDRATE_PARAM] = QString("%1").arg(m_iBaudRate);
	mapParams[NMEAPROCESSOR_DATABITS_PARAM] = QString("%1").arg(m_iDataBits);
	mapParams[NMEAPROCESSOR_STOPBITS_PARAM] = QString("%1").arg(m_iStopBits);
	mapParams[NMEAPROCESSOR_PARITY_PARAM] = QString("%1").arg(m_iParity);
}

void NMEAProcessor::GetParams(std::map<QString, ModelParameter> & mapParams)
{
	GPSProcessor::GetParams(mapParams);

	mapParams[NMEAPROCESSOR_FILENAME_PARAM].strValue = NMEAPROCESSOR_FILENAME_PARAM_DEFAULT;
	mapParams[NMEAPROCESSOR_FILENAME_PARAM].strDesc = NMEAPROCESSOR_FILENAME_PARAM_DESC;
	mapParams[NMEAPROCESSOR_FILENAME_PARAM].eType = (ModelParameterType)(ModelParameterTypeFile | ModelParameterFixed);

	mapParams[NMEAPROCESSOR_BAUDRATE_PARAM].strValue = NMEAPROCESSOR_BAUDRATE_PARAM_DEFAULT;
	mapParams[NMEAPROCESSOR_BAUDRATE_PARAM].strDesc = NMEAPROCESSOR_BAUDRATE_PARAM_DESC;
	mapParams[NMEAPROCESSOR_BAUDRATE_PARAM].eType = (ModelParameterType)(ModelParameterTypeInt | ModelParameterFixed);

	mapParams[NMEAPROCESSOR_DATABITS_PARAM].strValue = NMEAPROCESSOR_DATABITS_PARAM_DEFAULT;
	mapParams[NMEAPROCESSOR_DATABITS_PARAM].strDesc = NMEAPROCESSOR_DATABITS_PARAM_DESC;
	mapParams[NMEAPROCESSOR_DATABITS_PARAM].eType = (ModelParameterType)(ModelParameterTypeInt | ModelParameterFixed);

	mapParams[NMEAPROCESSOR_STOPBITS_PARAM].strValue = NMEAPROCESSOR_STOPBITS_PARAM_DEFAULT;
	mapParams[NMEAPROCESSOR_STOPBITS_PARAM].strDesc = NMEAPROCESSOR_STOPBITS_PARAM_DESC;
	mapParams[NMEAPROCESSOR_STOPBITS_PARAM].eType = (ModelParameterType)(ModelParameterTypeInt | ModelParameterFixed);

	mapParams[NMEAPROCESSOR_PARITY_PARAM].strValue = NMEAPROCESSOR_PARITY_PARAM_DEFAULT;
	mapParams[NMEAPROCESSOR_PARITY_PARAM].strDesc = NMEAPROCESSOR_PARITY_PARAM_DESC;
	mapParams[NMEAPROCESSOR_PARITY_PARAM].eType = (ModelParameterType)(ModelParameterTypeInt | ModelParameterFixed);
}

bool NMEAProcessor::Open(const QString & strFilename, int iBaudRate, int iDataBits, int iStopBits, int iParity)
{
	struct termios sPTS;
	static const int BAUD_RATES[30] = {B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000};

	tcflag_t ioFlags;

	Close();

	m_iFD = ::open(strFilename, O_RDWR | O_NOCTTY);
	if (m_iFD < 0)
		return false; // open failed!

	// get default serial device attributes
	if (::tcgetattr(m_iFD, &sPTS) != 0) {
		Close();
		return false;
	}

	// set baud rate
	iBaudRate = (iBaudRate > -1 && iBaudRate < 30) ? BAUD_RATES[iBaudRate] : B0;
	if (iBaudRate == B0) {
		Close();
		return false;
	}
	::cfsetispeed(&sPTS, iBaudRate);
	::cfsetospeed(&sPTS, iBaudRate);

	// set flags for character size, stop bit(s), and parity
	ioFlags = sPTS.c_cflag & ~CSIZE & ~CSTOPB & ~PARENB & ~PARODD;
	switch(iDataBits) {
		case 5:
			ioFlags |= CS5;
			break;
		case 6:
			ioFlags |= CS6;
			break;
		case 7:
			ioFlags |= CS7;
			break;
		case 8:
			ioFlags |= CS8;
			break;
		default:
			Close();
			return false;
	}
	switch(iStopBits) {
		case 1:
			break; // use one stop bit (default)
		case 2:
			ioFlags |= CSTOPB; // use two stop bits instead of one
			break;
		default:
			Close();
			return false;
	}
	switch(iParity) {
		case 0:
			break;
		case 1:
			ioFlags |= PARODD;
		case 2:
			ioFlags |= PARENB;
			break;
		default:
			Close();
			return false;
	}
	sPTS.c_cflag = ioFlags;
	// following two lines make device non-blocking - from rtnapos library file by Andreas Kloeckner <ak@ixion.net>
	sPTS.c_cc[VTIME] = 0;
	sPTS.c_cc[VMIN] = 0;

	::tcflush(m_iFD, TCIFLUSH); // flush all input already from device - we don't want it!
	::tcsetattr(m_iFD, TCSANOW, &sPTS);
	::tcflush(m_iFD, TCOFLUSH); // flush all input already from device - we don't want it!
	::fcntl(m_iFD, F_SETFL, O_NONBLOCK); // might also make device non-blocking

	return true;
}

void NMEAProcessor::Close()
{
	if (m_iFD > -1) {
		::close(m_iFD);
		m_iFD = -1;
	}
}

#define NMEAPROCESSOR_WAIT_USECS 50000

void NMEAProcessor::run()
{
	QString strText;
	std::vector<NMEAMessage> vecMessages;
	unsigned int i;

	while (!m_bCancelled)
	{
		usleep(NMEAPROCESSOR_WAIT_USECS);

		if (m_iFD < 0)
			continue;

		// first read the data into a string
		if (!ReadNonBlocking(m_iFD, strText))
			continue;

		// if there's been data read, parse it
		ParseNMEAMessages(strText, vecMessages);

		if (vecMessages.empty())
			continue;

		if (!m_mutexData.tryLock())
			continue;

		// update parameters for each message
		for (i = 0; i < vecMessages.size(); i++) {
			switch(vecMessages[i].eType) { // aggregate data
				case NMEAMessageTypeGPGGA:
					m_sGPSData.ptPosition.Set(vecMessages[i].sGPGGA.iLongitude, vecMessages[i].sGPGGA.iLatitude);
					break;
				case NMEAMessageTypeGPGLL:
					m_sGPSData.ptPosition.Set(vecMessages[i].sGPGLL.iLongitude, vecMessages[i].sGPGLL.iLatitude);
					break;
				case NMEAMessageTypeGPGSA:
					break;
				case NMEAMessageTypeGPGST:
					break;
				case NMEAMessageTypeGPGSV:
					break;
				case NMEAMessageTypeGPRMC:
					m_sGPSData.ptPosition.Set(vecMessages[i].sGPRMC.iLongitude, vecMessages[i].sGPRMC.iLatitude);
					m_sGPSData.iSpeed = (int)(vecMessages[i].sGPRMC.fSpeed * KNOTSTOMPH);
					m_sGPSData.iHeading = (short)(vecMessages[i].sGPRMC.fHeading * 100);
					break;
				case NMEAMessageTypeGPRRE:
					break;
				case NMEAMessageTypeGPVTG:
					m_sGPSData.iHeading = (short)(vecMessages[i].sGPVTG.fTrueCourse * 100);
					m_sGPSData.iSpeed = (int)(vecMessages[i].sGPVTG.fSpeedKnots * KNOTSTOMPH);
					break;
				case NMEAMessageTypeGPZDA:
					m_sGPSData.tTimestamp = NMEAToTime(vecMessages[i].sGPZDA.fTimestamp, vecMessages[i].sGPZDA.iDay, vecMessages[i].sGPZDA.iMonth, vecMessages[i].sGPZDA.iYear);
					// TODO: synchronize computer clock to GPS time
					break;
				default:
					break;
			}
		}
		vecMessages.clear();

		m_mutexData.unlock();
	}
}

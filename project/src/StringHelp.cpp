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

#include "StringHelp.h"

#include <math.h>
#include <errno.h>

int GetHexDigit(const char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else
		return 0;
}

int GetDigit(const char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else
		return 0;
}

int GetOctDigit(const char c)
{
	if (c >= '0' && c <= '7')
		return c - '0';
	else
		return 0;
}

bool StringToBoolean(const QString & strYesNo)
{
	if (strcasecmp(strYesNo, "Y") == 0)
		return true;
	else if (strcasecmp(strYesNo, "YES") == 0)
		return true;
	else if (strcasecmp(strYesNo, "T") == 0)
		return true;
	else if (strcasecmp(strYesNo, "TRUE") == 0)
		return true;
	else
		return false;
}

double StringToNumber(const QString & strNumber)
{
	char * pBuffer = strdup(strNumber), * pRemainder;
	double ret = strtod(pBuffer, &pRemainder);
	switch (pRemainder[0])
	{
		case GIGA:
			ret *= 1e9;
			break;
		case MEGA:
			ret *= 1e6;
			break;
		case KILO:
			ret *= 1e3;
			break;
		case MILLI:
			ret *= 1e-3;
			break;
		case MICRO:
			ret *= 1e-6;
			break;
		case NANO:
			ret *= 1e-9;
			break;
		case PICO:
			ret *= 1e-12;
			break;
		case FEMTO:
			ret *= 1e-15;
			break;
		case '%':
			ret *= 1e-2;
			break;
		default:
			break;
	}
	free(pBuffer);
	return ret;
}

bool StringToIPAddress(const QString & strIP, in_addr_t & ipAddr)
{
	struct in_addr addr;
	if (strIP.isEmpty())
		return false;

	if (inet_aton((const char *)strIP, &addr)) {
		ipAddr = ntohl(addr.s_addr);
		return true;
	} else
		return false;
}

QString & ResolveEscapeSeqs(QString & strInput)
{
	int iEscape = 0;
	char cReplace;

	while ((iEscape = strInput.find(ESCAPE_MARK, iEscape + 1)) > -1 && iEscape < (signed)strInput.length() - 1)
	{
		switch (strInput[iEscape+1].latin1())
		{
			case SQUOTE_MARK:
				strInput.replace(iEscape, 2, '\'');
				break;
			case TAB_MARK:
				strInput.replace(iEscape, 2, '\t');
				break;
			case NEWLINE_MARK:
				strInput.replace(iEscape, 2, '\n');
				break;
			case CR_MARK:
				strInput.replace(iEscape, 2, '\r');
				break;
			case HEX_MARK:
				strInput.remove(iEscape, 2);
				if (iEscape < (signed)strInput.length() - 2)
				{
					cReplace = (char)((GetHexDigit(strInput[iEscape].latin1()) << 4) + GetHexDigit(strInput[iEscape+1].latin1()));
					strInput.replace(iEscape, 2, cReplace);
				}
				else
					strInput.truncate(iEscape); /* remove remainder of string */
				break;
			case OCT_MARK:
				strInput.remove(iEscape, 2);
				if (iEscape < (signed)strInput.length() - 2)
				{
					cReplace = (char)((GetOctDigit(strInput[iEscape].latin1()) << 6) + (GetOctDigit(strInput[iEscape+1].latin1()) << 3) + GetOctDigit(strInput[iEscape+2].latin1()));
					strInput.replace(iEscape, 3, cReplace);
				}
				else
					strInput.truncate(iEscape); /* remove remainder of string */
				break;
			default:
				/* just remove the escape character */
				strInput.remove(iEscape, 1);
				break;
		}
	}
	return strInput;
}

int ExtractParams(const QString & strLine, std::vector<std::pair<QString, QString> > & vecParams)
{
	int iParamDelimPtr, iEnd = 0, length, numAdded = 0;
	std::pair<QString, QString> param;

	length = strLine.length();
	while ((iParamDelimPtr = strLine.find(ASSIGN_MARK, iEnd)) > 0)
	{
		/* get the parameter -- easy part */
		param.first = strLine.mid(iEnd, iParamDelimPtr - iEnd).simplifyWhiteSpace();
		if (param.first.isNull()) param.first = "";

		/* get the value -- hard part (can be quoted) */
		iParamDelimPtr++;
		/* strip leading whitespace */
		while (iParamDelimPtr < length && strLine[iParamDelimPtr].isSpace()) iParamDelimPtr++;
		/* now we're at the first non-whitespace character
		 * if it's a quote, search for ending quote; otherwise just go to space
		 */
		if (iParamDelimPtr < length && strLine[iParamDelimPtr] == QUOTE_MARK)
		{
			iEnd = iParamDelimPtr;
			iParamDelimPtr++;
			do {
				iEnd = strLine.find(QUOTE_MARK, iEnd + 1);
			} while (iEnd > 0 && strLine[iEnd - 1] == ESCAPE_MARK);
		}
		else
		{
			for (iEnd = iParamDelimPtr; iEnd < length && !strLine[iEnd].isSpace(); iEnd++);
		}
		/* now iParamDelimPtr points to beginning of token and iEnd to the
		 * character after the end
		 */
		if (iEnd < iParamDelimPtr) iEnd = length;
		param.second = strLine.mid(iParamDelimPtr, iEnd - iParamDelimPtr).simplifyWhiteSpace();
		if (param.second.isNull()) param.second = "";
		if (iEnd < length && strLine[iEnd] == QUOTE_MARK) iEnd++; /* skip trailing quote */
		ResolveEscapeSeqs(param.second);

		vecParams.push_back(param);
		numAdded++;
	}
	return numAdded;
}

QString FormatTime(double fTime, unsigned int iPrecision)
{
	int iHours;
	int iMinutes;
	int iSeconds;
	double fSeconds;
	double fMSec;
	QString ret;

	fMSec = modf(fTime, &fSeconds);
	iSeconds = (int)fSeconds;
	iMinutes = (iSeconds / 60) % 60;
	iHours = iSeconds / 3600;
	iSeconds %= 60;

	if (iPrecision > 0)
		ret.sprintf("%d:%02d:%02d.%0*d", iHours, iMinutes, iSeconds, iPrecision, (int)(fMSec * pow(10, iPrecision)));
	else
		ret.sprintf("%d:%02d:%02d", iHours, iMinutes, iSeconds);
	return ret;
}

QString BooleanToString(bool bValue)
{
	return bValue ? "YES" : "NO";
}

QString IPAddressToString(in_addr_t ipAddr)
{
	char buffer[256];
	struct in_addr s;
	s.s_addr = htonl(ipAddr);
	return inet_ntop(AF_INET, (const void *)&s, buffer, 256);
;
}

QString DegreesToString(long iDegrees, unsigned int iPrecision)
{
	return QString("%1 \260").arg(iDegrees * 1e-6, 0, 'f', iPrecision);
}

#define BUFFER_LENGTH 65536
#define MAX_BLOCKING 0
// Patched version:

bool ReadNonBlocking(int iFD, QString & strRead)
{
        char pBuffer[BUFFER_LENGTH];
        char* bufPtr = pBuffer;
        fd_set fds;
        timeval waitMax;
        int nLength = 0;

        FD_ZERO(&fds);
        FD_SET(iFD, &fds);
        waitMax.tv_sec = 0;
        waitMax.tv_usec = MAX_BLOCKING;
        if (TEMP_FAILURE_RETRY(::select(FD_SETSIZE, &fds, NULL, NULL, &waitMax)) <= 0 || FD_ISSET(iFD, &fds) == 0)
                return false;

        char tempchar = 0x0;
        while(tempchar != '$')
        {
                //printf("waiting for begin: %c\n", tempchar);
                TEMP_FAILURE_RETRY(::read(iFD, &tempchar, 1));
        }
        pBuffer[0] = '$';
        tempchar = 0x0;
        int i=0;

        do {
                i++;
                //printf("waiting for end: %c\n", pBuffer[i]);
                while(::read(iFD, &(pBuffer[i]), 1) < 1);
        } while(pBuffer[i] != '\n');
        //printf("i = %d\n", i);
        //printf("%s\n", bufPtr);

        pBuffer[i] = '\n';
        nLength = i;

        if (nLength <= 0)
                return false;

        pBuffer[nLength] = '\0';
        strRead = bufPtr;
        return true;
}
/*
bool ReadNonBlocking(int iFD, QString & strRead)
{
	char pBuffer[BUFFER_LENGTH];
	fd_set fds;
	timeval waitMax;
	int nLength;

	FD_ZERO(&fds);
	FD_SET(iFD, &fds);
	waitMax.tv_sec = 0;
	waitMax.tv_usec = MAX_BLOCKING;
	if (TEMP_FAILURE_RETRY(::select(FD_SETSIZE, &fds, NULL, NULL, &waitMax)) <= 0 || FD_ISSET(iFD, &fds) == 0)
		return false;

	nLength = TEMP_FAILURE_RETRY(::read(iFD, pBuffer, BUFFER_LENGTH-1));
	if (nLength <= 0)
		return false;

	pBuffer[nLength] = '\0';
	strRead = pBuffer;
	return true;
}
*/
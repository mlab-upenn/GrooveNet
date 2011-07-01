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

#ifndef _STRINGHELP_H
#define _STRINGHELP_H

#include <qstring.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <vector>

#define ASSIGN_MARK '='
#define ESCAPE_MARK '\\'
#define QUOTE_MARK '\"'
#define SQUOTE_MARK '\''
#define TAB_MARK 't'
#define NEWLINE_MARK 'n'
#define CR_MARK 'r'
#define HEX_MARK 'x'
#define OCT_MARK 'o'

#define GIGA 'G'
#define MEGA 'M'
#define KILO 'K'
#define MILLI 'm'
#define MICRO 'u'
#define NANO 'n'
#define PICO 'p'
#define FEMTO 'f'

int GetHexDigit(const char c);
int GetDigit(const char c);
int GetOctDigit(const char c);

bool StringToBoolean(const QString & strYesNo);
double StringToNumber(const QString & strNumber);
bool StringToIPAddress(const QString & strIP, in_addr_t & ipAddr);

QString & ResolveEscapeSeqs(QString & strInput);

QString FormatTime(double fTime, unsigned int iPrecision);
QString BooleanToString(bool bValue);
QString IPAddressToString(in_addr_t ipAddr);
QString DegreesToString(long iDegrees, unsigned int iPrecision);

int ExtractParams(const QString & strLine, std::vector<std::pair<QString, QString> > & vecParams);

bool ReadNonBlocking(int iFD, QString & strRead);

#endif

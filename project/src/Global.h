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

#ifndef _GLOBAL_H
#define _GLOBAL_H

/* code-control definitions */
#define MULTILANETEST

/* version string */
#ifdef VERSION
#undef VERSION
#endif
#define VERSION "2.0.1"

/* package info */
#define PACKAGE_TITLE "GrooveNet"
#define PACKAGE "groovenet"
#define SHARE_DIRECTORY "../../mapdata"

/* some standard includes that we use everywhere */
#include <errno.h>
#include <math.h>
#include <qstring.h>
#include <sys/time.h>

#define SECSPERHOUR 3600.
#define SECSPERMINUTE 60.
#define MINUTESPERHOUR 60.
#define EARTHRADIUS 3959.
#define RADIANSPERDEGREE (M_PI / 180.)
#define DEGREESPERRADIAN (180. / M_PI)
#define CENTIDEGREESPERRADIAN (100*DEGREESPERRADIAN)
#define RADIANSPERCENTIDEGREE (RADIANSPERDEGREE / 100)
#define TIGERDEGREESPERRADIAN (1000000*DEGREESPERRADIAN)
#define RADIANSPERTIGERDEGREE (RADIANSPERDEGREE / 1000000)
#define KNOTSTOMPH 1.15077945
#define KILOMETERSPERMILE 1.609344
#define METERSPERMILE (KILOMETERSPERMILE * 1000)
#define MILESPERMETER (MILESPERKILOMETER / 1000)
#define MILESPERKILOMETER 0.621371192

#define DBGP
#ifdef DBGP
#define dbgprint(...) { printf(__VA_ARGS__); fflush(stdout); }
#else
#define dbgprint(...)
#endif

inline double ValidateNumber(const double value, const double min, const double max)
{
	if (value < min)
		return min;
	else if (value > max)
		return max;
	else
		return value;
}

inline double round(const double value)
{
	return floor(value + 0.5);
}

inline long lmin(const long x, const long y)
{
	return x < y ? x : y;
}

inline long lmax(const long x, const long y)
{
	return x > y ? x : y;
}

inline double normangle(double theta)
{
	const double fDifference = 2 * M_PI;
	while (theta > M_PI)
		theta -= fDifference;
	while (theta < -M_PI)
		theta += fDifference;
	return theta;
}

signed int RandInt(const signed int min, const signed int max);
unsigned int RandUInt(const unsigned int min, const unsigned int max);
double RandDouble(const double min, const double max);

// get current time in milliseconds
const struct timeval timeval0 = {0, 0};

bool GetTimeMode();
void SetTimeMode(bool bSimulated, struct timeval tIncrement = timeval0);
void IncrementTime();
struct timeval GetTimeIncrement();
struct timeval GetCurrentTime();
struct timeval GetRealTime();
inline struct timeval MakeTime(long int iSeconds, long int iUSeconds)
{
	struct timeval t = {iSeconds, iUSeconds};
	return t;
}
inline struct timeval MakeTime(double fTime)
{
#ifdef DEBUG
	Q_ASSERT(fTime >= 0.);
#endif

	struct timeval t;
	double fInt, fFrac;
	fFrac = modf(fTime, &fInt);
	t.tv_sec = (long int)fInt;
	t.tv_usec = (long int)(fFrac * 1000000);
	return t;
}

struct timeval operator + (const struct timeval & t1, const struct timeval & t2);
struct timeval operator - (const struct timeval & t1, const struct timeval & t2);
struct timeval operator * (const struct timeval & t, const double fScale);
inline bool operator == (const struct timeval & t1, const struct timeval & t2)
{
	return t1.tv_sec == t2.tv_sec && t1.tv_usec == t2.tv_usec;
}
inline bool operator != (const struct timeval & t1, const struct timeval & t2)
{
	return t1.tv_sec != t2.tv_sec || t1.tv_usec != t2.tv_usec;
}
inline bool operator < (const struct timeval & t1, const struct timeval & t2)
{
	return t1.tv_sec < t2.tv_sec || (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}
inline bool operator > (const struct timeval & t1, const struct timeval & t2)
{
	return t1.tv_sec > t2.tv_sec || (t1.tv_sec == t2.tv_sec && t1.tv_usec > t2.tv_usec);
}
inline bool operator <= (const struct timeval & t1, const struct timeval & t2)
{
	return t1.tv_sec < t2.tv_sec || (t1.tv_sec == t2.tv_sec && t1.tv_usec <= t2.tv_usec);
}
inline bool operator >= (const struct timeval & t1, const struct timeval & t2)
{
	return t1.tv_sec > t2.tv_sec || (t1.tv_sec == t2.tv_sec && t1.tv_usec >= t2.tv_usec);
}
inline float ToFloat(const struct timeval & t)
{
	return t.tv_sec + t.tv_usec * 1e-6f;
}
inline double ToDouble(const struct timeval & t)
{
	return t.tv_sec + t.tv_usec * 1e-6;
}


class Buffer
{
public:
	// buffer takes ownership of pData
	Buffer(unsigned char * pData = NULL, unsigned int iLength = 0);
	Buffer(const Buffer & copy);
	~Buffer();

	unsigned char * m_pData;
	unsigned int m_iLength;

	Buffer & operator = (const Buffer & copy);
	Buffer & operator += (const Buffer & add);
};

Buffer operator + (const Buffer & b1, const Buffer & b2);

#endif

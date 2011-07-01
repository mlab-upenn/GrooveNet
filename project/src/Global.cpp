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

#include "Global.h"
#include <cstdlib>

#include <sys/time.h>

#define RAND_MAX_BITS 31

static bool g_bSimulated = false;
static struct timeval g_tCurrent = timeval0, g_tIncrement = timeval0;

static struct timeval GetCurrentTimeInternal()
{
	struct timeval tm;

	if (gettimeofday(&tm, NULL) != 0)
		return timeval0;
	tm.tv_usec %= 1000000; // make sure it's a million
	return tm;
}

bool GetTimeMode()
{
	return g_bSimulated;
}

void SetTimeMode(bool bSimulated, struct timeval tIncrement)
{
	g_bSimulated = bSimulated;
	if (g_bSimulated)
		g_tCurrent = GetCurrentTimeInternal();
	g_tIncrement = tIncrement;
}

struct timeval GetTimeIncrement()
{
	return g_tIncrement;
}

void IncrementTime()
{
	g_tCurrent = g_tCurrent + g_tIncrement;
}

struct timeval GetCurrentTime()
{
	if (g_bSimulated)
		return g_tCurrent;
	else
		return GetCurrentTimeInternal();
}

struct timeval GetRealTime()
{
	return GetCurrentTimeInternal();
}

struct timeval operator + (const struct timeval & t1, const struct timeval & t2)
{
	struct timeval t;
	t.tv_sec = t1.tv_sec + t2.tv_sec;
	t.tv_usec = t1.tv_usec + t2.tv_usec;
	if (t.tv_usec >= 1000000) { // take care of carry
		t.tv_usec -= 1000000;
		t.tv_sec++;
	}
	return t;
}

struct timeval operator - (const struct timeval & t1, const struct timeval & t2)
{
	struct timeval t;
	t.tv_sec = t1.tv_sec - t2.tv_sec;
	t.tv_usec = t1.tv_usec - t2.tv_usec;
	if (t.tv_usec < 0) { // take care of carry
		t.tv_usec += 1000000;
		t.tv_sec--;
	}
	return t;
}

struct timeval operator * (const struct timeval & t, const double fScale)
{
	return MakeTime(ToDouble(t)*fScale);
}

signed int RandInt(const signed int min, const signed int max)
{
	signed int ret = rand();
	if (max > min)
		return (ret % (max - min)) + min;
	else
		return min;
}

unsigned int RandUInt(const unsigned int min, const unsigned int max)
{
	unsigned int ret = 0;
	unsigned int bits;
	for (bits = 0; bits < (sizeof(unsigned int) << 3); bits += RAND_MAX_BITS)
		ret = (ret << RAND_MAX_BITS) | rand();
	if (max > min)
		return (ret % (max - min)) + min;
	else
		return min;
}

double RandDouble(const double min, const double max)
{
	double ret = ((double)rand()) / RAND_MAX;
	if (max > min)
		return (ret * (max - min)) + min;
	else
		return min;
}

Buffer::Buffer(unsigned char * pData, unsigned int iLength)
: m_pData(pData), m_iLength(iLength)
{
	if (m_iLength > 0 && m_pData == NULL)
		m_pData = (unsigned char *)malloc(m_iLength);
}

Buffer::Buffer(const Buffer & copy)
: m_pData(copy.m_iLength ? (unsigned char *)malloc(copy.m_iLength) : NULL), m_iLength(copy.m_iLength)
{
	if (m_pData != NULL && copy.m_pData != NULL)
		memcpy(m_pData, copy.m_pData, m_iLength);
}

Buffer::~Buffer()
{
	if (m_pData != NULL)
		free(m_pData);
}

Buffer & Buffer::operator = (const Buffer & copy)
{
	unsigned char * pData = NULL;
	m_iLength = copy.m_iLength;
	if (m_iLength > 0)
		pData = (unsigned char *)malloc(m_iLength);
	if (pData != NULL && copy.m_pData != NULL)
		memcpy(pData, copy.m_pData, m_iLength);
	if (m_pData != NULL)
		free(m_pData);
	m_pData = pData;
	return *this;
}

Buffer & Buffer::operator += (const Buffer & copy)
{
	unsigned char * pData = NULL;
	unsigned int iLength = m_iLength + copy.m_iLength;
	if (iLength > 0)
		pData = (unsigned char *)malloc(iLength);
	if (pData != NULL && m_pData != NULL)
		memcpy(pData, m_pData, m_iLength);
	if (pData != NULL && copy.m_pData != NULL)
		memcpy(pData + m_iLength, copy.m_pData, copy.m_iLength);
	if (m_pData != NULL)
		free(m_pData);
	m_pData = pData;
	m_iLength = iLength;
	return *this;
}

Buffer operator + (const Buffer & b1, const Buffer & b2)
{
	unsigned char * pData = NULL;
	unsigned int iLength = b1.m_iLength + b2.m_iLength;
	if (iLength > 0)
		pData = (unsigned char *)malloc(iLength);
	if (pData != NULL && b1.m_pData != NULL)
		memcpy(pData, b1.m_pData, b1.m_iLength);
	if (pData != NULL && b2.m_pData != NULL)
		memcpy(pData + b1.m_iLength, b2.m_pData, b2.m_iLength);
	return Buffer(pData, iLength);
}

/***************************************************************************
                          Coords.cpp - Two-dimensional coordinates class and
                          two-dimensional rectangle class. Coordinates
                          defined using TIGER (millionths of a degree) units
                             -------------------
    begin                : Mon Nov 29 2004
    copyright            : (C) 2004 by Daniel Weller
    email                : dweller@andrew.cmu.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Coords.h"

#include "Global.h"

Coords::Coords()
: m_iLong(0), m_iLat(0)
{
}

Coords::Coords(long iLong, long iLat)
: m_iLong(iLong), m_iLat(iLat)
{
}

QString Coords::ToString()
{
	return QString("%1, %2").arg(m_iLong).arg(m_iLat);
}

bool Coords::FromString(const QString & strDesc)
{
	int iSep = strDesc.find(',');
	if (iSep > -1)
	{
		bool bOK;
		m_iLong = strDesc.left(iSep).stripWhiteSpace().toLong(&bOK);
		if (bOK)
			m_iLat = strDesc.mid(iSep+1).stripWhiteSpace().toLong(&bOK);
		return bOK;
	} else
		return false;
}

Coords Coords::operator + (const Coords & x) const
{
	return Coords(x.m_iLong + m_iLong, x.m_iLat + m_iLat);
}

Coords Coords::operator - (const Coords & x) const
{
	return Coords(m_iLong - x.m_iLong, m_iLat - x.m_iLat);
}

Coords Coords::operator / (double x) const
{
	return Coords((long)(m_iLong / x), (long)(m_iLat / x));
}

Coords Coords::operator * (double x) const
{
	return Coords((long)(m_iLong * x), (long)(m_iLat * x));
}

double Coords::abs()
{
	return sqrt((double)m_iLong * m_iLong + m_iLat * m_iLat);
}

double Coords::angle()
{
	return atan2((double)m_iLat, m_iLong);
}

float Distance(const Coords & pt1, const Coords & pt2)
{
	float sindLong = sinf((pt1.m_iLong - pt2.m_iLong) * M_PI / 360000000);
	float sindLat = sinf((pt1.m_iLat - pt2.m_iLat) * M_PI / 360000000);

	// haversine distance formula
	float A = sindLat * sindLat + cosf(pt1.m_iLat * M_PI / 180000000) * cosf(pt2.m_iLat * M_PI / 180000000) * sindLong * sindLong;
	float c = 2 * atan2f(sqrtf(A), sqrtf(1-A));
	return EARTHRADIUS * c;
}


Rect::Rect()
: m_iLeft(0), m_iTop(0), m_iRight(0), m_iBottom(0)
{
}

Rect::Rect(long left, long top, long right, long bottom)
: m_iLeft(left), m_iTop(top), m_iRight(right), m_iBottom(bottom)
{
}

Rect::Rect(const Coords & topLeft, const Coords & bottomRight)
: m_iLeft(topLeft.m_iLong), m_iTop(topLeft.m_iLat), m_iRight(bottomRight.m_iLong), m_iBottom(bottomRight.m_iLat)
{
}

Rect::Rect(const Rect & copy)
: m_iLeft(copy.m_iLeft), m_iTop(copy.m_iTop), m_iRight(copy.m_iRight), m_iBottom(copy.m_iBottom)
{
}

Rect & Rect::operator = (const Rect & copy)
{
	m_iLeft = copy.m_iLeft;
	m_iTop = copy.m_iTop;
	m_iRight = copy.m_iRight;
	m_iBottom = copy.m_iBottom;
	return *this;
}

Rect Rect::unionRect(const Rect & r) const
{
	return Rect(lmin(m_iLeft, r.m_iLeft), lmax(m_iTop, r.m_iTop), lmax(m_iRight, r.m_iRight), lmin(m_iBottom, r.m_iBottom));
}

Rect Rect::BoundingRect(Coords * coords, int numCoords)
{
	if (numCoords == 0) return Rect(0,0,0,0);
	Coords * left = coords, * top = coords, * right = coords, * bottom = coords;
	int i;
	// when coordinates are outside bounds, update bounds to include coordinates
	for (i = 1; i < numCoords; i++) {
		++coords;
		if (coords->m_iLong < left->m_iLong) left = coords;
		if (coords->m_iLong > right->m_iLong) right = coords;
		if (coords->m_iLat < bottom->m_iLat) bottom = coords;
		if (coords->m_iLat > top->m_iLat) top = coords;
	}
	return Rect(left->m_iLong, top->m_iLat, right->m_iLong, bottom->m_iLat);
}

Rect Rect::BoundingRect(std::list<Coords>::const_iterator begin, std::list<Coords>::const_iterator end)
{
	if (begin == end) return Rect(0,0,0,0);
	std::list<Coords>::const_iterator left = begin, top = begin, right = begin, bottom = begin;
	// when coordinates are outside bounds, update bounds to include coordinates
	for (++begin; begin != end; ++begin) {
		if (begin->m_iLong < left->m_iLong) left = begin;
		if (begin->m_iLong > right->m_iLong) right = begin;
		if (begin->m_iLat < bottom->m_iLat) bottom = begin;
		if (begin->m_iLat > top->m_iLat) top = begin;
	}
	return Rect(left->m_iLong, top->m_iLat, right->m_iLong, bottom->m_iLat);
}

double Rect::GetArea() const
{
	Coords ptTL(m_iLeft, m_iTop), ptTR(m_iRight, m_iTop), ptBL(m_iLeft, m_iBottom), ptBR(m_iRight, m_iBottom);

	return 0.5 * (Distance(ptTL, ptTR) + Distance(ptBL, ptBR)) * Distance(ptTL, ptBL);
}

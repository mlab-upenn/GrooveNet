/***************************************************************************
                          Coords.h  -  Two-dimensional coordinates class and
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

#ifndef _COORDS_H
#define _COORDS_H

#include <qstring.h>

#include <stdlib.h>
#include <list>

#include "Global.h"

class Coords
{
public:
	/*	Default constructor */
	Coords();
	/*	Constructor sets with specified longitude, latitude */
	Coords(long iLong, long iLat);
	/*	Sets coordinates with longitude, latitude */
	inline void Set(long iLong, long iLat)
	{
		m_iLong = iLong; m_iLat = iLat;
	}

	/*	Return string representation of coordinates: "<TIGER longitude>, <TIGER latitude>" */
	QString ToString();
	/*	Set coordinates from string representation, return true if successful */
	bool FromString(const QString & strDesc);

	/*	Calculate magnitude of coordinates on cartesian plane */
	double abs();
	/*	Calculate phase angle of coordinates on cartesian plane */
	double angle();

	inline Coords Flatten() const
	{
		return Coords((long)(m_iLong * cosf(m_iLat * RADIANSPERTIGERDEGREE)), m_iLat);
	}

	inline Coords & operator +=(const Coords & x)
	{
		m_iLong += x.m_iLong;
		m_iLat += x.m_iLat;
		return *this;
	}
	inline Coords & operator -=(const Coords & x)
	{
		m_iLong -= x.m_iLong;
		m_iLat -= x.m_iLat;
		return *this;
	}
	inline Coords & operator /=(double x)
	{
		m_iLong = (long)(m_iLong / x);
		m_iLat = (long)(m_iLat / x);
		return *this;
	}
	inline Coords & operator *=(double x)
	{
		m_iLong = (long)(m_iLong * x);
		m_iLat = (long)(m_iLat * x);
		return *this;
	}

	/*	Return new coordinates added like vectors */
	Coords operator +(const Coords & x) const;
	/*	Return new coordinates subtracted like vectors */
	Coords operator -(const Coords & x) const;
	/*	Return new coordinates scaled by factor 1/x */
	Coords operator /(double x) const;
	/*	Return new coordinates scaled by factor x */
	Coords operator *(double x) const;

	long m_iLong;
	long m_iLat;
};

inline bool operator < (const Coords & x, const Coords & y)
{
	return x.m_iLong < y.m_iLong || (x.m_iLong == y.m_iLong && x.m_iLat < y.m_iLat);
}

inline bool operator > (const Coords & x, const Coords & y)
{
	return x.m_iLong > y.m_iLong || (x.m_iLong == y.m_iLong && x.m_iLat > y.m_iLat);
}

inline bool operator == (const Coords & x, const Coords & y)
{
	return x.m_iLong == y.m_iLong && x.m_iLat == y.m_iLat;
}

inline bool operator != (const Coords & x, const Coords & y)
{
	return x.m_iLong != y.m_iLong || x.m_iLat != y.m_iLat;
}

float Distance(const Coords & pt1, const Coords & pt2);

class Rect
{
public:
	/*	Create uninitialized rectangle */
	Rect();
	/*	Create rectangle with specified bounding coordinates */
	Rect(long left, long top, long right, long bottom);
	/*	Create rectangle with top-left and bottom-right coordinates */
	Rect(const Coords & topLeft, const Coords & bottomRight);
	/*	Copy constructor */
	Rect(const Rect & copy);

	inline Rect & normalize()
	{
		long iSwap;
		if (m_iLeft > m_iRight)
		{
			iSwap = m_iLeft;
			m_iLeft = m_iRight;
			m_iRight = iSwap;
		}
		if (m_iBottom > m_iTop)
		{
			iSwap = m_iTop;
			m_iTop = m_iBottom;
			m_iBottom = iSwap;
		}
		return *this;
	}

	/*	Returns true if pt lies within this rectangle */
	inline bool contains(const Coords & pt) const {return m_iLeft <= pt.m_iLong && m_iRight >= pt.m_iLong && m_iTop >= pt.m_iLat && m_iBottom <= pt.m_iLat;}
	/* Returns true if r lies within this rectangle */
	inline bool contains(const Rect & r) const {return m_iLeft <= r.m_iLeft && m_iRight >= r.m_iRight && m_iTop >= r.m_iTop && m_iBottom <= r.m_iBottom;}
	/*	Returns true if r intersects this rectangle */
	inline bool intersectRect(const Rect & r) const {return m_iLeft <= r.m_iRight && m_iRight >= r.m_iLeft && m_iTop >= r.m_iBottom && m_iBottom <= r.m_iTop;}
	/*	Returns the union of this rectangle and r */
	Rect unionRect(const Rect & r) const;

	/*	Assign copy to this rectangle, return reference to this rectangle */
	Rect & operator = (const Rect & copy);

	/*	Create a new rectangle from list of coordinates */
	static Rect BoundingRect(Coords * coords, int numCoords);
	/*	Like above function using iterator instead of pointer to array of coordinates */
	static Rect BoundingRect(std::list<Coords>::const_iterator begin, std::list<Coords>::const_iterator end);

	double GetArea() const;
	inline Coords GetCenter() const
	{
		return Coords((m_iLeft + m_iRight) >> 1, (m_iTop + m_iBottom) >> 1);
	}

	long m_iLeft;
	long m_iTop;
	long m_iRight;
	long m_iBottom;
};

#endif

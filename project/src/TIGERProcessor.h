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

#ifndef _TIGERPROCESSOR_H
#define _TIGERPROCESSOR_H

#include <stdlib.h>
#include <vector>
#include <map>

#include "Coords.h"
#include "MapDB.h"

/* Put all the TIGER data in one preprocessed file:

This file's format is the following: (TGR<FIPS code>.MAP)

<county code><bounding rect.>
int          Rect
<# of strings>
<first string>\0...\0<last string>\0 (note that each string is null-terminated!)
<# of vertices>
<first vertex>
<last vertex>
<# of records>
<first record>
...
<last record>
<# of polygons>
<first polygon>
<last polygon>

Each vertex contains the following all on one line (n bytes - variable):
<coord><# of neighbors><first neighbor = <vertex #><record #>>...<last neighbor = <vertex #><record #>>
Coords ushort                            uint      uint                           uint      uint

Each record contains the following (n bytes - variable):

<# of feature names><first feature name><first feature type>...<last feature name><last feature type>
ushort              uint                uint                   uint               uint
<cost><waterLeft/waterRight/record type><waterLeft pt.><waterRight pt.>
float byte[7,6,5..0]                    Coords(8byte)  Coords(8byte)
<# of address ranges><first address range>...<last address range>
ushort               AddressRange(8byte)     AddressRange(8byte)
<bounding rect.><# of coords><first coord>...<last coord>
Rect            ushort       Coords(8byte)   Coords(8byte)
<# of adj. vertices><first adj. vertex>...<last adj. vertex>
ushort              uint                  uint

Each polygon contains the following (n bytes - variable)
<bounding rect.><# of points><first point>...<last point>
Rect            unsigned int Coords          Coords

Coords data structure (8 bytes): <longitude><latitude>
Rect data structure (16 bytes): <left><top><right><bottom>
AddressRange data structure (8 bytes): <from><to><zip>, if zip is negative (high bit set), then on left, otherwise not
*/

class TIGERProcessor {
public:
	/*	Default constructor */
	TIGERProcessor();
	/*	Default destructor */
	~TIGERProcessor();

	/*	Cleanup - free records, vertices, etc. */
	void Cleanup();

	/*	Load type 1 TIGER data */
	bool LoadType1(const QString & strFilename);	
	/*	Load type 2 TIGER data */
	bool LoadType2(const QString & strFilename);
	/*	Load type 4 TIGER data */
	bool LoadType4(const QString & strFilename);	
	/*	Load type 5 TIGER data */
	bool LoadType5(const QString & strFilename);
	/*	Load type 6 TIGER data */
	bool LoadType6(const QString & strFilename);	
	/*	Load type I TIGER data */
	bool LoadTypeI(const QString & strFilename);	
	/*	Load type P TIGER data */
	bool LoadTypeP(const QString & strFilename);	
	/*	Load TIGER data set */
	bool LoadSet(const QString & strBaseName);	

	/*	Write TIGER data */
	bool WriteMap(const QString & fileName);

	/*	Enumerate vertices from endpoints of records, also do other preprocessing */
	void EnumerateVertices();
	/*	Fix zip codes of records that do not have zip codes specified */
	void FixZipCodes();
	/*	Add string to string list, return index of string in list */
	unsigned int AddString(const QString & str);

protected:
	std::vector<QString> m_Strings;
	QString stateName;
	// county code
	int countyCode;
	// bounding rectangle
	Rect boundingRect;

	// coordinates -> vertex #
	std::map<Coords, unsigned int> m_mapCoordinateToVertex;	

	std::vector<Vertex> m_Vertices; // contains vertex pairs (edges) and record indexes

	std::list<std::pair<Rect, std::list<Coords> > > m_WaterPolygons;

	// the records themselves
	MapRecord * m_pRecords;

	// the number of records
	unsigned int m_nRecords;

	// some temporary variables used during the loading process
	std::map<unsigned int, unsigned int> m_mapTLIDtoRecord;	
	std::map<unsigned int, std::list<std::pair<unsigned int, bool> > > m_mapPolyIDtoRecords;
	std::map<unsigned int, std::vector<unsigned int> > m_mapAdditionalNameIDtoRecord;	
	std::map<QString, unsigned int> m_mapStringsToIndex; // temporary variable
	Coords * areaLeft, * areaRight, * areaTop, * areaBottom;
};

/*	Write memory buffer to file */
void WriteMemory(const void * pMem, int fd, const unsigned int length);
/*	Write coordinates to file */
void WriteCoords(const Coords * src, int fd);
/*	Write rectangle to file */
void WriteRect(const Rect * src, int fd);
/*	Write address range and zip to file */
void WriteAddressRange(const AddressRange * src, int fd);
/*	Write string to file */
void WriteString(const char * src, int fd, const unsigned int length);

/*	Read memory buffer from file */
bool ReadMemory(void * pMem, int fd, const unsigned int length);
/*	Read coords from file */
bool ReadCoords(Coords * dest, int fd);
/*	Read rectangle from file */
bool ReadRect(Rect * dest, int fd);
/*	Read address range and zip from file */
bool ReadAddressRange(AddressRange * dest, int fd);
/*	Read string from file */
void ReadString(char * dest, int fd, const unsigned int destLength);

#endif

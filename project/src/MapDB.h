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

/* MapDB.h -- this file incorporates map and map database support into this
 * software. This file contains several elements: important data structures
 * used for the map database, global helper functions for manipulating and
 * utilizing the map database's information, and global functions and
 * variables used for setting up and accessing related global information.
 * This file also contains data structures and functions helpful for displaying
 * map information on the screen.
 */

#ifndef _MAPDB_H
#define _MAPDB_H

#include "Global.h"
#include "Coords.h"
#include "FibonacciHeap.h"

#include <stdlib.h>
#include <vector>
#include <map>
#include <set>

#include <qdatetime.h>
#include <qmutex.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qcolor.h>

// this is the distance threshold between a point and a road segment associated
// with that point
// it is in TIGER coordinate units
#define MAX_DISTANCE_THRESHOLD 90

// this enumeration lists the different types of records found in the map
// database
// the default type has no meaning -- it's for records of no other type
typedef enum RecordTypesEnum
{
	RecordTypeDefault = 0,
	RecordTypeOneWaySmallRoad,
	RecordTypeOneWayLargeRoad,
	RecordTypeOneWayPrimary,
	RecordTypeOneWayHighway,
	RecordTypeTwoWaySmallRoad,
	RecordTypeTwoWayLargeRoad,
	RecordTypeTwoWayPrimary,
	RecordTypeTwoWayHighway,
	RecordTypeRailroad,
	RecordTypeWater,
	RecordTypeInvisibleLandBoundary,
	RecordTypeInvisibleWaterBoundary,
	RecordTypeInvisibleMiscBoundary,
	RecordTypeLandmark,
	RecordTypePhysicalFeature,
	RecordTypeTownOrCity
} RecordTypes;

// this is the number of record types -- make sure this is correct, or the
// software may not work properly!
#define NUM_RECORD_TYPES ((int)RecordTypeTownOrCity + 1)

// these are the minimum and maximum values for the zoom level of a map display
// they correspond to the scale of the map; a higher number is more zoomed
// out than a lower number
#define MIN_DETAIL_LEVEL 1
#define MAX_DETAIL_LEVEL 15

// this data structure should identify a street address or intersection
// because of the peculiarities of the road system, a uniquely identifiable
// address requires either a vertex number for an intersection or a record
// number and street number for a street address
// the coordinates of the address can uniquely specify either an intersection
// or street address; however the procedure of isolating the address in question
// from its longitude and latitude is not truly straightforward
typedef struct AddressStruct
{
	QString szStreetName;
	QString szStreetType;
	QString szCityName;
	QString szStateName;
	Coords ptCoordinates;
	int iStreetNumber;
	unsigned int iZipCode;
	unsigned int iRecord;
	unsigned int iVertex;
} Address;

// given a string representation of a street address or intersection, this
// function will parse that string and fill the specified address structure
// with that address' information
// this function returns true if it could find an address, false otherwise
bool StringToAddress(const QString & strValue, Address * pAddress);

// given an address (street address or intersection), this function will
// construct a string that describes this address
// if you then pass this string to the above function, the same address
// won't necessarily be returned, because these strings cannot uniquely
// identify all addresses (TODO: work on this issue)
QString AddressToString(const Address * pAddress);

// this data structure specifies a range of street addresses (such as those
// used by the US Postal Service) along a segment of a road
// a given road segment may have several such ranges
// ranges may be for either the left or right (or both) sides of the road
typedef struct AddressRangeStruct
{
	unsigned short iFromAddr; // the address at the segment's starting point
	unsigned short iToAddr; // the address at the segment's ending point
	unsigned int iZip; // the ZIP code in this address range; is not necessarily valid
	bool bOnLeft; // true if address range corresponds to the "left" side of the road,
	              // false otherwise
} AddressRange;

// this data structure describes a record in the map database
// a record can be thought of as representing an edge in a graph,
// where every intersection on the map is a node in the graph
// in the TIGER database, the graph is planar, so not all vertices are
// true intersections
typedef struct MapRecordStruct
{
	Coords * pShapePoints; // an array of TIGER coordinates that describe the shape
	                       // of the segment
	Rect rBounds; // the segment's bounding rectangle
	AddressRange * pAddressRanges; // an array of address ranges for this segment
	Coords ptWaterL, ptWaterR; // TIGER coordinates for the centroids of water polygons
	                           // on the left or right of the record (if there is water)
	RecordTypes eRecordType; // the type of record
	float fCost; // the cost for driving the length of the record (for Djikstra's)
	unsigned int * pVertices; // a list of vertices that this record intersects
	unsigned int * pFeatureNames; // the names of this road (e.g. Main or State)
	unsigned int * pFeatureTypes; // the types of this road (e.g. Street or Avenue)
	unsigned short nFeatureNames; // the number of name and type pairs for this segment
	unsigned short nAddressRanges; // the number of address ranges for this segment
	unsigned short nShapePoints; // the number of TIGER coordinates in the shape points
	                             // array
	unsigned short nVertices; // the number of vertices (normally 1 or 2)
	bool bWaterL, bWaterR; // true of there is water on the left or right side of the
	                       // record
} MapRecord;

bool IsRoad(const MapRecord * pRecord);
bool IsBigRoad(const MapRecord * pRecord);
bool IsOneWay(const MapRecord * pRecord);
bool IsSameRoad(const MapRecord * pRecord1, const MapRecord * pRecord2);
unsigned char NumberOfLanes(const MapRecord * pRecord);
float CostFactor(const MapRecord * pRecord);
float TimeFactor(const MapRecord * pRecord);
float RecordDistance(const MapRecord * pRecord);
float PointRecordDistance(const Coords & pt, const MapRecord * pRecord, unsigned short & iShapePoint, float & fProgress);
bool IsVehicleGoingForwards(unsigned short iShapePoint, short iHeading, const MapRecord * pRecord);
float DistanceAlongRecord(const MapRecord * pRecord, unsigned short iStartShapePoint, float fStartProgress, unsigned short iEndShapePoint, float fEndProgress);

typedef struct DijkstraVertexStruct {
	unsigned int vertex;
	unsigned int predecessor;
	unsigned int record;
} DijkstraVertex;

typedef struct DetailSettingsStruct
{
	QColor clrLine;
	QColor clrText;
	int iWidth;
	int iFontSize;
	int iStyle;
	int iArrowSpacing;
	bool bTextVisible;
	bool bLineVisible;
} DetailSettings;

typedef struct MapDrawingSettingsStruct {
	DetailSettings vecLevelDetails[MAX_DETAIL_LEVEL+1][NUM_RECORD_TYPES];
	QRect rUpdate;
	QPainter * pMemoryDC;
	QPixmap * pOldEverythingBmp;
	QPixmap * pOldMapBmp;
	Coords ptTopLeftClip;
	Coords ptBottomRightClip;
	Coords ptTopLeft;
	Coords ptBottomRight;
	Coords ptCenter;
	struct timeval tLastChange;
	QColor clrBackground;
	int iControlWidth;
	int iControlHeight;
	int iDetailLevel;
	int iCurrentObject;
	bool bL1Redraw;
	bool bL2Redraw;
	bool bShowMarkers;
	bool bShowCompass;
	bool bShowScale;
	bool bShowTracks;
	bool bFillInWater;
} MapDrawingSettings;

void InitMapDrawingSettings(MapDrawingSettings * pSettings);
void UpdateMapDrawingSettings(MapDrawingSettings * pSettings);
void CopyMapDrawingSettings(MapDrawingSettings * pDest, const MapDrawingSettings * pSrc);
void ReplaceOldEverythingBmp(MapDrawingSettings * pDrawingSettings, QPixmap * pNewImage);
void ReplaceOldMapBmp(MapDrawingSettings * pDrawingSettings, QPixmap * pNewImage);
void InvalidateRect(MapDrawingSettings * pSettings, const QRect & rClip = QRect(), bool bL1 = true, bool bL2 = false);

inline QPoint MapLongLatToScreen(MapDrawingSettings * pSettings, const Coords & pt) {
//	return QPoint((int)((pt.m_iLong - pSettings->ptTopLeft.m_iLong) / pSettings->vecLevelScales[pSettings->iDetailLevel] * cos(pt.m_iLat * M_PI / 180000000.)), (int)((pSettings->ptTopLeft.m_iLat - pt.m_iLat) / pSettings->vecLevelScales[pSettings->iDetailLevel]));
	return QPoint((pt.m_iLong - pSettings->ptTopLeft.m_iLong) >> pSettings->iDetailLevel, (pSettings->ptTopLeft.m_iLat - pt.m_iLat) >> pSettings->iDetailLevel);
}

inline Coords MapScreenToLongLat(MapDrawingSettings * pSettings, const QPoint & ptScreen) {
//	long lat = (long)(pSettings->ptCenter.m_iLat - (ptScreen.y() - pSettings->iControlHeight / 2) * pSettings->vecLevelScales[pSettings->iDetailLevel]);
//	return Coords((long)(pSettings->ptCenter.m_iLong + (ptScreen.x() - pSettings->iControlWidth / 2) * pSettings->vecLevelScales[pSettings->iDetailLevel] / cos(lat * M_PI / 180000000.)), lat);
	return Coords(pSettings->ptCenter.m_iLong + ((ptScreen.x() - pSettings->iControlWidth / 2) << pSettings->iDetailLevel), pSettings->ptCenter.m_iLat - ((ptScreen.y() - pSettings->iControlHeight / 2) << pSettings->iDetailLevel));
}

typedef struct StreetNameAndTypeStruct
{
	unsigned int iStreetName;
	unsigned int iStreetType;
} StreetNameAndType;

inline bool operator < (const StreetNameAndType & s1, const StreetNameAndType & s2)
{
	return s1.iStreetName < s2.iStreetName || s1.iStreetName == s2.iStreetName && s1.iStreetType < s2.iStreetType;
}

bool IsOnScreen(const Coords & pt, const Coords & ptTopLeft, const Coords & ptBottomRight);
bool IsRecordVisible(Coords * coords, int nCoords, const Coords & ptTopLeft, const Coords & ptBottomRight);
bool IsRecordVisible(const std::vector<Coords> & coords, const Coords & ptTopLeft, const Coords & ptBottomRight);

void DrawLine(QPainter * dc, int sx, int sy, int dx, int dy, int iWidth, const QColor & clrColor, int iStyle);
void DrawRotatedText(QPainter * dc, const QString & str, const QPoint & p, const QSize & sz, double angle, const QColor & clrText);

typedef struct VertexStruct
{
	std::map<unsigned int, unsigned int> mapEdges;
	std::vector<unsigned int> vecRoads;
	unsigned int iRoadPermitted;
} Vertex;

void AddRecordToVertex(Vertex * pVertex, const MapRecord * pRecordSet, unsigned int iRecord, unsigned int iPreviousVertex);
bool CanCarGoThrough(const Vertex & vertex, unsigned int iRecord);

typedef std::pair<unsigned int, unsigned int> RecordRange;
typedef std::vector<unsigned int> CountySquare;
typedef std::vector<CountySquare> CountySquares;
typedef std::pair<Rect, std::vector<Coords> > WaterPolygon;
typedef std::vector<WaterPolygon> WaterPolygons;

class MapDB
{
public:
	MapDB();
	~MapDB();

	void Clear();
	void ResetTrafficLights();

	bool IsCountyLoaded(unsigned short iFIPSCode);
	bool DownloadCounties(const std::set<unsigned short> & setFIPSCodes);
	bool DownloadCounty(unsigned short iFIPSCode);
	bool LoadAll(const QString & strDirectory);
	bool LoadAll(const QString & strDirectory, const std::set<QString> & setFilenames);

	inline bool UseTrafficLights() const
	{
		return m_bTrafficLights;
	}
	inline void UseTrafficLights(bool bTrafficLights)
	{
		m_bTrafficLights = bTrafficLights;
	}
	inline struct timeval GetLastChange() const
	{
		return m_tLastChange;
	}
	inline unsigned int GetCount() const
	{
		return m_nRecords;
	}
	inline MapRecord * const GetRecord(unsigned int iRec) const
	{
		return m_pRecords + iRec;
	}
	inline MapRecord * GetRecord(unsigned int iRec)
	{
		return m_pRecords + iRec;
	}
	inline const Vertex & GetVertex(unsigned int iVertex) const
	{
		return m_vecVertices[iVertex];
	}
	inline Vertex & GetVertex(unsigned int iVertex)
	{
		return m_vecVertices[iVertex];
	}
	inline unsigned int GetVertexCount() const
	{
		return m_vecVertices.size();
	}
	inline const QString & GetString(unsigned int i) const
	{
		return m_vecStrings[i];
	}
	inline QString & GetString(unsigned int i)
	{
		return m_vecStrings[i];
	}
	inline unsigned int GetStringIndex(const QString & str) const
	{
		std::map<QString, unsigned int>::const_iterator iterString;
		return (iterString = m_mapStringsToIndex.find(str)) == m_mapStringsToIndex.end() ? (unsigned)-1 : iterString->second;
	}
	void GetStreetsByName(const QString & strStreetName, const QString & strStreetType, std::set<unsigned int> & setMatching);
	QString GetNameAndType(unsigned int iRec, int iEntry = 0, bool bLongTypeForm = false);
	bool IsVertex(unsigned int iVertex, const std::set<StreetNameAndType> & setStreets);
	bool GetVertex(unsigned int iVertex, Address * pAddress);

	bool FindAddress(Address * pAddress, int iSearchNumber, const QString & strSearchStreet, const QString & strSearchType, const QString & strCity, const QString & strState);
	bool FindCoordinates(Address * pAddress, const Coords & ptSearch);
	bool GetRandomAddress(Address * pAddress, const Rect & rRect);
	bool GetRecordsInRegion(std::vector<unsigned int> & vecRecords, const Rect & rRect);
	bool AddressFromRecord(Address * pAddress, unsigned int iRecord, unsigned int iShapePoint, float fProgress);
	std::list<unsigned int> ShortestPath(Address * pStart, Address * pEnd, bool & bBackwardsStart, bool & bBackwardsEnd);
	std::list<unsigned int> ShortestPath(unsigned int iStartVertex, unsigned int iStartRecord, unsigned short iStartShapePoint, float fStartProgress, unsigned int iEndVertex, unsigned int iEndRecord, unsigned short iEndShapePoint, float fEndProgress, bool & bBackwardsStart, bool & bBackwardsEnd);
	void DrawMap(MapDrawingSettings * pSettings, const QRect & rMap);
	void DrawBorder(MapDrawingSettings * pSettings, bool bFocus);
	void DrawRecordHighlights(MapDrawingSettings * pSettings, const std::set<unsigned int> & setRecords, const QColor & clrHighlight);
	bool AddressToPosition(Address * pAddress, unsigned int & iRecord, unsigned int & iShapePoint, float & fProgress);
	bool GetNextPossibleRecords(std::vector<unsigned int> & vecRecords, unsigned int iVertex, unsigned int iPrevRecord = (unsigned)-1);
	bool CoordsToRecord(const Coords & ptPosition, unsigned int & iRecordClosest, unsigned short & iShapePointClosest, float & fProgressClosest, float fDistanceClosest = MAX_DISTANCE_THRESHOLD);
	bool GetRelativeRecord(unsigned int & iRecord, unsigned short & iCountyCode);
	bool GetAbsoluteRecord(unsigned int & iRecord, unsigned short iCountyCode);

protected:
	bool LoadMap(const QString & strBaseName);
	void AddRecordsToRegionSquares(unsigned int begin, unsigned int end, CountySquares * squares, const Rect & totalBounds);
	unsigned int AddString(const QString & str);
	void DrawMapFeatures(MapDrawingSettings * pSettings);
	void DrawMapCompass(MapDrawingSettings * pSettings);
	void DrawMapKey(MapDrawingSettings * pSettings);

	struct timeval m_tLastChange;
	QMutex m_Mutex;

	MapRecord * m_pRecords;
	unsigned int m_nRecords;
	std::vector<Vertex> m_vecVertices;
	bool m_bTrafficLights;

	std::vector<QString> m_vecStrings;
	std::vector<std::vector<unsigned int> > m_vecStringRoads;

	std::map<unsigned short, RecordRange > m_mapCountyCodeToRecords;
	std::map<unsigned short, Rect> m_mapCountyCodeToBoundingRect;
	std::map<unsigned short, CountySquares > m_mapCountyCodeToRegions;
	std::map<unsigned short, WaterPolygons > m_mapCountyCodeToWaterPolys;

	std::map<Coords, unsigned int> m_mapCoordinateToVertex;

	std::vector<FibonacciHeapNode<double, DijkstraVertex> * > m_vecVerticesHeapLookup;

	// some temporary variables used during the loading process
	std::map<unsigned int, unsigned int> m_mapTLIDtoRecord;
	std::map<unsigned int, unsigned int> m_mapPolyIDtoRecord;
	std::map<unsigned int, unsigned int> m_mapPolyIDtoSide;
	std::map<unsigned int, std::vector<unsigned int> > m_mapAdditionalNameIDtoRecords;
	std::map<QString, unsigned int> m_mapStringsToIndex; // temporary variable
	Coords * m_pAreaLeft, * m_pAreaRight, * m_pAreaTop, * m_pAreaBottom;
};

bool InitMapDB();
bool LoadAbbreviationInformation(const QString & strFilename);
QString GetAbbreviation(const QString & strName);
QString GetFullName(const QString & strName);
bool IsFeatureType(const QString & strName);
bool LoadCountyInformation(const QString & strFilename);
unsigned short CountyCodeByName(const QString & strName);
QString CountyNameByCode(unsigned short iCode);
bool LoadStateInformation(const QString & strFilename);
QString StateAbbreviationByCode(unsigned short iCode);
QString StateAbbreviationByName(const QString & strName);
QString StateNameByCode(unsigned short iStateCode);
QString StateNameByAbbreviation(const QString & strName);
unsigned short StateCodeByName(const QString & strStateName);
char * ExtractField(char * pszText, int iStart, int iEnd);
QString FormatAddress(Address * psAddr);
QString FormatName(const QString & strIn);
QString FormatAbbreviation(const QString & strIn);
QString GetDataPath(const QString & strFilename = "", const QString & strURL = "", bool downloadAnyway = false);
bool LoadZipInformation(const QString & strFilename);
std::vector<unsigned int> ZipCodesByCityState(const QString & strCity, const QString & strState);
QString StateByZip(unsigned int iZip);
QString CityByZip(unsigned int iZip);
int CountyCodeByZip(unsigned int iZip);
float PointSegmentDistance(const Coords & pt, const Coords & s0, const Coords & s1, float & fProgress);

extern std::map<QString,QString> g_mapAbbreviationToFullName;
extern std::map<QString,QString> g_mapNameToAbbreviation;

extern std::map<QString, unsigned short> g_mapCountyNameToCode;
extern std::map<unsigned short, QString> g_mapCodeToCountyName;
extern std::vector<QString> g_vecCountyNames;

extern std::map<QString, unsigned short> g_mapStateNameToCode;
extern std::map<unsigned short, QString> g_mapStateCodeToName;
extern std::map<QString,QString> g_mapStateAbbreviationToName;
extern std::map<QString,QString> g_mapStateNameToAbbreviation;
extern std::vector<QString> g_vecStateAbbreviations;
extern std::vector<QString> g_vecStateNames;

extern std::map<QString, std::vector<unsigned int> > g_mapCityToZip;
extern std::map<unsigned int, QString> g_mapZipToCity;
extern std::map<unsigned int, QString> g_mapZipToStateName;
extern std::map<unsigned int, unsigned int> g_mapZipToCountyCode;

extern MapDB * g_pMapDB;

#endif

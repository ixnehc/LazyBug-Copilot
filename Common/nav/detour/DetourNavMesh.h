//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef DETOURNAVMESH_H
#define DETOURNAVMESH_H

#include "DetourAlloc.h"
#include "DetourStatus.h"
#include "class/class.h"

// Note: If you want to use 64-bit refs, change the types of both dtPolyRef & dtTileRef.
// It is also recommended to change dtHashRef() to proper 64-bit hash too.

/// Reference to navigation polygon.
typedef unsigned __int64 dtPolyRef;

/// Reference to navigation mesh tile.
typedef unsigned __int64 dtTileRef;

/// Maximum number of vertices per navigation polygon.
static const int DT_VERTS_PER_POLYGON = 6;

static const int DT_NAVMESH_MAGIC = 'D'<<24 | 'N'<<16 | 'A'<<8 | 'V'; ///< 'DNAV'
static const int DT_NAVMESH_VERSION = 7;

static const int DT_NAVMESH_STATE_MAGIC = 'D'<<24 | 'N'<<16 | 'M'<<8 | 'S'; ///< 'DNMS'
static const int DT_NAVMESH_STATE_VERSION = 1;

static const unsigned short DT_EXT_LINK = 0x8000;
static const unsigned int DT_NULL_LINK = 0xffffffff;
static const unsigned int DT_OFFMESH_CON_BIDIR = 1;

static const int DT_MAX_AREAS = 64;

/// Flags for addTile
enum dtTileFlags
{
	DT_TILE_FREE_DATA = 0x01,					///< Navmesh owns the tile memory and should free it.
};

/// Flags returned by findStraightPath().
enum dtStraightPathFlags
{
	DT_STRAIGHTPATH_START = 0x01,				///< The vertex is the start position.
	DT_STRAIGHTPATH_END = 0x02,					///< The vertex is the end position.
	DT_STRAIGHTPATH_OFFMESH_CONNECTION = 0x04,	///< The vertex is start of an off-mesh link.
};

/// Flags describing polygon properties.
enum dtPolyTypes
{
	DT_POLYTYPE_GROUND = 0,						///< Regular ground polygons.
	DT_POLYTYPE_OFFMESH_CONNECTION = 1,			///< Off-mesh connections.
};


/// Structure describing the navigation polygon data.
//Area type:
// Area_NotWalkable=0,
// Area_WalkableAdv=1,//高级权限的可走
// Area_Switchable=2,//可切换
// Area_SwitchOn=3,//切换为Walkable
// Area_SwitchOff=4,//切换为NotWalkable
// Area_Walkable=63,
struct dtPoly
{
	unsigned int firstLink;						///< Index to first link in linked list.
	unsigned short verts[DT_VERTS_PER_POLYGON];	///< Indices to vertices of the poly.
	unsigned short neis[DT_VERTS_PER_POLYGON];	///< Refs to neighbours of the poly.
	unsigned short flags;						///< Flags (see dtPolyFlags).
	unsigned char vertCount;					///< Number of vertices.
	unsigned char areaAndtype;					///< Bit packed: Area ID of the polygon, and Polygon type, see dtPolyTypes..
	inline void setArea(unsigned char a) { areaAndtype = (areaAndtype & 0xc0) | (a & 0x3f); }
	inline void setType(unsigned char t) { areaAndtype = (areaAndtype & 0x3f) | (t << 6); }
	inline unsigned char getArea() const { return areaAndtype & 0x3f; }
	inline unsigned char getType() const { return areaAndtype >> 6; }
};

/// Stucture describing polygon detail triangles.
struct dtPolyDetail
{
	unsigned int vertBase;						///< Offset to detail vertex array.
	unsigned int triBase;						///< Offset to detail triangle array.
	unsigned char vertCount;					///< Number of vertices in the detail mesh.
	unsigned char triCount;						///< Number of triangles.
};

/// Stucture describing a link to another polygon.
struct dtLink
{
	dtPolyRef ref;							///< Neighbour reference.
	unsigned int next;						///< Index to next link.
	unsigned char edge;						///< Index to polygon edge which owns this link.
	unsigned char side;						///< If boundary link, defines on which side the link is.
	unsigned char bmin, bmax;				///< If boundary link, defines the sub edge area.
};

struct dtBVNode
{
	unsigned short bmin[3], bmax[3];		///< BVnode bounds
	int i;									///< Index to item or if negative, escape index.
};

struct dtOffMeshConnection
{
	float pos[6];							///< Both end point locations.
	float rad;								///< Link connection radius.
	unsigned short poly;					///< Poly Id
	unsigned char flags;					///< Link flags
	unsigned char side;						///< End point side.
	unsigned int userId;					///< User ID to identify this connection.
};

struct dtMeshHeader
{
	int magic;								///< Magic number, used to identify the data.
	int version;							///< Data version number.
	int x, y, layer;						///< Location of the tile on the grid.
	unsigned int userId;					///< User ID of the tile.
	int polyCount;							///< Number of polygons in the tile.
	int vertCount;							///< Number of vertices in the tile.
	int maxLinkCount;						///< Number of allocated links.
	int detailMeshCount;					///< Number of detail meshes.
	int detailVertCount;					///< Number of detail vertices.
	int detailTriCount;						///< Number of detail triangles.
	int bvNodeCount;						///< Number of BVtree nodes.
	int offMeshConCount;					///< Number of Off-Mesh links.
	int offMeshBase;						///< Index to first polygon which is Off-Mesh link.
	float walkableHeight;					///< Height of the agent.
	float walkableRadius;					///< Radius of the agent
	float walkableClimb;					///< Max climb height of the agent.
	float bmin[3], bmax[3];					///< Bounding box of the tile.
	float bvQuantFactor;					///< BVtree quantization factor (world to bvnode coords)
};


class dtLinkAlloc
{
public:
	dtLinkAlloc(void)
	{
		clear();
	}
	
	int allocLink(dtLink **lk)
	{
		int idx = 0;
		if(!m_idles.empty())
		{
			idx = m_idles.back();
			m_idles.pop_back();
		}
		else
		{
			idx = m_links.size();
			m_links.resize(idx + 1);
		}

		dtLink * link = &m_links[idx];
		link->next = DT_NULL_LINK;

		if(lk)
			*lk = link;
		
		return idx;
	}

	void freeLink(int idx)
	{
		m_idles.push_back(idx);
	}

	dtLink * getLink(int idx)
	{
		assert(idx<m_links.size());
		return &m_links[idx];
	}

	void clear()
	{
		m_links.clear();
		m_idles.clear();
	}

private:
	std::vector<dtLink>		m_links;
	std::vector<int>		m_idles;
};

struct dtMeshTile
{
	unsigned int linksFreeList;				///< Index to next free link.
	dtMeshHeader* header;					///< Pointer to tile header.
	dtPoly* polys;							///< Pointer to the polygons (will be updated when tile is added).
	float* verts;							///< Pointer to the vertices (will be updated when tile added).

	dtPolyDetail* detailMeshes;				///< Pointer to detail meshes (will be updated when tile added).
	float* detailVerts;						///< Pointer to detail vertices (will be updated when tile added).
	unsigned char* detailTris;				///< Pointer to detail triangles (will be updated when tile added).
	dtBVNode* bvTree;						///< Pointer to BVtree nodes (will be updated when tile added).
	dtOffMeshConnection* offMeshCons;		///< Pointer to Off-Mesh links. (will be updated when tile added).
		
	unsigned char* data;					///< Pointer to tile data.
	int dataSize;							///< Size of the tile data.
	int flags;								///< Tile flags, see dtTileFlags.
	
	bool			 m_hasLink;				///has Link
	dtLinkAlloc 	*m_linkAlloc;			///< Link Alloc

	void unconnectSelfLinks()
	{
		if (!m_linkAlloc) return;

		for (int i = 0; i < header->polyCount; ++i)
		{
			dtPoly* poly = &polys[i];
			int nj = 0;

			for(int j = poly->firstLink;j!=DT_NULL_LINK;)
			{
				dtLink *link = m_linkAlloc->getLink(j);
				nj = link->next;
				m_linkAlloc->freeLink(j);
				j = nj;
			}

			poly->firstLink = DT_NULL_LINK;
		}
	}
};

class dtMeshBase
{
public:
	virtual dtStatus getTileAndPolyByRef(const dtPolyRef &ref, const dtMeshTile** tile, const dtPoly** poly) const = 0;
	virtual void getTileAndPolyByRefUnsafe(const dtPolyRef &ref, const dtMeshTile** tile, const dtPoly** poly) const = 0;
	virtual bool isValidPolyRef(dtPolyRef &ref) const = 0;
	virtual void calcTileLoc(const float* pos, int* tx, int* ty) const = 0;
	virtual int getTilesAt(int x, int y,dtMeshTile ** tiles, int maxTiles) const = 0;

	virtual unsigned int decodePolyIdPoly(const dtPolyRef& ref) const;
	virtual int decodePolyIdTile(const dtPolyRef &ref) const;
	virtual void decodePolyLoc(const dtPolyRef &ref,short &x,short &y) const;
	virtual dtPolyRef getPolyRefBase(const dtMeshTile* tile) const;
	void	addTileLink(dtMeshTile *tile);
	void	removeTileLink(dtMeshTile *tile);

protected:
	int getNeighbourTilesAt(const int x, const int y, const int side,dtMeshTile** tiles, const int maxTiles) const;

	//Link polygon
	void connectIntLinks(dtMeshTile* tile);
	void connectIntOffMeshLinks(dtMeshTile* tile);
	void connectExtLinks(dtMeshTile* tile, dtMeshTile* target, int side);
	void connectExtOffMeshLinks(dtMeshTile* tile, dtMeshTile* target, int side);
	void unconnectExtLinks(dtMeshTile* tile, dtMeshTile* target);

	//polygon test
	void closestPointOnPolyInTile(const dtMeshTile* tile, unsigned int ip,
		const float* pos, float* closest) const;

	int queryPolygonsInTile(const dtMeshTile* tile, const float* qmin, const float* qmax,
		dtPolyRef* polys, const int maxPolys) const;

	dtPolyRef findNearestPolyInTile(const dtMeshTile* tile, const float* center,const float* extents, float* nearestPt) const;

	int findConnectingPolys(const float* va, const float* vb,
		const dtMeshTile* tile, int side,
		dtPolyRef* con, float* conarea, int maxcon) const;
	
	// Tile identify
	dtTileRef getTileRef(const dtMeshTile* tile) const {return dtTileRef(getPolyRefBase(tile));}
};


#endif // DETOURNAVMESH_H

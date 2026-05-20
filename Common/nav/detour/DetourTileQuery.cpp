
#include "stdh.h"

#include "DetourTileQuery.h"


dtStatus dtMeshTileQuery::MeshTile::getTileAndPolyByRef(const dtPolyRef &ref, const dtMeshTile** tile, const dtPoly** poly) const
{
	if(!m_tileMesh)
		return DT_FAILURE | DT_INVALID_PARAM;
	
	int ip = decodePolyIdPoly(ref);
	if(ip>= m_tileMesh->header->polyCount)
		return DT_FAILURE | DT_INVALID_PARAM;

	*tile = m_tileMesh;
	*poly = m_tileMesh->polys + ip;

	return DT_SUCCESS;
}

bool dtMeshTileQuery::MeshTile::isValidPolyRef(dtPolyRef &ref) const
{
	int ip = decodePolyIdPoly(ref);
	return m_tileMesh&&(ip<m_tileMesh->header->polyCount);
}

void dtMeshTileQuery::MeshTile::getTileAndPolyByRefUnsafe(const dtPolyRef &ref, const dtMeshTile** tile, const dtPoly** poly) const
{
	assert(m_tileMesh);

	int ip = decodePolyIdPoly(ref);
	assert(ip<m_tileMesh->header->polyCount);

	*tile = m_tileMesh;
	*poly = m_tileMesh->polys + ip;
}

int dtMeshTileQuery::MeshTile::getTilesAt(int x, int y,dtMeshTile ** tiles, int maxTiles) const
{
	if(m_tileMesh)
	{
		assert(x==m_tileMesh->header->x&&y==m_tileMesh->header->y);
		tiles[0] = m_tileMesh;
		return 1;
	}
	return 0;
}

void dtMeshTileQuery::MeshTile::calcTileLoc(const float* pos, int* tx, int* ty) const
{
	*tx = m_tileMesh->header->x;
	*ty = m_tileMesh->header->y;
}

//////////////////////////////////////////////////////////////////////////
void dtMeshTileQuery::init(int maxNodes)
{
	m_query.init(&m_navMesh,maxNodes);
	m_pathRef = new dtPolyRef[maxNodes];
}

void dtMeshTileQuery::unInit()
{
	delete []m_pathRef;
	m_pathRef = 0;
}

void dtMeshTileQuery::setMeshTile(dtMeshTile * tile)
{
	m_navMesh.m_tileMesh = tile;
	if(!tile->m_hasLink)
	{
		assert(tile->m_linkAlloc);
		m_navMesh.connectIntLinks(tile);
		tile->m_hasLink = true;
	}
}

bool dtMeshTileQuery::isCon(const dtVec3 &startPos,const dtVec3 &endPos)
{
	int nPoly = 0;
	const float ext[3] = {1.0f,1.0f,1.0f};

	if (dtStatusFailed(m_query.findNearestPoly((const float *)&startPos,ext,&m_filter,&m_startRef,0)))
		return false;
	if (dtStatusFailed(m_query.findNearestPoly((const float *)&endPos,ext,&m_filter,&m_endRef,0)))
		return false;
	
	if(DT_SUCCESS ==m_query.findPath(m_startRef,m_endRef,
		(float *)&startPos,(float *)&endPos,
		&m_filter,
		m_pathRef,&nPoly,MAX_POLY_REF,true))
	{
		return true;
	}

	return false;
}




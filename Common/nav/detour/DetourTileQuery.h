
#pragma once

#include "DetourNavMesh.h"

#include "DetourNavMeshQuery.h"

#include "DetourMath.h"

#define  MAX_POLY_REF	256

class dtMeshTileQuery
{
public:
	void init(int maxNodes);
	void unInit();
	void setMeshTile(dtMeshTile * mesh);
	bool isCon(const dtVec3 &startPos,const dtVec3 &endPos);
	
	class MeshTile :public dtMeshBase
	{
	public:
		MeshTile(void){m_tileMesh = 0;}
		virtual dtStatus getTileAndPolyByRef(const dtPolyRef &ref, const dtMeshTile** tile, const dtPoly** poly) const;
		virtual void getTileAndPolyByRefUnsafe(const dtPolyRef &ref, const dtMeshTile** tile, const dtPoly** poly) const;
		virtual bool isValidPolyRef(dtPolyRef &ref) const;
		virtual void calcTileLoc(const float* pos, int* tx, int* ty) const;
		virtual int getTilesAt(int x, int y,dtMeshTile ** tiles, int maxTiles) const;	
		
		friend dtMeshTileQuery;

	private:
		dtMeshTile * m_tileMesh;
	};

private:

	MeshTile						m_navMesh;
	dtNavMeshQuery					m_query;
	dtQueryFilter					m_filter;
	dtPolyRef						m_startRef,m_endRef;
	dtPolyRef						* m_pathRef;
};


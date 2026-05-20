
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "navdata.h"

#include "nav/detour/DetourCommon.h"
#include "nav/detour/DetourNavMeshQuery.h"

#include "rvo2/RvoObstacleMap.h"

#include <fstream>

using namespace RVO;


#define KEY_FROM_XY(x,y) (x<<16|(y&0xffff))
#define KEY_TO_XY(k,x,y)						\
(x)=(short)((k)>>16);											\
(y)=(short)((k)&0xffff);



dtStatus navMesh::getTileAndPolyByRef(const dtPolyRef& ref, const dtMeshTile** tile, const dtPoly** poly) const
{
	DWORD key = (DWORD)decodePolyIdTile(ref);

	int x,y;
	KEY_TO_XY(key,x,y);

	if (_isIn(x,y))
	{
		*tile=_getTile(x,y);

		int ip = decodePolyIdPoly(ref);
		
		if(ip<(*tile)->header->polyCount)
		{
			*poly = (*tile)->polys + ip;
			return DT_SUCCESS;
		}
	}

	return DT_FAILURE;
}

void navMesh::getTileAndPolyByRefUnsafe(const dtPolyRef& ref, const dtMeshTile** tile, const dtPoly** poly) const
{
	DWORD key = (DWORD)decodePolyIdTile(ref);

	DWORD x,y;
	KEY_TO_XY(key,x,y);

	*tile=_getTile(x,y);

	int ip = decodePolyIdPoly(ref);

	assert(ip<(*tile)->header->polyCount);
	*poly = (*tile)->polys + ip;
}

bool navMesh::isValidPolyRef(dtPolyRef &ref) const
{
	DWORD key = (DWORD)decodePolyIdTile(ref);

	DWORD x,y;
	KEY_TO_XY(key,x,y);

	if (!_isIn(x,y))
		return false;

	const dtMeshTile *tile=_getTile(x,y);

	int ip = decodePolyIdPoly(ref);
	
	if(ip>=tile->header->polyCount)
		return false;

	return true;
}

void navMesh::calcTileLoc(const float* pos, int* tx, int* ty) const
{
	*tx = int(floor(pos[0]/float(_tileLen)));
	*ty = int(floor(pos[2]/float(_tileLen)));
}

void navMesh::calcTileLoc(float x,float y, int* tx, int* ty) const
{
	*tx = int(floor(x/float(_tileLen)));
	*ty = int(floor(y/float(_tileLen)));
}


int navMesh::getTilesAt(int x, int y,dtMeshTile ** tiles, int maxTiles) const
{
	int numTile = 0;

	if (_isIn(x,y))
	{
		*tiles=(dtMeshTile *)_getTile(x,y);
		numTile = 1;
	}

	return numTile;
}

const RvoObstacleMap *navMesh::getRvoObstacleMap() const
{
	return _mpRvoObstacle;
}


void navMesh::init(int xTileStart,int yTileStart,int xTileEnd,int yTileEnd)
{
	_xTileStart=xTileStart;
	_yTileStart=yTileStart;
	_xTileEnd=xTileEnd;
	_yTileEnd=yTileEnd;
	_tiles.resize((_xTileEnd-_xTileStart)*(_yTileEnd-_yTileStart));
	memset(_tiles.data(),0,sizeof(_tiles[0])*_tiles.size());
}

void navMesh::_buildRvoObstacleMap(dtMeshTile *tile,dtNavMeshQuery &query, dtQueryFilter &filter)
{
	if (!tile)
		return;

	std::vector<RvoVec2> vertices;
	dtMeshHeader * header = tile->header;

	if (TRUE)
	{
		const int maxSegments=64;
		float verts[maxSegments*2*3];
		dtPolyRef base = getPolyRefBase(tile);
		for (int i = 0; i < tile->header->polyCount; ++i)
		{
			dtPolyRef refPoly=base | (dtPolyRef)i;

			int c;
			query.getPolyWallSegments(refPoly,&filter,verts,NULL,&c,maxSegments);

			for (int i=0;i<c;i++)
			{
				vertices.clear();
				float *v=&verts[i*6];
				vertices.push_back(RvoVec2(v[0],v[2]));
				v=&verts[i*6+3];
				vertices.push_back(RvoVec2(v[0],v[2]));
				_mpRvoObstacle->addObstacle(vertices,false);
			}
		}
	}
}


void navMesh::addTile(dtMeshTile * tile)
{
	int x = tile->header->x;
	int y = tile->header->y;

	if (_isIn(x,y))
	{
		_setTile(x,y,tile);
		addTileLink(tile);
	}
}


void navMesh::buildRvoObstacleMap(dtNavMeshQuery &query, dtQueryFilter &filter)
{
	if (TRUE)
	{
		_mpRvoObstacle=new RvoObstacleMap;

		i_math::recti rc;
		rc.set(_xTileStart,_yTileStart,_xTileEnd,_yTileEnd);
		rc*=_tileLen;

		_mpRvoObstacle->Create(rc);
	}


	for (int i=0;i<_tiles.size();i++)
		_buildRvoObstacleMap(_tiles[i],query,filter);

	_mpRvoObstacle->build(_poolRvoObstacleUnit,_bufWorking,_bufWorking2);

}


void navMesh::clear()
{
	for (int i=0;i<_tiles.size();i++)
	{
		if (_tiles[i])
			delete _tiles[i];
	}
	_tiles.clear();

	if (_mpRvoObstacle)
	{
		_mpRvoObstacle->clear();
		delete _mpRvoObstacle;
	}

	_poolRvoObstacleUnit.FreeAll();

	zero();
}

//////////////////////////////////////////////////////////////////////////
//navCells
void navCells::init(i_math::recti &rcTiles,float lenTile,float lenCell)
{
	hdr.rcTiles=rcTiles;
	hdr.lenCell=lenCell;
	hdr.wCellPerTile=(int)(lenTile/lenCell);
	hdr.xStart=lenTile*(float)rcTiles.Left();
	hdr.yStart=lenTile*(float)rcTiles.Top();
	hdr.lenTile=lenTile;

	tiles.resize(rcTiles.getArea());
	for (int i=0;i<tiles.size();i++)
	{
		tiles[i].cells.resize(hdr.wCellPerTile*hdr.wCellPerTile);
		VEC_SET(tiles[i].cells,0);
		tiles[i].polys.resize(2);
		VEC_SET(tiles[i].polys,0);
	}
}

navCellTile *navCells::getTile(int x,int y)
{
	if (!hdr.rcTiles.isPointInside(x,y))
		return NULL;

	return &tiles[(y-hdr.rcTiles.Top())*hdr.rcTiles.getWidth()+(x-hdr.rcTiles.Left())];
}



WORD*navCells::findPolys(float x,float y,int &count,short &xTile0,short &yTile0)
{
	count=0;
	x-=hdr.xStart;
	y-=hdr.yStart;

	if ((x<0.0f)||(y<0.0f))
		return 0;

	int xTile=(int)(x/hdr.lenTile);
	int yTile=(int)(y/hdr.lenTile);

	if ((xTile>=hdr.rcTiles.getWidth())||(yTile>=hdr.rcTiles.getHeight()))
		return 0;

	navCellTile *tile=&tiles[yTile*hdr.rcTiles.getWidth()+xTile];

	x=x-((float)xTile)*hdr.lenTile;
	y=y-((float)yTile)*hdr.lenTile;

	DWORD xCell,yCell;
	xCell=(DWORD)(x/hdr.lenCell);
	yCell=(DWORD)(y/hdr.lenCell);

	if (xCell>=hdr.wCellPerTile)
		xCell=hdr.wCellPerTile-1;
	if (yCell>=hdr.wCellPerTile)
		yCell=hdr.wCellPerTile-1;

	WORD idx=tile->cells[yCell*hdr.wCellPerTile+xCell];
	count=(DWORD)tile->polys[idx];

	xTile0=(short)(xTile+hdr.rcTiles.Left());
	yTile0=(short)(yTile+hdr.rcTiles.Top());

	return &tile->polys[idx+1];
}



//////////////////////////////////////////////////////////////////////////
void build(dtMeshTile *tile,unsigned char *data,int dataSize)
{
	dtMeshHeader* header = (dtMeshHeader*)data;
	if (header->magic != DT_NAVMESH_MAGIC)
		return;

	if (header->version != DT_NAVMESH_VERSION)
		return;

	const int headerSize = dtAlign4(sizeof(dtMeshHeader));
	const int vertsSize = dtAlign4(sizeof(float)*3*header->vertCount);
	const int polysSize = dtAlign4(sizeof(dtPoly)*header->polyCount);
	const int linksSize = dtAlign4(sizeof(dtLink)*(header->maxLinkCount));
	const int detailMeshesSize = dtAlign4(sizeof(dtPolyDetail)*header->detailMeshCount);
	const int detailVertsSize = dtAlign4(sizeof(float)*3*header->detailVertCount);
	const int detailTrisSize = dtAlign4(sizeof(unsigned char)*4*header->detailTriCount);
	const int bvtreeSize = dtAlign4(sizeof(dtBVNode)*header->bvNodeCount);
	const int offMeshLinksSize = dtAlign4(sizeof(dtOffMeshConnection)*header->offMeshConCount);

	unsigned char* d = data + headerSize;
	tile->verts = (float*)d; d += vertsSize;
	tile->polys = (dtPoly*)d; d += polysSize;
 	/*tile->links = (dtLink*)d;*/ d += linksSize;
	tile->detailMeshes = (dtPolyDetail*)d; d += detailMeshesSize;
	tile->detailVerts = (float*)d; d += detailVertsSize;
	tile->detailTris = (unsigned char*)d; d += detailTrisSize;
	tile->bvTree = (dtBVNode*)d; d += bvtreeSize;
	tile->offMeshCons = (dtOffMeshConnection*)d; d += offMeshLinksSize;
	
	tile->m_linkAlloc = NULL;
	tile->m_hasLink = false;

	// If there are no items in the bvtree, reset the tree pointer.
	if (!bvtreeSize)
		tile->bvTree = 0;
	
	tile->header = header;
	tile->data = data;
	tile->dataSize = dataSize;
	tile->flags = 0/*flags*/;
	
	//
	for(int i = 0; i<header->polyCount; ++i)
	{
		dtPoly * poly = tile->polys + i;
		poly->firstLink = DT_NULL_LINK;
	}
}

bool navData::load(streamRead &stream)
{
	navMeshFileHeader header;

	stream.read((char *)&header,sizeof(header));

	//navMesh
	nmesh.init(header.xTileStart,header.yTileStart,header.xTileEnd,header.yTileEnd);
	nmesh.setTileLen(header.tileLen);

	int szData = 0;
	dataNMesh.resize(header.szNavMeshData);
	unsigned char *p = dataNMesh.data();

	int number=(header.yTileEnd-header.yTileStart)*(header.xTileEnd-header.xTileStart);
	for(int i = 0;i<number;++i)
	{
		stream.read((char*)&szData,sizeof(szData));
		if (szData>0)
		{
			dtMeshTile * tile = new dtMeshTile;
			stream.read((char *)p,szData);
			build(tile,p,szData);
			p += szData;

			tile->m_linkAlloc = &linkAlloc;
			tile->m_hasLink = true;
			nmesh.addTile(tile);
		}
	}

	if (TRUE)
	{
		dtNavMeshQuery query;
		query.init(&nmesh,10);

		dtQueryFilter filter;
		filter.m_tp=dtQueryFilter::FindWalkable;

		nmesh.buildRvoObstacleMap(query,filter);
	}

	assert(p-(dataNMesh.data())==header.szNavMeshData);

	//routing data
	std::vector<unsigned char> buf(header.szRoutingData);
	stream.read((char *)buf.data(),header.szRoutingData);
	CDataPacket dp;
	dp.SetDataBufferPointer(buf.data());
	world.load(dp);

	//cells data
	if (TRUE)
	{
		stream.read((char*)&ncells.hdr,sizeof(ncells.hdr));

		ncells.tiles.resize(ncells.hdr.rcTiles.getArea());

		for (int i=0;i<ncells.tiles.size();i++)
		{
			navCellTile *tile=&ncells.tiles[i];
			DWORD sz;

			stream.read((char*)&sz,sizeof(sz));
			tile->polys.resize(sz);
			if (sz>0)
				stream.read((char*)&tile->polys[0],sz*sizeof(WORD));
			stream.read((char*)&sz,sizeof(sz));
			tile->cells.resize(sz);
			if (sz>0)
				stream.read((char*)&tile->cells[0],sz*sizeof(WORD));
		}
	}

	return true;

}

struct streamRead_ifs:public navData::streamRead
{
	std::ifstream ifs;
	virtual void read(char *buf,unsigned int sz)
	{
		ifs.read(buf,sz);
	}
};


bool navData::load(const char *path)
{
	streamRead_ifs stream;
 	stream.ifs.open(path,std::ios_base::in|std::ios_base::binary);
	if(!stream.ifs.is_open())
		return false;

	load(stream);

	stream.ifs.close();
	return true;
}


void navData::clear()
{
	nmesh.clear();
	world.clear();

	ncells.clear();

	linkAlloc.clear();

	dataNMesh.clear();
}



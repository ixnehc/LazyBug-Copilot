
#pragma once

#include "detour/DetourNavMesh.h"

#include "routing/rtWorld.h"

#include "rvo2/RvoObstacleMap.h"

#include <map>

struct navMeshFileHeader
{
	//NavMesh
	int xTileStart;
	int yTileStart;
	int xTileEnd;
	int yTileEnd;
	int tileLen;//每个Tile的宽/高,以米为单位
	int szNavMeshData;//所有的tile一共有多少数据

	//Routing
	int szRoutingData;
};

class RvoObstacleMap;
class dtNavMeshQuery;
class dtQueryFilter;
struct navMesh :public dtMeshBase
{
public:
	navMesh()
	{
		zero();
	}

	void zero()
	{
		_tileLen=0;
		_xTileStart=0;
		_yTileStart=0;
		_xTileEnd=0;
		_yTileEnd=0;
		_mpRvoObstacle=NULL;
	}
	virtual dtStatus getTileAndPolyByRef(const dtPolyRef& ref, const dtMeshTile** tile, const dtPoly** poly) const;
	virtual void getTileAndPolyByRefUnsafe(const dtPolyRef& ref, const dtMeshTile** tile, const dtPoly** poly) const;
	virtual bool isValidPolyRef(dtPolyRef& ref) const;
	virtual void calcTileLoc(const float* pos, int* tx, int* ty) const;
	virtual void calcTileLoc(float x,float y, int* tx, int* ty) const;
	virtual int getTilesAt(int x, int y,dtMeshTile ** tiles, int maxTiles) const;
	const RvoObstacleMap *getRvoObstacleMap() const;
	void init(int xTileStart,int yTileStart,int xTileEnd,int yTileEnd);

	//以下四个函数的返回值都以tile为单位
	int getLeft()	{		return _xTileStart;	}
	int getRight()	{		return _xTileEnd;	}
	int getTop()	{		return _yTileStart;	}
	int getBottom()	{		return _yTileEnd;	}

	void addTile(dtMeshTile * tile);
	void setTileLen(int len){_tileLen = len;}
	int getTileLen()	{		return _tileLen;	}

	void buildRvoObstacleMap(dtNavMeshQuery &query, dtQueryFilter &filter);

	void  clear();
private:

	bool _isIn(int x,int y) const
	{
		if ((x>=_xTileStart)&&(x<_xTileEnd)&&
			(y>=_yTileStart)&&(y<_yTileEnd))
			return true;

		return false;
	}
	const dtMeshTile *_getTile(int x,int y) const
	{
		int idx=(y-_yTileStart)*(_xTileEnd-_xTileStart)+(x-_xTileStart);
		return _tiles[idx];
	}
	void _setTile(int x,int y,dtMeshTile * tile)
	{
		int idx=(y-_yTileStart)*(_xTileEnd-_xTileStart)+(x-_xTileStart);
		_tiles[idx]=tile;
	}

	void _buildRvoObstacleMap(dtMeshTile *tile,dtNavMeshQuery &query, dtQueryFilter &filter);
	int _tileLen;
	int _xTileStart;
	int _yTileStart;
	int _xTileEnd;
	int _yTileEnd;
	std::vector<dtMeshTile *> _tiles;

	RvoObstacleMap *_mpRvoObstacle;
	CClassPool<RvoObstacleUnit> _poolRvoObstacleUnit;

	std::vector<i_math::pos2di> _bufWorking;
	std::vector<RvoObstacle *> _bufWorking2;
};

//一个CellTile对应于一个MeshTile
struct navCellTile
{
	std::vector<WORD>polys;
	std::vector<WORD>cells;

};

struct navCells
{
public:
	struct Header
	{
		Header()
		{
			Zero();
		}
		void Zero()
		{
			memset(this,0,sizeof(*this));
		}
		i_math::recti rcTiles;//以Tile为单位
		float xStart;//以米为单位
		float yStart;//以米为单位
		float lenCell;//单个cell的大小,以米为单位
		float lenTile;//单个tile的大小,以米为单位
		int wCellPerTile;//一个tile有wCellPerTile*wCellPerTile个cell
	};

	void init(i_math::recti &rcTiles,float lenTile,float lenCell);

	void clear()
	{
		hdr.Zero();
		tiles.clear();
	}

	navCellTile *getTile(int x,int y);//x,y为世界坐标系,以tile为单位

	
	WORD *findPolys(float x,float y,int &count,short &xTile,short &yTile);//返回poly的索引,

	Header hdr;
	std::vector<navCellTile> tiles;
};


struct navData
{
public:
	struct streamRead
	{
		virtual void read(char *buf,unsigned int sz)=0;
	};
	bool load(streamRead &stream);
	bool load(const char *path);
	void clear();
	rtWorld *getWorld(){return	&world;}
	dtMeshBase * getNavMesh(){return	&nmesh;}
	navCells *getCells()	{		return &ncells;	}

	rtWorld		world;
	navMesh		nmesh;
	navCells		ncells;
	dtLinkAlloc linkAlloc;
	std::vector<unsigned char> dataNMesh;
};

void build(dtMeshTile *tile,unsigned char *data,int dataSize);



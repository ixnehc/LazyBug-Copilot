
#pragma once

#include "class/class.h"

#include "unitsystem/unitsystem.h"

#define GAMEHEIGHT_LOW (-32)
#define GAMEHEIGHT_HIGH (32)

struct GameTrisTile
{
	DWORD tris;
	DWORD nTris;
};

struct GameTri
{
	i_math::triangle3df tri;
	DWORD flags;
};

class CGameTrisMap
{
public:
	struct Header
	{
		i_math::recti rcTiles;//以Tile为单位
		float lenTile;//单个tile的大小,以米为单位
	};

	CGameTrisMap()
	{
		Zero();
	}

	void Zero()
	{
		memset(&_hdr,0,sizeof(_hdr));
	}

	void Clear()
	{
		_tris.clear();
		_tiles.clear();
		_indices.clear();
		Zero();
	}

	float GetHeight(float x,float y);

	//Return if any hit
	BOOL RayCheck(i_math::vector3df &posSrc,i_math::vector3df &posTarget,i_math::vector3df &posHit);

public:
	Header _hdr;
	std::vector<GameTrisTile> _tiles;

	std::vector<GameTri> _tris;
	std::vector<DWORD> _indices;

	std::vector<i_math::pos2di> _temp;
};
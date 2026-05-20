
#pragma once

#include "class/class.h"

#include "unitsystem/unitsystem.h"

#define GAMEHEIGHT_LOW (-32)
#define GAMEHEIGHT_HIGH (32)

struct GameTile
{
	WORD ht:13;//VUS13 Unit System
	WORD bAbyss:1;
	WORD bWalkable:1;
	WORD bEnum:1;
};

class GameTileMap
{
public:
	struct Header
	{
		i_math::recti rcTiles;//以Tile为单位
		float lenTile;//单个tile的大小,以米为单位
	};

	GameTileMap()
	{
		Zero();
	}

	void Zero()
	{
		memset(&hdr,0,sizeof(hdr));
	}

	void Clear()
	{
		data.clear();
		Zero();
	}

	float GetHeight(float x,float y)//x,y以米为单位
	{
		int xTile=(int)floor(x/hdr.lenTile);
		int yTile=(int)floor(y/hdr.lenTile);
		if (!hdr.rcTiles.isPointInside(xTile,yTile))
			return 0.0f;
		GameTile *tile=&data[(yTile-hdr.rcTiles.Top())*hdr.rcTiles.getWidth()+xTile-hdr.rcTiles.Left()];
		return VUS13_MtrFromVu(tile->ht);
	}

	void Clamp(i_math::vector3df &pos)
	{
		Clamp(pos.x,pos.z,pos.y);
	}

	void Clamp(float x,float z,float &y)
	{
		GameTile *tile=GetTile(x,z);
		if (tile)
		{
			float ht=VUS13_MtrFromVu(tile->ht);
			if ((!tile->bWalkable)&&(!tile->bAbyss))
				ht+=1.0f;
			if (y<ht)
				y=ht;
		}
	}

	BOOL IsWalkable(float x,float y)
	{
		int xTile=(int)floor(x/hdr.lenTile);
		int yTile=(int)floor(y/hdr.lenTile);
		if (!hdr.rcTiles.isPointInside(xTile,yTile))
			return FALSE;
		GameTile *tile=&data[(yTile-hdr.rcTiles.Top())*hdr.rcTiles.getWidth()+xTile-hdr.rcTiles.Left()];
		return tile->bWalkable;
	}

	GameTile *GetTileAt(int xTile,int yTile)
	{
		if (!hdr.rcTiles.isPointInside(xTile,yTile))
			return NULL;
		return &data[(yTile-hdr.rcTiles.Top())*hdr.rcTiles.getWidth()+xTile-hdr.rcTiles.Left()];
	}

	GameTile *GetTile(float x,float y)
	{
		int xTile=(int)floor(x/hdr.lenTile);
		int yTile=(int)floor(y/hdr.lenTile);

		return GetTileAt(xTile,yTile);
	}


	Header hdr;
	std::vector<GameTile> data;
};
/********************************************************************
	created:	2011/9/15   14:25
	file path:	e:\IxEngine\Interfaces\WorldSystem
	author:		chenxi
	
	purpose:	a minimap baker
*********************************************************************/
#pragma once

#include "datapacket/DataPacket.h"
#include "gamedata/GameTileMap.h"

#define GTIBLOCK_LEN 64//以米为单位

#define GTI_RESO 2 //1米有GTI_RESO*GTI_RESO个像素

struct GtiBlock
{
	std::vector<GameTile>data;

	void Save(CDataPacket &dp)
	{
		DP_WriteVector(dp,data);
	}
	void Load(CDataPacket &dp)
	{
		DP_ReadVector(dp,data);
	}

};


class IGtiBaker
{
public:
	INTERFACE_REFCOUNT;

	virtual BOOL Bake(i_math::recti &rc)=0;//rc单位为米

};


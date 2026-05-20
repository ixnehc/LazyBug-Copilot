/********************************************************************
	created:	2008/3/31   13:52
	file path:	d:\IxEngine\Interfaces\FileSystem
	author:		cxi
	
	purpose:	map file defines
*********************************************************************/

#pragma once

#include "math/imath_all.h"


//////////////////////////////////////////////////////////////////////////
//MapFile
#define BLOCK_LENGTH 4 //in meter


#define MAP_SIZE_GRAN 64 //以米为单位,表示地图的粒度(地图Rect的四个顶点必须在这个值的倍数上)

//MapChannels could be devided into 2 caterories by the method it's compressed: 
//image channel and none-image channel
//for none-image channel,a general compression algorythm will be used
//for image channel,jpeg format compression will be used
enum MapChannel
{
	MapChannel_TrrnBase=0,
	MapChannel_TrrnHLB,//地表的LargeBlock高度信息
	MapChannel_TrrnPLB0,//地表的LargeBlock Pixel信息(lod0)
	MapChannel_TrrnPLB1,//地表的LargeBlock Pixel信息(lod1)
	MapChannel_TrrnPLB2,//地表的LargeBlock Pixel信息(lod2)
	MapChannel_Entity,
	MapChannel_SptForest,
	MapChannel_Water,
	MapChannel_Grass,
	MapChannel_EnvLight,
	MapChannel_Baffles,
	MapChannel_ETProbe,
	MapChannel_Shore,
	MapChannel_Ridge,
	MapChannel_Road,
	MapChannel_NavMesh,
	MapChannel_Tris,//Triangles
	MapChannel_GameRgn,
	MapChannel_RawMiniMap,
	MapChannel_OutlineMap,
	MapChannel_Gti,//Game Tile Info
	MapChannel_MiniMap,
	MapChannel_Max,
};



struct MapBlockData
{
	i_math::pos2di ptBlk;
	int indices[MapChannel_Max];
	int sizes[MapChannel_Max];
	std::vector<BYTE>data;

	void Clear()
	{
		ptBlk.set(0,0);
		memset(indices,0xff,sizeof(indices));
		memset(sizes,0,sizeof(sizes));
		data.clear();
	}


	MapBlockData &operator=(const MapBlockData&src)
	{
		ptBlk=src.ptBlk;
		memcpy(indices,src.indices,sizeof(indices));
		memcpy(sizes,src.sizes,sizeof(sizes));
		data=src.data;
		return *this;
	}
};

typedef DWORD HMapFileCache;
#define HMapFileCache_Null 0


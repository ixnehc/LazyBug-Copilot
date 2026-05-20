/********************************************************************
	created:	2011/9/15   14:25
	file path:	e:\IxEngine\Interfaces\WorldSystem
	author:		chenxi
	
	purpose:	a minimap baker
*********************************************************************/
#pragma once

#include "datapacket/DataPacket.h"

#define OUTLINEMAPBLOCK_LEN 64//以米为单位

#define OUTLINEMAP_RESO 2 //1米有OUTLINEMAP_RESO*OUTLINEMAP_RESO个像素

struct OutlineMapBlock
{
	std::vector<BYTE>dataRgn;
	std::vector<BYTE>dataOutline;


	void Save(CDataPacket &dp)
	{
		DP_WriteVector(dp,dataRgn);
		DP_WriteVector(dp,dataOutline);
	}
	void Load(CDataPacket &dp)
	{
		DP_ReadVector(dp,dataRgn);
		DP_ReadVector(dp,dataOutline);
	}

};


struct NavMeshBuildParams;
class IOutlineMapBaker
{
public:
	INTERFACE_REFCOUNT;

	virtual BOOL Bake(i_math::recti &rc,NavMeshBuildParams *params)=0;//rc单位为米

};


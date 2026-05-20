/********************************************************************
	created:	2011/9/15   14:25
	file path:	e:\IxEngine\Interfaces\WorldSystem
	author:		chenxi
	
	purpose:	a minimap baker
*********************************************************************/
#pragma once

#include "datapacket/DataPacket.h"

#define MINIMAPBLOCK_LEN 64
#define MINIMAP_RESO 4

struct MiniMapBlock
{
	WORD len;
	std::vector<DWORD>data;

	void Save(CDataPacket &dp)
	{
		dp.Data_NextWord()=1;
		dp.Data_NextWord()=len;
		DP_WriteVector(dp,data);
	}
	void Load(CDataPacket &dp)
	{
		dp.Data_NextWord();
		len=dp.Data_NextWord();
		DP_ReadVector(dp,data);
	}

};

class ITexture;
class IMiniMapBaker
{
public:
	INTERFACE_REFCOUNT;

	virtual BOOL BakeRaw(i_math::recti &rc,float reso)=0;//rc单位为米,reso为每米有多少个像素(比如reso=4,则每平方米有4x4个像素),必须为2的次方,最小1/16(2的-4次方)
	virtual ITexture* BakeWhole(int reso,i_math::vector3df &dirCam)=0;
	virtual BOOL ImportFromData(DWORD *data,int w,int h)=0;//

};


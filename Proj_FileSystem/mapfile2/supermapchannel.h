#pragma once

#include "FileSystem/IMapFileDefines.h"

#define DEFINE_MAPCHANNEL_REF(ch,sch_,x,y) refs[ch].Set(sch_,x,y)

struct SuperMapChannels
{
	SuperMapChannels()
	{
		Zero();
	}
	enum 
	{
		TrrnBase=0,
		Channelx2,
		Channelx4,
		Channelx8,
		Channelx16,
		Channelx64,

		SuperMapChannel_Max,
	};

	struct ChannelRefInfo
	{
		void Set(int sch_,int x,int y)
		{
			sch=sch_;
			pt.set(x,y);
		}
		int sch;
		i_math::pos2di pt;
	};

	void Zero()
	{

		lens[TrrnBase]=1;
		lens[Channelx2]=2;
		lens[Channelx4]=4;
		lens[Channelx8]=8;
		lens[Channelx16]=16;
		lens[Channelx64]=64;

		DEFINE_MAPCHANNEL_REF(MapChannel_TrrnBase,TrrnBase,0,0);
		DEFINE_MAPCHANNEL_REF(MapChannel_TrrnHLB,Channelx8,0,0);
		DEFINE_MAPCHANNEL_REF(MapChannel_TrrnPLB0,Channelx8,1,0);
		DEFINE_MAPCHANNEL_REF(MapChannel_TrrnPLB1,Channelx8,2,0);
		DEFINE_MAPCHANNEL_REF(MapChannel_TrrnPLB2,Channelx8,3,0);
//		DEFINE_MAPCHANNEL_REF(MapChannel_EnvLight,Channelx4,0,0);
		DEFINE_MAPCHANNEL_REF(MapChannel_EnvLight,Channelx8,4,0);
		DEFINE_MAPCHANNEL_REF(MapChannel_Grass,Channelx4,1,1); 
		DEFINE_MAPCHANNEL_REF(MapChannel_Entity,Channelx2,0,0);
		DEFINE_MAPCHANNEL_REF(MapChannel_SptForest,Channelx2,0,1);
		DEFINE_MAPCHANNEL_REF(MapChannel_Water,Channelx16,0,4);
		DEFINE_MAPCHANNEL_REF(MapChannel_Baffles,Channelx16,0,5);
		DEFINE_MAPCHANNEL_REF(MapChannel_ETProbe,Channelx16,0,2);
		DEFINE_MAPCHANNEL_REF(MapChannel_Shore,Channelx16,0,3);
		DEFINE_MAPCHANNEL_REF(MapChannel_Ridge,Channelx16,0,6);
		DEFINE_MAPCHANNEL_REF(MapChannel_Road,Channelx16,1,0);
		DEFINE_MAPCHANNEL_REF(MapChannel_NavMesh,Channelx16,1,1);
		DEFINE_MAPCHANNEL_REF(MapChannel_Tris,Channelx16,1,2);
		DEFINE_MAPCHANNEL_REF(MapChannel_GameRgn,Channelx64,0,0);
		DEFINE_MAPCHANNEL_REF(MapChannel_RawMiniMap,Channelx16,0,7);
		DEFINE_MAPCHANNEL_REF(MapChannel_OutlineMap,Channelx16,0,9);
		DEFINE_MAPCHANNEL_REF(MapChannel_Gti,Channelx16,0,10);
		DEFINE_MAPCHANNEL_REF(MapChannel_MiniMap,Channelx16,0,11);
	}

	void Resolve(MapChannel ch,i_math::pos2di &ptBlk)
	{
		DWORD len=lens[refs[ch].sch];
		if (len>1)
		{
			ptBlk.scale_signed(len);
			ptBlk*=len;
			ptBlk+=refs[ch].pt;
		}
	}
	int GetSuper(MapChannel ch)
	{
		return refs[ch].sch;
	}

	DWORD GetLen(MapChannel ch)
	{
		return lens[refs[ch].sch];
	}

	DWORD lens[SuperMapChannel_Max];
	ChannelRefInfo refs[MapChannel_Max];

};


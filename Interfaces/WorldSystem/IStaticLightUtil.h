#pragma once

#include "math/range.h"

#include "class/class.h"



#include "WorldSystem/IBake.h"

#include "datapacket/DataPacket.h"

#include "RenderSystem/ITexture.h"


#include "fvfex/fvfex_type.h"
#include "shaderlib/SLDefines.h"

enum SldShdwType
{
	Sld_NoShdw,//不在阴影下
	Sld_PartialShdw,//部分在阴影下
	Sld_FullShdw,//全在阴影下
};

enum SldLocalType
{
	Sld_NoLocal,
	Sld_UniformLocal,
	Sld_Local,
};


#define STATICLIGHTDATA_VER 1
struct StaticLightData
{
public:
	struct Header
	{
		DWORD bValid:1;
		DWORD tpShdw:2;
		DWORD bGlobal:1;
		DWORD tpLocal:2;
		DWORD bDot3:1;
		//w/h表示LightData的数据量,但并不表示shdw数据的数据量,它们的数据量是可能不同的
		//shdw的数据量保存在dataShdw里面
		DWORD w:12;
		DWORD h:12;

		i_math::rangef range;//data里的数据归一化后,它们的取值范围记录在range中

	};

	DEFINE_CLASS(StaticLightData);
	StaticLightData(){ 	Zero();	}
	void Zero(){	memset(&header,0,sizeof(header));}
	void Clean()
	{
		data.clear();
		Zero();
	}

	void Save(CDataPacket &dp)
	{
		dp.Data_NextByte()=STATICLIGHTDATA_VER;

		DP_PreSafeSave(dp);
		DP_WriteVar(dp,header);
		if (header.tpLocal==Sld_UniformLocal)
			DP_WriteVar(dp,colUL);
		DP_WriteVector(dp,data);
		if (header.tpShdw==Sld_PartialShdw)
			DP_WriteVector(dp,dataShdw);
		DP_PostSafeSave();
	}
	void Load(CDataPacket &dp)
	{
		BYTE ver=dp.Data_NextByte();
		DP_PreSafeLoad(dp);

		if (ver==STATICLIGHTDATA_VER)
		{
			DP_ReadVar(dp,header);
			if (header.tpLocal==Sld_UniformLocal)
				DP_ReadVar(dp,colUL);
			DP_ReadVector(dp,data);
			if (header.tpShdw==Sld_PartialShdw)
				DP_ReadVector(dp,dataShdw);
		}

		DP_PostSafeLoad();
	}
	BOOL IsValid()	{		return header.bValid;	}
	
	StaticLightData & operator = (const StaticLightData &oth)
	{
		header = oth.header;
		colUL = oth.colUL;
		data = oth.data;
		dataShdw = oth.dataShdw;
		return *this;
	}

	StaticLightData(const StaticLightData &oth)
	{
		*this = oth;
	}

	Header header;
	i_math::vector3df colUL;//只在local为Uniform时有效
	std::vector<BYTE> data;//标准的static light data数据
	std::vector<BYTE> dataShdw;//经过压缩的shdw数据,只在shdw的类型为Sld_PartialShdw时有效
};



struct LightMap
{
	DEFINE_CLASS(LightMap);
	LightMap()
	{
		Zero();
	}

	~LightMap()
	{
		Clear();
	}

	void Zero()
	{
		memset(texes,NULL,sizeof(texes));
		nTexes=0;
		texShdw=NULL;
	}

	void Clear()
	{
		for (int i=0;i<nTexes;i++)
			SAFE_RELEASE(texes[i]);
		SAFE_RELEASE(texShdw);
		Zero();
	}
	BOOL IsEmpty() const
	{ 
		return ((nTexes==0)&&(texShdw==NULL));
	}
	FeatureCode fc;
	ITexture *texes[3];
	ITexture *texShdw;
	DWORD nTexes:4;
	i_math::vector3df colUL;
	i_math::vector2df scale;
};

struct StaticLightDataTrrn
{
	BYTE *gl;
	SldShdwType typeShdw;
	DWORD szShdw;
	BYTE *shdw;
};


class IStaticLightUtil
{
public:

	//resultsShdw为额外的,用来生成Global Shadow Lightmap的信息,如果传入NULL,
	//则直接使用results来生成Global Shadow  Lightmap
	virtual BOOL Build(StaticLightData &sld,BakeResult *results,DWORD w,DWORD h,BOOL bCompress,
											BakeResult *resultsShdw,DWORD wShdw,DWORD hShdw)=0;

	//注意:sldTrrn内返回的指针为暂时指针
	virtual BOOL BuildForTrrn(StaticLightDataTrrn&sldTrrn,BakeResult *results,DWORD len,BakeResult *resultsShdw,DWORD lenShdw)=0;

	// create d3d resource
	virtual BOOL Create(StaticLightData  &data,LightMap&lmp) = 0;
};




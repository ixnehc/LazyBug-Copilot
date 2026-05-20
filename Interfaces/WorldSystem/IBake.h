/********************************************************************
	created:	2009/02/23
	created:	23:2:2009   21:00
	filename: 	d:\IxEngine\Interfaces\WorldSystem\IBakeDefines.h
	file path:	d:\IxEngine\Interfaces\WorldSystem
	file base:	IBakeDefines
	file ext:	h
	author:		cxi
	
	purpose:	exposed baking related defines
*********************************************************************/
#pragma once

#include "math/vector3d.h"
#include "math/vector2d.h"
#include "math/aabbox3d.h"

#include "fvfex/fvfex_type.h"

class IFileSystem;
class IRenderSystem;
class IUtilRS;


enum LightMapChannel
{

	LMC_GlobalAmbient=0,
	LMC_GlobalSun,
	LMC_Local,

	LMC_Max,
};



//in block
#define BAKEUNIT_LEN 16

#define ENERGY_ToUnit(ev) ((unsigned short)(i_math::clamp_u((DWORD)((ev)*1024.0f),0,65535)))
#define ENERGY_FromUnit(eu) (((float)(eu))/1024.0f)

#define OCC_ToUnit(ov) ((unsigned char)(i_math::clamp_u((DWORD)((ov)*255.0f),0,255)))
#define OCC_FromUnit(ou) (((float)(ou))/255.0f)

typedef DWORD HBakeMap;
#define HBAKEMAP_NULL 0

enum BakeMapFlag
{
	BM_None=0,
	BM_Blur=2,//need blurring the results
	BM_ShrinkGutter=4,//need shrink gutter for the results
};

enum BakePass
{
	Pass_Direct=1,//直接光照Pass
	Pass_Indirect=2,//间接光照Pass
	Pass_Shadow=4,//shadow pass
};

#define BAKE_ALBEDO_BASE (0.10f)

//ETP代表Energy Transfer Patch
typedef DWORD ETPHandle;
#define ETPHandle_Invalid (0xffffffff)

struct BakeSample
{
	BakeSample()
	{
		bDot3=0;
		bEnum=0;
		passes=Pass_Direct;
		bVPL=0;
		bm=HBAKEMAP_NULL;
		x=y=0;
		area=0;
	}

	i_math::vector3df pos;
	i_math::vector3df nml;
	i_math::vector3df basis[3];

	i_math::vector3df albedo;//对能量的反射率,只在bVPL为1时有效

	BYTE bDot3:1;//是不是一个dot3的采样点,
	BYTE bEnum:1;//一个通用的枚举标志位
	BYTE bVPL:1;//这个采样点可以作为一个vpl(virtual point light),也就是可以反射能量
	BakePass passes;//表示这个sample可以参与的Pass类型的组合

	float area;//面积,在bVPL为1时,需要填
	
	//Host Info
	HBakeMap bm;
	DWORD x,y;
};

#define MAX_BAKE_PASS 2
struct BakeResult
{
	BakeResult()
	{
		Zero();
	}
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	BakeResult operator+(const BakeResult& other) const 
	{ 
		BakeResult t;
		DWORD c=3;
		if (bValid)
		{
			for (int i=0;i<ARRAY_SIZE(chs);i++)
			{
				for (int j=0;j<c;j++)
					t.chs[i].eFull[j]+=chs[i].eFull[j];
			}
			t.shdw+=shdw;
			t.bValid=1;
		}
		if (other.bValid)
		{
			for (int i=0;i<ARRAY_SIZE(chs);i++)
			{
				for (int j=0;j<c;j++)
					t.chs[i].eFull[j]+=other.chs[i].eFull[j];
			}
			t.shdw+=other.shdw;
			t.bValid=1;
		}
		if (bDot3&&bValid)
			t.bDot3=1;
		if (other.bValid&&other.bDot3)
			t.bDot3=1;
		return t;	
	}
	BakeResult operator*(float other) const 
	{ 
		BakeResult t;
		DWORD c=3;
		t=*this;
		for (int i=0;i<ARRAY_SIZE(chs);i++)
		{
			for (int j=0;j<c;j++)
				t.chs[i].eFull[j]*=other;
		}
		t.shdw*=other;

		return t;
	}
	BakeResult operator/(float other) const 
	{ 
		return (*this)*(1.0f/other);
	}

	struct Channel
	{
		i_math::vector3df eFull[3];
	};

	i_math::vector3df GetAverageEnergy(LightMapChannel ch)
	{
		i_math::vector3df e;
		if (bValid)
		{
			Channel *p=&chs[ch];
			if (bDot3)
				e=(p->eFull[0]+p->eFull[1]+p->eFull[2])/1.732f;
			else
				e=p->eFull[0];
		}
		return e;
	}

	Channel chs[LMC_Max];
	float shdw;//这个值代表对LMC_GlobalSun中的直线光的阴影mask值,1表示全遮住,0表示没遮住
	BYTE bShdwBack:1;//这个值表示这个采样点与LMC_GlobalSun中的直线光方向相反
	BYTE bValid:2;//whether this result is valid
	BYTE bDot3:1;
};

// inline float MakeGlobalLightValue(BakeResult *r)
// {
// 	return r->chs[LMC_GlobalAmbient].eFull.x + r->chs[LMC_GlobalSun].eFull.x;
// }

//计算dot3 的global light value的公式:
//某个basis方向上受到的全局光能量值为:
//
//法线方向的环境光能量值除以sqrtf(3) 
//				+
//basis方向的直线光能量值
// inline float MakeDot3GlobalLightValue(BakeResult *rBasis,BakeResult * rNormal)
// {
// 	return rNormal->chs[LMC_GlobalAmbient].eFull.x/1.7320f + rBasis->chs[LMC_GlobalSun].eFull.x;
// }



struct BakeLight
{
	BakeLight()
	{
		tp=Ambient;
		ch=LMC_GlobalSun;
	}
	enum Type
	{
		Directional,
		Point,
		Ambient,
		Spot,//Not support yet
		VPL,//Virtual Point Light(反射光源)
	};
	Type tp;
	LightMapChannel ch;
	i_math::vector3df energy;
	i_math::vector3df dir;//for Directional & Spot &Reflect
	i_math::vector3df pos;//for Point &Spot
	float range;//for Point & Spot
};



#define FVFEX_BAKEVTX (FVFEX_XYZ0)
struct BakeVtx
{
	i_math::vector3df pos;
};

#define FVFEX_BAKEVTX_UV (FVFEX_XYZ0|FVFEX_FLAG_TEX0)
struct BakeVtxUV
{
	i_math::vector3df pos;
	i_math::texcoordf uv;
};

//ET代表Energy Transfer,注意我们使用三维贴图坐标来表示能量值
#define FVFEX_BAKEVTX_NE (FVFEX_XYZ0|FVFEX_NORMAL0|FVFEX_FLAG_VOX0)
struct BakeVtxET
{
	i_math::vector3df pos;
	i_math::vector3df normal;
	i_math::vector3df energy;
};

class IMtrl;
class ITexture;
class IMesh;
template<class TVtx>
struct BakePatch_T
{
	BakePatch_T()
	{
		mtrl=NULL;
		texDif=NULL;
		bTrrn=0;
	}
	std::vector<TVtx> vertices;
	std::vector<WORD> indices;
	IMtrl *mtrl;//this is optional,used for those triangles with alpha-test texture,
				//注意,给这个指针赋值需要添加引用计数
	ITexture *texDif;//diffuse map的texture,这张贴图是可选的,可以为NULL。注意,给这个指针赋值需要添加引用计数
	i_math::aabbox3df aabb;
	DWORD bTrrn:1;//表示这个Patch是否来自Terrain
};

struct BakePatchSpt
{
	std::vector<i_math::vector3df> vertices;
	std::vector<float> scale;
	std::vector<float> rotY;
	std::string  nameSptRes;
};

struct BakePatchMesh
{
	BakePatchMesh()
	{
		msh=NULL;
		mtrl=NULL;
	}
	IMesh *msh;
	IMtrl *mtrl;
	i_math::matrix43f mat;
	i_math::aabbox3df aabb;
};

typedef BakePatch_T<BakeVtx> BakePatch;
typedef BakePatch_T<BakeVtxUV> BakePatchTex;

enum BakeQuality
{
	BQ_Low,
	BQ_Medium,
	BQ_Hi,
};


class CProgress;
struct TriSample;
struct VBPatch;
class ISpt;
class ISceneBaker
{
public:
	INTERFACE_REFCOUNT;

	virtual BOOL Bake(i_math::recti &rcBound,BakeQuality quality)=0;//rcBound is in block

	//szMap用来传入sample map的大小,并且在里面会返回实际的大小
	virtual TriSample*ObtainMeshSamples(const char *mesh,i_math::size2di &szMap,DWORD &nSamples)=0;

	virtual BOOL ObtainSptSamples(ISpt * pSpt,int lvl,TriSample **pSamples,DWORD *nSamples,i_math::size2di *szSpt) =0;

	virtual VBPatch ObtainMeshPatch(const char *mesh,i_math::matrix43f &mat,FVFEx fvf)=0;

	virtual void SetProgress(CProgress *prgMain,CProgress *prgSub)=0;

	virtual HBakeMap NewBakeMap(DWORD w,DWORD h,BakeMapFlag flag)=0;

	virtual BOOL AddSample(BakeSample &s)=0;
	virtual BOOL AddSamples(BakeSample *s,DWORD count)=0;
	virtual BakePatch *NewPatch()=0;
	virtual BakePatchTex *NewTexPatch()=0;
	virtual BakePatchMesh *NewMeshPatch()=0;
	virtual BakePatchSpt *NewSptPatch() = 0;

	virtual BOOL AddLight(BakeLight &lgt)=0;
	virtual BOOL GetResults(HBakeMap bm,BakeResult *results)=0;
	virtual BOOL GetMapSize(HBakeMap bm,DWORD &w,DWORD &h)=0;

	virtual DWORD GetDumpMapCount()=0;
	virtual DWORD *GetDumpMap(DWORD idx,DWORD &w,DWORD &h)=0;

};

//注意这个宏内部使用了一个静态的局部变量
#define Baker_GetResults(baker,bm,w,h,ptr)					\
{																						\
	static std::vector<BakeResult>buf;							\
	buf.clear();																	\
	(w)=(h)=0;																		\
	(ptr)=NULL;																	\
	if (baker->GetMapSize(bm,w,h))							\
	{																						\
		buf.resize((w)*(h));															\
		baker->GetResults(bm,buf.data());								\
		(ptr)=buf.data();																	\
	}																						\
}



/********************************************************************
	created:	2009/03/01
	created:	1:3:2009   8:39
	filename: 	d:\IxEngine\Interfaces\WorldSystem\ITrrn.h
	file path:	d:\IxEngine\Interfaces\WorldSystem
	file base:	ITrrn
	file ext:	h
	author:		cxi
	
	purpose:	terrain defines and interfaces
*********************************************************************/
#pragma once


//////////////////////////////////////////////////////////////////////////
//Trrn Brush Related

#define MAX_BRUSH_TEX 8


#define MAX_TRRN_BRUSHLEVEL 12

#define MAX_BRUSH 255
#define MAX_BRUSH_IN_LEVEL 20//max brushes count in a level

#define BRUSHLEVEL_NS_COUNT 4 //有多少个level保留给带法线/高光贴图的笔刷

typedef unsigned char BrushID;
#define BRUSHID_INVALID 0xff
#define BRUSHID_Make(lvlBrush,idx) ((lvlBrush)*20+((idx)%20))
#define BRUSHID_GetLevel(bid) (bid/20)
#define BRUSHID_IsNSLevel(bid) (BRUSHID_GetLevel(bid)>=(MAX_TRRN_BRUSHLEVEL-BRUSHLEVEL_NS_COUNT))
#define BRUSHLEVEL_IsNSLevel(lvl) (lvl>=(MAX_TRRN_BRUSHLEVEL-BRUSHLEVEL_NS_COUNT))

#define BRUSH_PRIORITY_INVALID (0xffffffff)

typedef DWORD TTSID;//Terrain Texture Set ID
#define TTSID_INVALID 0

#define TTSID_GetLevel(id) ((id)>>16)

struct BrushLibRTArg
{
	BrushLibRTArg()
	{
		fmtTex=21;//D3DFMT_A8R8G8B8;
		lenTex=2048;
		miplvlTex=9;
		bNS=0;
		reserved=0;
	}
	DWORD fmtTex;
	DWORD lenTex;//size of the tex,in pixel
	DWORD miplvlTex:8;
	DWORD bNS:1;
	DWORD reserved:23;
	//注意,如果新加入成员变量,要在BrushLibRTArgEx::operator==(..)中添加相应的代码
};



//A describing for the RunTime Terrain Map
struct TrrnParam
{
	TrrnParam()
	{
		Zero();
	}
	void Zero()
	{
		radius=128;
		bEditMode=FALSE;
		bCullHole=FALSE;
		nDetailClip=2;
		bDitherBlend=FALSE;
		bForcePLBM=FALSE;
		bLoadLightMap=TRUE;
		bLoadNormalMap=TRUE;
		bAllowNS=FALSE;
		bShowNml=FALSE;
		bShowSpec=FALSE;
	}
	void Limit()
	{
		radius=i_math::clamp_i(radius,4,512);

		nDetailClip=i_math::clamp_i(nDetailClip,2,3);
	}

	int radius;//in block
	BOOL bEditMode;
	BOOL bCullHole;//hole的面片是否要cull掉
	DWORD nDetailClip;//how many inner clip should be detailed
	BOOL bDitherBlend;//whether a transition will be applied between the detail 
										//part and naked part
	BOOL bForcePLBM;//是否强制载入Large Block的Pixel Map,这个标志通常在编辑时置为TRUE
	BOOL bLoadLightMap;
	BOOL bLoadNormalMap;

	BOOL bAllowNS;//是否需要NS的贴图
	BOOL bShowNml;//是否需要绘制高光(只在bAllowNS为TRUE时有效)
	BOOL bShowSpec;//是否需要绘制高光(只在bAllowNS为TRUE时有效)

};



//////////////////////////////////////////////////////////////////////////
//Terrain Map Editor Related
struct TrrnSeedMapArg
{
	TrrnSeedMapArg()
	{
		purpose=Purpose_None;
	}
	enum Purpose
	{
		Purpose_None,
		Purpose_AddBr,
		Purpose_RemoveBr,
		Purpose_SetBaseBr,

		Purpose_AddHt,

		Purpose_AddHole,
		Purpose_RemoveHole,
	};
	Purpose purpose;

	i_math::vector3df vCenter;
	float radius;
	float radius2;

	unsigned char idBr;//BrushID,for reference,
};


struct TrrnSeedMap
{
	TrrnSeedMap()
	{
		Zero();
	}
	void Zero()
	{
		vCenter.setZero();
		lvl=0;
		bContent=FALSE;
	}
	void Clear()
	{
		points.clear();
		boundary.clear();
		boundary2.clear();
		blocksAffected.clear();
		rc.set(0,0,0,0);
		rcAllowed.set(0,0,0,0);
		Zero();
	}

	BOOL IsEmpty()	{		return !bContent;	}

	enum SeedPointFlag
	{
		SeedPointF_None=0,
		SeedPointF_ForbiddenBrush=1,
		SeedPointF_ForbiddenDueToReadOnly=2,
	};

	struct SeedPoint
	{
		int x,y;//in grid
		vector3df v;
		float wt;//weight
		SeedPointFlag flag;
	};
	BOOL bContent;
	std::vector<SeedPoint> points;
	std::vector<i_math::vector3df> boundary;
	std::vector<i_math::vector3df> boundary2;
	std::vector<i_math::pos2di> blocksAffected;
	i_math::vector3df vCenter;
	//注意
	i_math::recti rc;//in the split unit for lvl
	i_math::recti rcAllowed;//in the split unit for lvl
	int lvl;
};



class ITrrnBrushLibRT
{
public:
	INTERFACE_REFCOUNT;
};


//add pattern texture to a pattern set,one tex for each pattern
//each pattern tex is arranged in texFullPattern in a defined format
//The defined format:
//		00					20					22					02

//		20|22|02		00|22|02		00|20|02		00|20|22

//		00|20				20|22				22|02				00|02

//		20|02				00|22				[undefined]		[undefined]

struct TexData;
class ITrrnBrushLib
{
public:
	INTERFACE_REFCOUNT;
	virtual void SetPath(const char *path)=0;//这个路径是一个局部路径
	virtual const char *GetSscPath()=0;//返回完整的路径
	virtual const char *GetCachePath()=0;//返回完整的Cache文件的路径
	virtual BOOL New()=0;
	virtual BOOL Load()=0;
	virtual BOOL Save()=0;
	virtual void ClearContent()=0;
	virtual BOOL IsModified()=0;
	virtual ITrrnBrushLibRT* GetRT(BrushLibRTArg &arg)=0;//will always return a 1-refcount pointer,if successful

	//Tex Set
	virtual TTSID NewTexSet(DWORD iLevel,const char *name,DWORD lenSlot)=0;//new a tex set using the name
	virtual BOOL RenameTexSet(TTSID idTS,const char *nameNew)=0;
	virtual BOOL RemoveTexSet(TTSID idTS)=0;
	virtual const char *GetTexSetName(TTSID idTS)=0;
	virtual DWORD GetTexSetLevel(TTSID idTS)=0;//return -1 on failure
	virtual TTSID FindTexSetID(DWORD iLevel,const char *name)=0;
	virtual BOOL SetTexSetLenSlot(TTSID idTS,DWORD lenSlot)=0;
	virtual BOOL GetTexSetLenSlot(TTSID idTS,DWORD &lenSlot)=0;

	virtual DWORD GetTexSetCount(DWORD iLevel)=0;
	virtual TTSID GetTexSetID(DWORD iLevel,DWORD iTexSet)=0;
	virtual void ClearTexSet(DWORD iLevel)=0;//clear all the tex set in the level

	virtual BOOL AddTex(TTSID idTS,TexData *data)=0;//add a texture to the specified tex set
	virtual BOOL ReplaceTex(TTSID idTS,DWORD iTex,TexData *data,BOOL bNS)=0;
	virtual BOOL RemoveTex(TTSID idTS,DWORD iTex)=0;
	virtual void ClearTex(TTSID idTS)=0;//clear all the texture in a tex set
	virtual DWORD GetTexCount(TTSID idTS)=0;
	virtual TexData *GetTexData(TTSID idTS,DWORD iTex,BOOL bNS)=0;

	virtual BOOL IsTexSetTransparent(TTSID idTS)=0;

	//Brush
	virtual BOOL IsBrushLevelFull(DWORD iLevel)=0;
	virtual BrushID NewBrush(DWORD iLevel,const char *name)=0;
	virtual BOOL RenameBrush(BrushID idBr,const char *nameNew)=0;
	virtual BOOL RemoveBrush(BrushID idBr)=0;
	virtual const char *GetBrushName(BrushID idBr)=0;
	virtual BrushID FindBrushID(const char *name)=0;
	virtual DWORD GetBrushPriority(BrushID idBr)=0;

	virtual DWORD GetBrushCount()=0;
	virtual BrushID GetBrushID(DWORD iBrush)=0;
	virtual void ClearBrush()=0;//Clear all the brushes

	virtual BOOL SetBrushTexSet(BrushID idBr,TTSID idTS)=0;
	virtual TTSID GetBrushTexSet(BrushID idBr)=0;
	virtual BOOL SetBrushRepeat(BrushID idBr,DWORD repeat)=0;
	virtual BOOL GetBrushRepeat(BrushID idBr,DWORD &repeat)=0;

	virtual BOOL AssignBrushID(BrushID idBr,BrushID idBrNew)=0;

	virtual BOOL SetFallBack(BrushID idBr,BrushID idBrFallBack)=0;
	virtual BOOL GetFallBack(BrushID idBr,BrushID &idBrFallBack)=0;

	//Switch priority value with the neighbour brush,bInc indicate whether switch with 
	//the higher-priority brush or the lower one.
	virtual BOOL ModifyBrushPriority(BrushID idBr,BOOL bInc)=0;
};

enum TrrnViewMode
{
	Default,
	Black,
	White,
	Gray,
};

class IRenderer;
class IRenderPort;
class ITrrnMapEditor;
class ILight;
class IMapFile;
struct TrrnDrawArg;
struct TrrnParam;
class CProgress;
class ITexture;
class ITrrnMap
{
public:
	INTERFACE_REFCOUNT;

	virtual BOOL Reset(TrrnParam &cfg)=0;
	virtual IMapFile *GetMapFile()=0;//will NOT increase ref-count
	virtual ITrrnBrushLibRT *GetBrushLib()=0;//will NOT increase ref-count
	virtual TrrnParam &GetParam()=0;
	virtual ITrrnMapEditor *GetEditor()=0;
	virtual BOOL IsEmpty()=0;

	virtual void SetCenter(i_math::vector3df &pos,CProgress *prg=NULL)=0;
	virtual void GetCenter(i_math::vector3df &pos)=0;
	virtual void GetCenterInBlock(i_math::pos2di &pt)=0;

	virtual BOOL Draw(IRenderPort *rp,TrrnDrawArg&arg)=0;

	virtual BOOL GetShadowMap(ITexture * &tex,i_math::vector4df &uvParam) = 0;
	virtual BOOL GetLightMap(ITexture * &tex,i_math::vector4df &uvParam) = 0;

	virtual ITexture *GetTestTex()=0;

	virtual void SetViewMode(TrrnViewMode mode)=0;
	virtual TrrnViewMode GetViewMode()=0;

	virtual void SaveModified()=0;
	virtual void Reload(i_math::pos2di*blkpos,DWORD nBlks)=0;
	virtual void ReloadAll()=0;

	virtual const char *GetStats()=0;
	virtual DWORD GetDrawCallCount()=0;

};


class HitProbe;
class CProgress;
struct TrrnSeedMap;
struct TrrnSeedMapArg;
struct BakeSample;
class ITrrnMapEditor
{
public:

	//CalcXXXXBound()返回临时指针,不能保留
	virtual i_math::vector3df*CalcRoundBound(i_math::vector3df &vCenter,float radius,int nSteps,DWORD &count)=0;
	virtual i_math::vector3df*CalcQuadBound(i_math::rectf rc,int nSteps,DWORD &count)=0;

	virtual BOOL GetHitPos(float x,float z,BOOL bIgnoreHole,i_math::vector3df &vHit,i_math::vector3df *vHitNormal=NULL)=0;

	virtual BOOL GetHitPos(HitProbe &probe,BOOL bIgnoreHole,i_math::vector3df &vHit,i_math::vector3df *vHitNormal=NULL)=0;

	virtual BOOL CollectTris(i_math::aabbox3df &aabb,std::vector<i_math::vector3df>&tris,std::vector<i_math::vector3df>*nmls=NULL)=0;

	virtual BOOL CalcSeedMap(TrrnSeedMap &seedmap,TrrnSeedMapArg &arg)=0;

	virtual DWORD GetHoleLvl()=0;

	virtual BOOL BeginModify()=0;

	virtual BOOL AddBrush(TrrnSeedMap &seedmap,BrushID idBr)=0;
	virtual BOOL AddHeight(TrrnSeedMap &seedmap,BOOL bRelative,float htHi,float htLow,BOOL bHiReso=FALSE,BOOL bImprint=FALSE)=0;
	virtual BOOL ModOpaque(TrrnSeedMap &seedmap,BOOL bRemove)=0;
	virtual BOOL ModHole(TrrnSeedMap &seedmap,BOOL bRemove)=0;

	virtual BOOL EndModify()=0;

	virtual i_math::vector3df *CalcNormals(i_math::recti &rcBlk,DWORD lvl)=0;

	virtual BOOL BeginUpdateDitherColor()=0;
	//update a range of blocks,rcBlock should be within the currently loaded(touched) range
	virtual BOOL UpdateDitherColor(i_math::recti &rcBlock)=0;
	virtual BOOL EndUpdateDitherColor()=0;

	virtual i_math::vector4df GetLightMapUVF()=0;//返回一个参数,用来根据世界位置计算在LightMap上的uv
	virtual i_math::vector2df GetLightMapScale()=0;
	virtual ITexture* GetLightMap()=0;//返回LightMap的贴图
	virtual ITexture* GetShdwMap()=0;//返回Shadow LightMap的贴图

	virtual ITexture *GetNormalMap()=0;
	virtual ITexture *GetHeightMap()=0;
	virtual i_math::vector4df GetHeightMapUVF()=0;

	virtual void EnableTrisCache(BOOL bEnable)=0;

	virtual void SetTestValue(float x,float y)=0;


	virtual void GarbageCollect()=0;

};

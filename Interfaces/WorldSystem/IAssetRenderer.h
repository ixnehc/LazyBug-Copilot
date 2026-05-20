/********************************************************************
	created:	1:3:2009   8:48
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		cxi
	
	purpose:	interfaces for Asset Renderer
*********************************************************************/

#pragma once

#include "gds/GObj.h"

#include "ref/ref.h"

#include "WorldSystem/IBrushLib.h"

#include "RenderSystem/IVertexBuffer.h"

#include "IAssetRendererDefines.h"

#include "valueset/valueset.h"

#include "timer/wuid.h"

class IRatomsBv
{
public:
	virtual int AddRef()=0;
	virtual int Release()=0;
	virtual void Destroy()=0;//会减一个引用计数
	virtual void Add(RatomID id)=0;
	virtual void Remove(RatomID id)=0;
};



class IRenderPort;
class ICamera;
struct GlobalRenv;
class IAnimNode;
class IRatomsBv;
class IEnvLight;
class IDynamicLocalLight;

class IAssetRenderer
{
public:
	virtual void ResetConfig()=0;
	virtual void UpdateCurZoner()=0;
	virtual BOOL EnableAllRagents(BOOL bEnable)=0;
	virtual BOOL EnableRagent(RagentType type,BOOL bEnable)=0;
	virtual BOOL SetRP(IRenderPort *rp)=0;//这个函数只用于Lab的AssetSystem的AssetRenderer
	virtual BOOL Render(IRenderPort *rp,AssetRendererPart part)=0;
	virtual BOOL UpdateRender()=0;
	virtual void DumpStats(AdrStats &stats)=0;

	virtual GlobalRenv*GetGlobalRenv()=0;

	virtual void SetEnvLight(IEnvLight *el)=0;
	virtual IEnvLight *GetEnvLight()=0;

	//使用最近一次绘制时的信息来计算hit probe
	virtual ICamera *GetRecentCamera()=0;//得到最近一次绘制使用的camera
	virtual BOOL CalcHitProbe(i_math::line3df &line,int x,int y,float length)=0;
	virtual BOOL TransPos(i_math::vector3df &v)=0;

	//Ratoms Bv(bounding volume) related
	//RatomsBv用来包裹ratoms
	virtual IRatomsBv* CreateRatomsBv(IAnimNode *anAabb)=0;//anAabb要支持GetAabb(..)

	virtual OccluderID RegisterOccluder(const char *pathMesh,i_math::matrix43f &mat)=0;
	virtual void UnRegisterOccluder(OccluderID idOccluder)=0;

	virtual RatomID *EnumRatoms(i_math::aabbox3df &aabb,DWORD &c)=0;
	virtual RagentType RagentFromRatom(RatomID id)=0;
	virtual BOOL CheckRatomMotive(RatomID id)=0;//检查这个ratom是否是运动

	virtual RenvID RegisterRenv(IDynamicLocalLight *lgt)=0;
	virtual void UnRegisterRenv(RenvID id)=0;

	virtual void RegisterGlobalLightColMod(IAnimNode *an)=0;
	virtual void UnregisterGlobalLightColMod(IAnimNode *an)=0;

	virtual BOOL IsDumping()=0;
	virtual BOOL BeginDump(const char *path)=0;
	virtual AResult CheckDump()=0;
	virtual void EndDump()=0;

};


//////////////////////////////////////////////////////////////////////////
//Ratoms

struct RatomFadeDefault
{
	RatomFadeDefault()
	{
		Zero();
	}
	enum Mode
	{
		Mode_3D,//距离以3D方式计算
		Mode_2D,//距离以2D(XZ平面)方式计算
	};
	void Zero()
	{
		dist=0;
	}
	BOOL IsValid()	{		return dist>0;	}
	WORD dist;//多远距离开始消失,单位为米,如果为0x0,不需要fade
	BYTE range;//从dist开始,再过多远距离完全消失,,单位为米
	BYTE md;
};


class IAsset;
class IMatrice43;
class ITexture;
class IMtrl;
struct TexData;
class IAnimNode;
class IRatomsBv;
struct StaticLightData;
class IMano;
class IDtrPieces;
class IDummies;

class IRatomsDefault
{
public:

	virtual RatomID Register(IAnimNode *base,IAsset *owner)=0;
	virtual void UnRegister(RatomID ratom)=0;
	virtual BOOL UpdateContent_Mesh(RatomID ratom,const char *pathMesh,const char *pathMtrl)=0;
	virtual BOOL UpdateContent_Mesh(RatomID ratom,const char *pathMesh,IMtrl *mtrl)=0;
	virtual BOOL UpdateContent_ShdwMesh(RatomID ratom,const char *pathMesh)=0;
	virtual BOOL UpdateContent_Dtr(RatomID ratom,IDtrPieces *piecesDtr,const char *pathMtrl,DWORD iPart)=0;
	virtual BOOL UpdateContent(RatomID ratom,VBPatch &patch,i_math::aabbox3df &aabb,const char *pathMtrl)=0;
	virtual BOOL UpdateGlow(RatomID ratom,BOOL bGlow)=0;
	virtual BOOL UpdateMasking(RatomID ratom,BOOL bMasking)=0;
	virtual BOOL UpdateShadowCast(RatomID ratom,ShadowCast cast)=0;
	virtual BOOL UpdateEnvMap(RatomID ratom,const char *pathEnvMap)=0;
	virtual BOOL UpdateLightMap(RatomID ratom,StaticLightData *sld)=0;
	virtual BOOL UpdateGLM(RatomID ratom,GlobalLightingMethod glm)=0;
	virtual BOOL UpdateMano(RatomID ratom,BOOL bCheckMano)=0;
	virtual BOOL UpdateFade(RatomID ratom,RatomFadeDefault &fade)=0;
	virtual BOOL UpdateTrrn(RatomID ratom,BOOL bTrrn,RenderLayorOnTrrnLevel level=RenderLayorOnTrrnLevel_Default)=0;////如果bTrrn为TRUE,这个ratom需要以紧贴地表的方式绘制.目前level的取值范围为[0,9]
	virtual BOOL UpdateSeeThru(RatomID ratom,const char *pathDummies,i_math::matrix43f &mat,WUID idGrp)=0;
	virtual BOOL UpdateDtr_HidePiece(RatomID ratom,DWORD iPiece)=0;//
	virtual BOOL UpdateDtr_ShowPiece(RatomID ratom,DWORD iPiece)=0;//
	virtual BOOL UpdateOpaqueSort(RatomID ratom,BOOL bOpaqueSort)=0;
	virtual DWORD GetDtrMask(RatomID ratom)=0;//
	virtual BOOL UpdateVisible(RatomID ratom,BOOL bVisible)=0;//
	virtual BOOL UpdateAllowDecal(RatomID ratom,BOOL bAllow)=0;//
	virtual BOOL UpdateStaticShdwCaster(RatomID ratom,BOOL bStaticShdwCaster)=0;
	virtual BOOL UpdateCustomEP(RatomID ratom,IAnimNode *anEP)=0;
	virtual BOOL GetAllowDecal(RatomID ratom)=0;

	virtual GlobalLightingMethod GetActualGLM(RatomID ratom)=0;

	virtual SeeThruTargetHandle RegisterSeeThruTarget(IAnimNode *an,float radius)=0;
	virtual void UnRegisterSeeThruTarget(SeeThruTargetHandle h)=0;

	//修改这个ratom的zone info
	//zone info决定这个ratom怎样被(从空间上)枚举到
	//目前有三种方式,
	//1.加入到一个Ratoms Bound Volume,只要这个bound volume被枚举到,这个ratom就被枚举到
	//2.作为静态的zonee被加入到内部的ratom zoner里
	//3.作为动态的zonee被加入到内部的ratom zoner里
	virtual BOOL UpdateZoneInfo(RatomID ratom)=0;

	//得到一个ratom的mesh,注意返回的mesh和an都不带引用计数
	virtual IMesh *GetMesh(RatomID ratom,IAnimNode *&an)=0;

	//根据mesh的路径名和位置,找到一个ratom,注意这个函数有点慢
	virtual RatomID FindStatic(const char *mesh,i_math::matrix43f &mat)=0;
};

class ITrrnMap;
class IRatomsTrrn
{
public:
	virtual RatomID Register(ITrrnMap *trrn)=0;
	virtual void UnRegister(RatomID rat)=0;

	virtual void SetWireframe(BOOL bWireframe)	=0;
	virtual BOOL GetWireframe()=0;

	virtual void SetDrawHole(BOOL bDrawHole)=0;
	virtual BOOL GetDrawHole()=0;

	virtual void SetDraft(ITexture *texDraft)=0;
	virtual ITexture *GetDraft()=0;

};


struct RataSkySphere
{
	std::string pathBgTex;
	std::string pathBgMesh;
	std::string pathFgTex;
	std::string pathFgMesh;

	BEGIN_GOBJ_PURE(RataSkySphere,1);
	GELEM_STRING_INIT(pathBgTex,"");
	GELEM_STRING_INIT(pathBgMesh,"");
	GELEM_STRING_INIT(pathFgTex,"");
	GELEM_STRING_INIT(pathFgMesh,"");
	END_GOBJ();
};

class IRatomsSky
{
public:
	virtual RatomID Register(RataSkySphere &rata)=0;
	virtual void UnRegister(RatomID ratom)=0;
	virtual BOOL ShowMerged(RatomID ratom1,RatomID ratom2,float rate)=0;//if rate is 1.0,full ratom2,if 0.0,full ratom1
	virtual BOOL ShowFull(RatomID ratom)=0;
};



struct CWater;
struct WMesh
{
	IMPLEMENT_REFCOUNT_C

	DEFINE_CLASS(WMesh);

	WMesh(){
		bInvalid = true;
		uid = INVALID_BRUID;
		verDraw = 0;
	}

	~WMesh(void){
		Clean();
	}

	void Clean(){
		SAFE_RELEASE(vb.vb);
		SAFE_RELEASE(vb.ib);
	}

	VBHandles vb;
	bool  bInvalid;
	BRUID  uid;
	DWORD verDraw;		//绘制版本 
};

struct WRatomInfo
{
	i_math::recti rc;
	WMesh * pMesh;
};

struct WaterMesh
{
	WaterMesh(){
		vb = NULL;
		ibInner = NULL;
		pcInner = 0;
		ibOutter = NULL;
		pcOutter = 0;
	}
	IVertexBuffer *vb; //共用相同的顶点Buffer
	struct _Tris{ 
		_Tris(){ ps = 0;pc = 0;brID=0;}
		DWORD ps ,pc;
		int brID;
	};
	std::vector<_Tris> innerMeshs;    //
	std::vector<_Tris> outterMeshs;	  //
	IIndexBuffer  *ibInner,*ibOutter;
	DWORD pcInner,pcOutter;
	i_math::vector4df border;
	i_math::vector3df orgPosInner,orgPosOutter;
	/************************************************************************/
	/*一个水域包括精细的内部区域 和 一个较粗糙的外部区域 并按照刷子的高度分类类型                                                                    */
	/************************************************************************/
};

class IRatomsLiquid
{
public:

	virtual RatomID Register(WRatomInfo &info) = 0;

	virtual void UnRegister(RatomID rat)=0;

	virtual void SetWaterLib(IBrushLib * pLib) = 0;

	virtual void SetWaterMesh(WaterMesh * mesh) = 0;
};


enum ShellRatomType
{
	ShellRatom_Image,
	ShellRatom_Text,
	ShellRatom_Polygon,
	ShellRatom_Canvas,
	ShellRatom_Board,
};

enum ShellImageCombo
{
	ImgCombo_Normal,//一整张
	ImgCombo_Row3,//三行
	ImgCombo_Bow9,//九格
};


struct CanvasPassEnv
{
	i_math::vector3df dirDL;//直线光的方向
	DWORD colDifDL;//直线光的颜色
};

class IShader;
class IMtrl;
class IRenderer;
class IShellDrawer
{
public:

	//部分自主的绘制
	virtual IMtrl*GetMtrl()=0;
	virtual BOOL BindShader(IShader *shader)=0;

	//完全自主的绘制,在GetMtrl()返回NULL时调用
	virtual void DoDraw(IRenderer *rdr,float pixelrate)=0;
};

class ITextPiece;
struct VtxQuad;
struct ImageCombo;
class IRatomsShell
{
public:
	virtual void SetPixelRate(float rate)=0;//代表一个逻辑单位对应多少像素
	virtual float GetPixelRate()=0;

	virtual RatomID Register(ShellRatomType type,IAnimNode *an=NULL)=0;//an只在type为ShellRatom_Board有意义
	virtual void UnRegister(RatomID ratom)=0;

	virtual BOOL UpdateLoc(RatomID ratom,ShellRect &loc)=0;//更新location,loc is in logic unit
	virtual BOOL UpdateLoc(RatomID ratom,ShellPos &ptLoc)=0;//更新location,ptLoc is in logic unit,这个函数只对字体有效
	virtual BOOL UpdateClip(RatomID ratom,ShellRect &clip)=0;//pass a ShellLoc(0,0,0,0) to disable clipping
	virtual BOOL UpdateImage(RatomID ratom,const char *path)=0;
	virtual BOOL UpdateImage(RatomID ratom,const char *path,ShellRect &rcOnImage)=0;
	virtual BOOL UpdateImageCombo(RatomID ratom,ShellImageCombo combo,ShellSize &szEdge)=0;
	virtual BOOL UpdateText(RatomID ratom,ITextPiece *tp)=0;
	virtual BOOL UpdatePolygon(RatomID ratom,VtxQuad *vertices,DWORD nVertices,WORD *indices,DWORD nIndices,BOOL bPixelRateAdjusted=FALSE)=0;//vertices里,每个顶点的z必须为0.5f
	virtual BOOL UpdateColor(RatomID ratom,DWORD col)=0;
	virtual BOOL UpdateAlpha(RatomID ratom,float alpha)=0;
	virtual BOOL UpdateCanvasRT(RatomID ratom,int w,int h)=0;//w/h传0来取消RT
	virtual BOOL UpdateCanvasPass(RatomID ratom,IAnimNode *an,ImageCombo *mask,AnimTick tFixed,RenderStyle style)=0;//an传NULL,表示不需要Pass
	virtual BOOL UpdateBoardZoneInfo(RatomID ratom)=0;
	virtual BOOL UpdateBoardScale(RatomID ratom,float unitscale)=0;//unitscale表示1米代表几个logic unit
	virtual BOOL UpdateBoardOff(RatomID ratom,i_math::pos2df &off)=0;//off单位为米
	virtual BOOL UpdateDrawer(RatomID ratom,IShellDrawer *drawer)=0;

	virtual BOOL UpdateMaskDrawer(RatomID ratom)=0;//这个ratom用来绘制mask
	virtual BOOL UpdateMaskClearer(RatomID ratom)=0;//这个ratom用来清除mask

	virtual BOOL GetPixel(RatomID ratom,ShellPos &pt,DWORD &col)=0;//pt is in logic unit,relative to the loc of this ratom
	virtual DWORD GetCanvasFrameCount(RatomID ratom)=0;//返回使用当前的canvas pass	一共画了几帧

	virtual BOOL BeginBoard(RatomID ratom)=0;//ratom必须是ShellRatom_Board类型,调用这个函数,后续的DrawRatom(..)会画在这个board上
	virtual void EndBoard()=0;

	virtual void DrawRatom(RatomID ratom)=0;
	virtual void ClearDrawRatom()=0;

};

struct SptWndCfg;
struct SptDrawArg;
struct CTree;
class IBrushLib;
struct TreeShadowData;
struct TreeRatomInfo
{
	i_math::vector3df pos;
	float rotY;
	float scale;
	i_math::aabbox3df aabb;		//runtime calc when  touch to scene
	BRUID refModel; 
	int idxShdw;
	TreeShadowData * shdwData;
};

class IRatomsTree
{	
public:	
	virtual RatomID Register(const TreeRatomInfo &info) = 0;
	virtual void	UnRegister(RatomID ratom) = 0;
	virtual void	SetSptLib(IBrushLib * sptLib) = 0;
	virtual void	SetWindLib(IBrushLib * wndLib) = 0;
};

class IDynamicPatch;
struct MtrlData;
class IRatomsPatches
{
public:
	virtual RatomID Register(IDynamicPatch *dnp,const char *pathMtrl,const char *pathMtrlGlow="")=0;
	virtual RatomID Register(IDynamicPatch *dnp,MtrlData &md)=0;
	virtual void UnRegister(RatomID ratom)=0;
	virtual BOOL Reside(RatomID ratom,BOOL bMotive,IRatomsBv *bv)=0;
	virtual void UnReside(RatomID ratom)=0;
	virtual IDynamicPatch *GetPatch(RatomID ratom)=0;
	virtual void UpdateVisible(RatomID ratom,BOOL bVisible)=0;
};

#define HelperLayor_None 0
#define HelperLayor_Path 1//路径
#define HelperLayor_Collision 2//碰撞体
#define HelperLayor_TrrnImprint 4//地表印刻
#define HelperLayor_Aligner 8//对齐边界
#define HelperLayor_Icon 16//缩略模型
#define HelperLayor_Spot 32//位点
#define HelperLayor_GameObj 64//游戏对象
#define HelperLayor_Occluder 128//遮挡体
#define HelperLayor_Skeleton 256//骨骼系统
#define HelperLayor_Decal 512//贴花空间
#define HelperLayor_HitTestBody 1024//HitTest碰撞体
//XXXXX:more Helper Layors

typedef DWORD HelperLayor;

class IDummies;
class IMopp;
class IRatomsHelper
{
public:
	virtual RatomID RegisterSphere(HelperLayor layor,DWORD col,float radius,IAnimNode *an)=0;
	virtual RatomID RegisterAabb(HelperLayor layor,DWORD col,IAnimNode *an)=0;
	virtual RatomID RegisterObb(HelperLayor layor,IAsset *ast,DWORD col,i_math::aabbox3df &aabb,IAnimNode *an)=0;
	virtual RatomID Register3DIcon(HelperLayor layor,IAsset *ast,const char *pathMesh,const char *mtrl,IAnimNode *an,DWORD col,BOOL bKeepScale=FALSE)=0;
	virtual RatomID RegisterPath(HelperLayor layor,IAsset *ast,i_math::vector3df *nodes,DWORD count,IAnimNode *an,DWORD col)=0;
	virtual RatomID RegisterDummies(HelperLayor layor,IAsset *ast,IDummies *dummies,IAnimNode *an,DWORD col)=0;
	virtual RatomID RegisterMopp(HelperLayor layor,IAsset *ast,IMopp *mopp,IAnimNode *an,DWORD col)=0;
	virtual RatomID RegisterMopp(HelperLayor layor,IAsset *ast,IMesh*mesh,IAnimNode *an,DWORD col)=0;

	virtual void UnRegister(RatomID id)=0;
};

class  ISpgLib;
struct SpgWindCfg;
struct CGrassPackage;
class IBrushLib;
struct SpgDrawInfo
{
	float scale; 
	float  rot;
	i_math::vector3df pos;
	float fLod;

	BRUID uid;
	DWORD eDif;		    //
	float eAmb;         //
	
	DEFINE_CLASS(SpgDrawInfo);
	IMPLEMENT_REFCOUNT_C
	
	void Load(CDataPacket &dp)
	{
		scale = dp.Data_NextFloat();
		rot = dp.Data_NextFloat();
		dp.Data_ReadData(&pos,sizeof(pos));
		fLod = dp.Data_NextFloat();

		dp.Data_ReadData(&uid,sizeof(uid));
		eDif = dp.Data_NextDword();
		eAmb = dp.Data_NextFloat();
	}

	void Save(CDataPacket &dp)
	{
		dp.Data_NextFloat() = scale;
		dp.Data_NextFloat() = rot;
		dp.Data_WriteData(&pos,sizeof(pos));
		dp.Data_NextFloat() = fLod;

		dp.Data_WriteData(&uid,sizeof(uid));
		dp.Data_NextDword() = eDif;
		dp.Data_NextFloat() = eAmb;		
	}

	float alpha; //runtime use only
};
struct SpgRatomInfo
{
	i_math::aabbox3df aabb;
	std::vector<SpgDrawInfo *> spgs;	//一系列的草的集合
};
class IRatomVegetable
{
public:
	virtual void SetSpgLib(IBrushLib * pSpgLib) = 0;
	virtual RatomID Register(const SpgRatomInfo &info) = 0;
	virtual void UnRegister(RatomID ratom) = 0;
	virtual void SetWindCfg(const SpgWindCfg * cfg) = 0;
};

class IRatomsGrass
{
public:
	virtual void SetSpgLib(IBrushLib * pSpgLib) = 0;
	virtual RatomID Register(const SpgRatomInfo &info) = 0;
	virtual void UnRegister(RatomID ratom) = 0;
	virtual void SetWindCfg(const SpgWindCfg * cfg) = 0;
};

//顶点结构体
struct ShoreVtx {
	i_math::vector4df pos;  //顶点信息	
	i_math::vector2df uv;	  
	DWORD eL;				//.xyz local  .w sun 
};

struct ShorePatch{
	
	IMPLEMENT_REFCOUNT_C

	DEFINE_CLASS(ShorePatch);

	std::vector<ShoreVtx> vtxs;
	std::vector<WORD> idxs;
};

struct ShoreRatomInfo
{
	BRUID uid;
	ShorePatch * patch;
	i_math::aabbox3df abb;		 //海岸线的包围盒
	float delay;
	float max_dist,trans_dist;
};

class CShore;
class IBrushLib;
class IRatomsShore
{
public:
	virtual void SetShoreLib(IBrushLib * pLib) = 0;
	virtual void SetWireframeVisible(BOOL bVisible) = 0;
	virtual RatomID Register(const ShoreRatomInfo & shoreInfo) = 0;
	virtual void UnRegister(RatomID ratom) = 0;
};

struct HaloParam
{
	std::string pathMesh;
	std::string pathHalo;
	std::string pathMtrl;
	DWORD nHalo;					//贴图的水平 垂直方向个分布多少个子贴图
	ValueSet distribute;			//描述各个贴图的在空间上的分布以及透明度	
	ValueSet alphaScale;			//调整Halo的透明度
	float szPixel;					//象素的宽度
	std::string pathFilter;			//光晕滤镜贴图路径
	float alpha;					//透明度
};

class IRatomsHalo
{
public:
	virtual void UpdateContent(RatomID ratom,HaloParam & param) = 0;
	virtual RatomID Register(IAnimNode *base,IAsset *owner) = 0;
	virtual void UnRegister(RatomID ratom) = 0;
};

struct RoadParam
{
	void * pVtx;
	DWORD  nVtx;
	WORD * pIB;
	DWORD  nIB;
	FVFEx  fvf;
	std::string pathTexDif;
	std::string pathMtrl;
	i_math::aabbox3df aabb;
};
class IRatomsRoad
{
public:
	virtual RatomID Register(RoadParam &param) = 0;
	virtual void UnRegister(RatomID ratomID) = 0;
	virtual void SetTrrnMap(ITrrnMap *trrnMap) = 0;
};

class IDynamicLocalLight;
class IRatomsDefer
{
public:
	virtual RatomID Register(IDynamicLocalLight*dnl)=0;
	virtual void UnRegister(RatomID ratom)=0;
};


struct DecalVBInfo
{
	DWORD nVtx;
	i_math::vector3df *pos;
	i_math::vector3df *nml;
	i_math::vector2df *uv0;
	i_math::vector2df *uv1;

	DWORD nIdx;
	WORD *idx;
};

struct DecalAnimArg
{
	DecalAnimArg()
	{
		Zero();
	}
	void Zero()
	{
		tLast=ANIMTICK_FROM_SECOND(1.0f);
		tFadeIn=0;
		tFadeOut=ANIMTICK_FROM_SECOND(1.0f);
		wTile=hTile=1;
	}
	AnimTick tLast;//如果为0,表示永久持续
	AnimTick tFadeIn;
	AnimTick tFadeOut;
	BYTE wTile,hTile;//用来表示贴图上有几乘几张动画序列帧
};

class IMtrl;
struct VtxDecal;
class IRatomsDecal
{
public:
	virtual BOOL Add(VtxDecal *vtx,DWORD nVtx,WORD *idx,DWORD nIdx,i_math::matrix43f &mat,IMtrl *mtrl,
							RatomID ratomOwner,DecalAnimArg &argAnim,BOOL bGlow)=0;
	virtual RatomID Register(VtxDecal *vtx,DWORD nVtx,WORD *idx,DWORD nIdx,i_math::matrix43f &mat,IMtrl *mtrl,
										RatomID ratomOwner,DecalAnimArg &argAnim,BOOL bGlow)=0;
	virtual RatomID Register(IVertexBuffer *vb,IIndexBuffer *ib,i_math::aabbox3df &aabb,i_math::matrix43f &mat,IMtrl *mtrl,
										RatomID ratomOwner,DecalAnimArg &argAnim,BOOL bGlow)=0;//调用方调用该函数后,不要再访问传入的vb/ib
	virtual void UnRegister(RatomID ratom)=0;
	virtual void Update_Stop(RatomID ratom)=0;//让这个ratom立即消失

	virtual IVertexBuffer *AllocVB(DWORD nVtx) = 0;
	virtual void FreeVB(IVertexBuffer *vb) = 0;
	virtual IIndexBuffer *AllocIB(DWORD nIdx) = 0;
	virtual void FreeIB(IIndexBuffer *ib) = 0;

};


struct LichenBrushArg
{
	std::string pathMesh;
	std::string pathMtrl;
	std::string pathGrndMesh;
	std::string pathGrndMtrl;
};

struct LichenInfluenceInfo
{
	enum Type
	{
		Spawn,
		Attract,
		Dispel,

		ForceDword=0xffffffff,
	};
	Type tp;
	i_math::vector3df pos;
	float str;
	float radius;
	float radiusCore;//用于Attract和Dispel
};

class ILichenInfluence
{
public:
	INTERFACE_REFCOUNT;

	virtual LichenInfluenceInfo &GetInfo(AnimTick t)=0;
};

class IRatomsLichen
{
public:
	virtual BOOL ExistBrush(StringID nmBrush)=0;
	virtual void AddBrush(StringID nmBrush,LichenBrushArg &arg)=0;
	virtual void ClearBrushes()=0;
	virtual RatomID Register(ILichenInfluence *infl,StringID nmBrush)=0;
	virtual void UnRegister(RatomID ratom)=0;
};

//XXXXX:more ragent


#pragma once
#include "IRenderSystemDefines.h"

#include "anim/animbase.h"

#include "bitset/bitset.h"

#include "shaderlib/SLDefines.h"

#include <string>
#include <vector>

//Device 
class IVertexMgr;
class ITexture;
class ITextureMgr;
class IRTextureMgr;
class IWTextureMgr;
class ISurface;
class ISurfaceMgr;
class IDeviceObject;

//renderer
class IRenderer;

//2d draw 
class ITextPiece;
class IFontMgr;

//3d draw 
class IRenderPort;
class ICamera;
class ILight;
class ISkeleton;
class IMatrice43;

class IMesh;
class IMeshSnapshot;
class IMeshMgr;
class IDynMeshMgr;
class IMatrice43Mgr;
class IMtrl;
class IMtrlMgr;
class IAnim;
class IAnimMgr;
class IDynAnimMgr;
class IDummies;
class IDummiesMgr;
class IDynDummiesMgr;
class ISpt;
class ISptMgr;
class IDynSptMgr;
class IMoppMgr;
class ISpgMgr;
class IDynSpgMgr;
class IAnimTreeMgr;
class IDynAnimTreeMgr;
class IBoneAnimMgr;
class IDynBoneAnimMgr;
class IMtrlExtMgr;
class ISoundMgr;
class IRecordsMgr;
class IRagdollMgr;
class IDtr;
class IDtrMgr;
class IBehaviorGraphMgr;
//XXXXX:more res type

class ISheet;
class ISheetMgr;

class IAnimPlayer;
class IShader;
class IShaderLibMgr;
class ITexAtlasPool;
class ITexAtlasMap;
class ITexAtlas;

class IVar;
class IPatchGeom;
class IPatchQueue;
class ICvxVolume;
//global 
class IRenderSystemUtil;
class IRenderSystem;
class IPatchBuilder;

struct FeatureCode;
struct ShaderCode;

struct GVar;
class IFileSystem;
struct ProfilerMgr;
struct LogHandler;
class SkeletonInfo;
class CStrLib;
struct TexInfo;
class IRenderSystem
{
public:						
	virtual BOOL Init(const RenderSystemInit &param)=0;//**stl used
	virtual BOOL UnInit()=0;//if check leak,and leak found,return FALSE,otherwise return TRUE

	virtual BOOL ResetDevice(DeviceConfig &cfg)=0;
	virtual void CleanDevice()=0;
	virtual BOOL IsDeviceReset()=0;
	virtual DeviceCap&GetDeviceCap()=0;

	virtual void OnIdle(__int64 &tIdle)=0;//tIdle is in ms
	virtual void Update(DWORD dwTick)=0;
	virtual BOOL CheckAllResLeak()=0;//return whether there is any leak
	virtual void SetResSyncLoad(BOOL bSync)=0;//设定资源载入模式是异步还是同步,

	virtual IFileSystem *GetFS()=0;
	virtual const char *GetPath(RenderSystemPath rsp)=0;
	virtual ProfilerMgr *GetProfilerMgr()=0;
	virtual void RegisterLogHandler(LogHandler &handler)=0;

	virtual void SetStrLib(CStrLib *strlib)=0;

	virtual BOOL BeginFrame()=0;
	virtual BOOL EndFrame()=0;
	virtual BOOL Present(i_math::recti *rcDest=NULL,i_math::recti *rcSrc=NULL,HWND hwndOverride=NULL)=0;
	virtual BOOL PresentAsyn(i_math::recti *rcDest=NULL,i_math::recti *rcSrc=NULL,HWND hwndOverride=NULL)=0;
	virtual void FlushCommand()=0;
	virtual void UpdateResMonitor()=0;

	virtual AnimTick GetPresentTick()=0;//得到最近一次Present()的时间,通常用于一些与逻辑无关的(只与显示有关)的动画更新

	virtual IVertexMgr *GetVertexMgr()=0;
	virtual ITextureMgr *GetTexMgr()=0;
	virtual IRTextureMgr *GetRTexMgr()=0;
	virtual IWTextureMgr *GetWTexMgr2()=0;
	virtual ISurfaceMgr *GetSurfMgr()=0;
	virtual IShaderLibMgr *GetShaderLibMgr()=0;
	virtual IFontMgr *GetFontMgr()=0;
	virtual IMeshMgr *GetMeshMgr()=0;
	virtual IMtrlMgr *GetMtrlMgr()=0;
	virtual IAnimMgr *GetAnimMgr()=0;
	virtual IDynAnimMgr *GetDynAnimMgr()=0;
	virtual IDummiesMgr *GetDummiesMgr()=0;
	virtual IDynDummiesMgr * GetDynDummiesMgr()=0;
	virtual ISptMgr   * GetSptMgr() = 0;
	virtual IDynSptMgr * GetDynSptMgr() = 0;
	virtual IMoppMgr *GetMoppMgr()=0;
	virtual ISpgMgr *GetSpgMgr()=0;
	virtual IDynSpgMgr *GetDynSpgMgr()=0;
	virtual ISheetMgr *GetSheetMgr()=0;
	virtual IAnimTreeMgr *GetAnimTreeMgr()=0;
	virtual IDynAnimTreeMgr *GetDynAnimTreeMgr()=0;
	virtual IBoneAnimMgr *GetBoneAnimMgr()	= 0;
	virtual IDynBoneAnimMgr *GetDynBoneAnimMgr()  = 0;
	virtual IMtrlExtMgr *GetMtrlExtMgr()	= 0;
	virtual ISoundMgr *GetSoundMgr()	= 0;
	virtual IRecordsMgr *GetRecordsMgr()	= 0;
	virtual IRagdollMgr *GetRagdollMgr()	= 0;
	virtual IDtrMgr *GetDtrMgr()	= 0;
	virtual IBehaviorGraphMgr *GetBehaviorGraphMgr()	= 0;
	//XXXXX:more res type

	virtual IPatchBuilder * GetPatchBuilder() = 0;
	
	virtual IRenderPort *CreateRenderPort()=0;
	virtual ICamera *CreateCamera()=0;
	virtual ILight *CreateLight()=0;
	virtual IAnimPlayer *CreateAnimPlayer()=0;
	virtual ISkeleton *CreateSkeleton(SkeletonInfo &si)=0;
	virtual IMatrice43 *CreateMatrice43()=0;
	virtual IMeshSnapshot *CreateMeshSnapshot()=0;
	virtual ITexAtlasPool *CreateTexAtlasPool(TexInfo &ti)=0;
	virtual ITexAtlasMap *CreateTexAtlasMap(TexInfo &ti,BOOL bAllowResize)=0;
	virtual IPatchGeom *CreatePatchGeom()=0;
	virtual IPatchQueue *CreatePatchQueue()=0;
	virtual ICvxVolume *CreateConvexVolume()=0;

};


struct ViewportInfo;

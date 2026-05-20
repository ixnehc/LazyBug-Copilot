/********************************************************************
created:	2008/1/16   12:33
filename: 	e:\IxEngine\Interfaces\WorldSystem\IWorldSystem.h
author:		cxi

purpose:	exposed world system interfaces for the world system user
*********************************************************************/

#pragma once
#include "IWorldSystemDefines.h"

#include "IWorldSystemInterfaces.h"

#include "math/rect.h"

struct LogHandler;
struct ProfilerMgr;
class CStrLib;
class ISpgLib;

class IAnim;
class IBoneAnim;
class IAnimTree;
class IAnimTreeCtrl;
class ISptDrawer;

class IPhysRagdoll;
class IRagdollCtrl;

class CConfig;
struct RagdollSwitchArg;
class IWorldSystem
{
public:
	virtual BOOL Init(WorldSystemInit &wsi)=0;
	virtual BOOL UnInit()=0;//return FALSE if there is some res leak
	virtual IRenderSystem * GetRS()=0;
	virtual IFileSystem * GetFS()=0;
	virtual IUtilRS *GetUtilRS()=0;
	virtual IKinectDevice *GetKD()=0;
	virtual const char *GetPath(WorldSystemPath wsp)=0;
	virtual ProfilerMgr *GetProfilerMgr()=0;
	virtual void RegisterLogHandler(LogHandler &handler)=0;
	virtual void SetStrLib(CStrLib *strlib)=0;

	virtual ISceneBaker *CreateSceneBaker(IEntitySystem *pES)=0;
	virtual IEnvLight *CreateEnvLight(IAssetSystem *pAS)=0;
	virtual IMiniMapBaker*CreateMiniMapBaker(IEntitySystem *pES)=0;
	virtual IOutlineMapBaker*CreateOutlineMapBaker(IEntitySystem *pES)=0;
	virtual IGtiBaker*CreateGtiBaker(IEntitySystem *pES)=0;

	virtual ISptDrawer * CreateSptDrawer() =0;

	virtual IAnimTreeCtrl *CreateAnimTreeCtrl(IBoneAnim **anims,DWORD nAnim,IAnimTree *animtree,AnimTick t,BOOL bStringTunerName=FALSE)=0;
	virtual IRagdollCtrl*CreateRagdollCtrl(IAnimTreeCtrl *ctrl,IPhysRagdoll *rgd,i_math::matrix43f &matBase,AnimTick t,AnimTick tLast,RagdollSwitchArg &arg)=0;

	//For terrains
	virtual ITrrnBrushLib *CreateTrrnBrushLib()=0;

	//will add ref-count for mapfile,brlib internally
	virtual ITrrnMap *CreateTrrnMap(IMapFile *mapfile,ITrrnBrushLibRT*brlib,TrrnParam &desc)=0;
	virtual BOOL NewTrrnMap(IMapFile *mf,ITrrnBrushLib *brlib,DWORD baseid)=0;//baseid的类型为BrushID

	//For AssetSystem
	virtual IAssetSystem *CreateAssetSystem(IMapFile *mapfile,IAssetPackage *pAPs,IPhysicsSystem *pPS,i_math::vector3df &center)=0;

	virtual IEntitySystem *CreateEntitySystem(IAssetSystem *pAS,IAssetSystem *pLabAS,IGameSystem *pGL,IClient *pClient)=0;
	
	virtual BOOL GetBrushLibRes(const char * pathLib,std::vector<std::string> &pathRes) = 0;//pathLib为full path,返回的为相对路径

	virtual void *GetDebugData()=0;
	virtual void SetDebugData(void *data)=0;
};


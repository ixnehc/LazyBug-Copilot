/********************************************************************
created:	2008/1/16   12:31
filename: 	e:\IxEngine\Interfaces\WorldSystem\IAssetSystemDefines.h
author:		cxi

purpose:	exposed asset system defines for the asset system user
*********************************************************************/

#pragma once

#include "anim/animbase.h"

#include "fastdelegate/FastDelegate.h"

#include "IDestroyCache.h"



class IAsset;
class IAssetRenderer;
class IAssetEventer;
class IAssetShell;
class IAssetBodyMap;

class IAnimNodes;

class IMapFile;

class IAssetSystem;
class IWorldSystem;
class IAssetPackage;

class IPhysicsSystem;
class IPhysWorld;

class IClient;
class IGameSystem;
class IDebugger;

class IStaticLightUtil;

class IBrushLib;

class CDataPacket;


//////////////////////////////////////////////////////////////////////////
//Asset Bit(bits used in the AssetSystem internally)
typedef WORD AssetBit;
#define AssetBit_Alive 1//the asset is alive
#define AssetBit_PostCreated 2//已经调过这个Asset的OnPostCreate() 了
#define AssetBit_Shell 16//the asset is in the shell management
#define AssetBit_Dying 32//the asset is going to be destroyed
#define AssetBit_Clock 64//这个asset是否需要定时器
#define AssetBit_ClockDiscarded 128
#define AssetBit_Desc 256//表示这个Asset是被创建用于得到desc,Asset可以根据这个标志来选择做或不做一些事情,注意,这个标志在CAsset::OnCreate()后才开始有效(在OnCreate()中无效)
#define AssetBit_Enum 32768//temply enumerated 


//////////////////////////////////////////////////////////////////////////
//AssetFlag (Note: Asset's flag could NEVER be modified after the asset is created)
typedef DWORD AssetFlag;
#define AstFlag_Motive 1//this asset will move itself
#define AstFlag_Giant 2//this asset is very very big
#define AstFlag_Global 4//这个asset是全局的(比如地表,ocean..)

//////////////////////////////////////////////////////////////////////////
//AssetUID
typedef DWORD AssetUID;
#define AssetUID_Null (0)


//////////////////////////////////////////////////////////////////////////
//
#define LOGICFRAME_DELTATIME (ANIMTICK_PER_SECOND/20)


//////////////////////////////////////////////////////////////////////////
//

class IEntity;
struct AssetCreateArg
{
	IAsset *parent;
	IEntity *owner;//这个asset属于哪个entity
};

//
typedef fastdelegate::FastDelegate2<const char *,const char *> AstDebugOutputDlgt;
typedef fastdelegate::FastDelegate1<IAsset *,const char *> AstGetDebugLocationDlgt;//这个delegate用来根据asset找到它所在的位置



//////////////////////////////////////////////////////////////////////////
//AssetSystemState:Each AssetSystemState repersents a totally independent AssetSystem


class IRenderSystem;
class ITextureMgr;
class IWTextureMgr;
class IMeshMgr;
class IMtrlMgr;
class IMatrice43Mgr;
class IDummiesMgr;
class IAnimMgr;

class IKinectDevice;

class ICompositorManager;

class IRatomsDefault;
class IRatomsLiquid;
class IRatomsTrrn;
class IRatomsShell;
class IRatomsTree;
class IRatomsSky;
class IRatomsPatches;
class IRatomsHelper;
class IRatomVegetable;
class IRatomsGrass;
class IRatomsShore;
class IRatomsHalo;
class IRatomsRoad;
class IRatomsDefer;
class IRatomsDecal;
class IRatomsLichen;

//XXXXX:more ragent


class CConfig;
struct AssetSystemState
{
	AssetSystemState()
	{
		Zero();
	}

	void Zero()
	{
		adr=NULL;
		eventer=NULL;
		shell=NULL;
		bodymap=NULL;

		ans=NULL;

		ratomsDefault=NULL;
		ratomsLiquid=NULL;
		ratomsTrrn=NULL;
		ratomsShell=NULL;
		ratomsTree=NULL;
		ratomsSky=NULL;
		ratomsPatches=NULL;
		ratomsHelper=NULL;
		ratomsVegetable_Obsolete = NULL;
		ratomsGrass=NULL;
		ratomsShore = NULL;
		ratomsHalo = NULL;
		ratomsRoad = NULL;
		ratomsDefer=NULL;
		ratomsDecal=NULL;
		ratomsLichen=NULL;
		//XXXXX:more ragent

		mf=NULL;

		pAS=NULL;//if this is NULL, the AssetSystemState is not valid
		pLabAS=NULL;
		pWS=NULL;
		pAPs=NULL;

		pPS=NULL;
		worldPhys=NULL;

		cfg=NULL;

		pClient=NULL;
		pGS=NULL;
		dlgtDebugOutput=NULL;
		dlgtGetDebugLocation=NULL;

		pRS=NULL;
		texmgr=NULL;
		wtexmgr=NULL;
		meshmgr=NULL;
		mtrlmgr=NULL;
		dmmgr=NULL;
		animmgr=NULL;
		sptLib = NULL;
		slu= NULL;

		pKD=NULL;

		pCompositorMgr = NULL;

		tLast=t=0;
		tFrame=0;
		tFrameF=0.0f;
		tAdjustF=0.0f;
		center.set(-100000,-100000,-100000);

		bEditMode=TRUE;
		bLab=FALSE;

		bInProgress=FALSE;
		tProgress=0.0f;

		bPaused=FALSE;
		nResumeFrames=0xffffffff;
	}

	BOOL IsCentered()
	{
		if ((center.x<=-100000)&&(center.y<=-100000)&&(center.z<=-100000))
			return FALSE;
		return TRUE;
	}

	BOOL IsEditMode()	{		return bEditMode;	}
	BOOL IsValid()	{		return pAS==NULL;	}
	void EnableEditMode(BOOL bEnable)
	{
		if(bEnable)
			ResetTime();
		bEditMode=bEnable;
	}
	BOOL IsInProgress()	{		return bInProgress;	}
	void SetProgress(BOOL bProgress)
	{
		if (bProgress==bInProgress)
			return;
		bInProgress=bProgress;
		tProgress=0.0f;
	}

	void ResetTime()
	{
		tLast=t=0;
		tFrame=0;
		tFrameF=0.0f;
	}
	IAssetRenderer *adr;
	IAssetEventer *eventer;
	IAssetShell *shell;
	IAssetBodyMap*bodymap;

	CConfig *cfg;
	
	IBrushLib * sptLib;

	IAnimNodes *ans;
	IStaticLightUtil* slu;

	//for ragents
	IRatomsDefault *ratomsDefault;
	IRatomsLiquid *ratomsLiquid;
	IRatomsTrrn *ratomsTrrn;
	IRatomsShell *ratomsShell;
	IRatomsTree  *ratomsTree;
	IRatomsSky *ratomsSky;
	IRatomsPatches *ratomsPatches;
	IRatomsHelper *ratomsHelper;
	IRatomVegetable *ratomsVegetable_Obsolete;
	IRatomsGrass *ratomsGrass;
	IRatomsShore * ratomsShore;
	IRatomsHalo * ratomsHalo;
	IRatomsRoad * ratomsRoad;
	IRatomsDefer*ratomsDefer;
	IRatomsDecal*ratomsDecal;
	IRatomsLichen*ratomsLichen;
	//XXXXX:more ragent


	IMapFile *mf;

	IAssetSystem *pAS;
	IAssetSystem *pLabAS;
	IWorldSystem *pWS;
	IAssetPackage *pAPs;

	IPhysicsSystem *pPS;
	IPhysWorld *worldPhys;

	//render system 
	IRenderSystem *pRS;
	ITextureMgr*texmgr;
	IWTextureMgr*wtexmgr;
	IMeshMgr *meshmgr;
	IMtrlMgr*mtrlmgr;
	IDummiesMgr *dmmgr;
	IAnimMgr *animmgr;

	IKinectDevice *pKD;
	
	IGameSystem *pGS;
	IClient *pClient;
	
	ICompositorManager * pCompositorMgr;

	AstDebugOutputDlgt dlgtDebugOutput;
	AstGetDebugLocationDlgt dlgtGetDebugLocation;
	

	DestroyCache<IAsset,AssetBit_Dying> dc;


	i_math::vector3df center;//center of the asset system,in meter

	BOOL bEditMode;
	BOOL bLab;//表示这个AssetSystem是不是一个Lab AssetSystem(实验场AS)

	BOOL bInProgress;
	float tProgress;

	AnimTick tLast;//上一个逻辑帧的时间
	AnimTick t;//逻辑帧时间
	AnimTick tFrame;//常规帧时间
	float tFrameF;////常规帧时间,以秒为单位

	float tAdjustF;//表示更新的速度要调整来弥补的时差,正值表示,更新速度要加快,负值表示要减慢

	BOOL bPaused;
	DWORD nResumeFrames;

};




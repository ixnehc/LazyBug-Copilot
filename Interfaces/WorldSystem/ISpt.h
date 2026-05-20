/********************************************************************
	created:	2009/02/23
	created:	23:2:2009   21:00
	filename: 	d:\IxEngine\Interfaces\WorldSystem\ISptDefines.h
	file path:	d:\IxEngine\Interfaces\WorldSystem
	file base:	ISptDefines
	file ext:	h
	author:		cxi
	
	purpose:	exposed speed tree related defines
*********************************************************************/
#pragma once

#include "math/vector3d.h"

#include "math/vector2d.h"

#include "math/aabbox3d.h"

#include "math/matrix43.h"

#include "gds/GObj.h"

#include "fastdelegate/FastDelegate.h"

#include "WorldSystem/IObjMap.h"

#include "ref/sharedPtr.h"

#include "WorldSystem/IBrushLib.h"

class IFileSystem;
class IRenderSystem;
class IUtilRS;

// spt forest
//////////////////////////////////////////////////////////////////////////
#define SPTHANDLE_INVALID  0x8fffffff

struct  ForestConfig
{
	ForestConfig()
	{
		w = 200;
		h = 200;
		owner = NULL;
	}
	DWORD w,h;
	void * owner;
};

class ICamera;
struct ForestDrawArg
{
	ForestDrawArg()
	{
		bNormalMap = true;
		bWind = true;
		camera = NULL;
	}
	bool bNormalMap;
	bool bWind;
	ICamera * camera;
	DWORD tickTime;		//in millsecond time unit.
};

class IBrushLib;

typedef DWORD HForrestTree;
#define HForrestTree_Null 0


#define NUM_WIND_MATRIXS 6
#define NUM_WIND_ANGLES  8


struct TreeInfo
{
	i_math::vector3df pos;
	float rotY;
	float scale;
	i_math::aabbox3df aabb;		//runtime calc when  touch to scene
	BRUID refModel; 
	i_math::matrix43f GetTransform() const
	{
		i_math::matrix43f mat;
		mat.setScale(scale,scale,scale);
		mat.addRotationY(rotY);
		mat.addTranslation(pos);
		return mat;
	}
};

//////////////////////////////////////////////////////////////////////////

class IWorldSystem;
struct NodeTreeRef;
typedef void * NodeHandle;
class ISpt;
struct SptShadowVtx;
class IWorldEvent;
class IStaticLightUtil;
class CProgress;

struct SptWndCfg;

typedef void * HForestWind;

class HitProbe;
struct SptWndCfg;
struct GlobalRenv;

class IBrushLib;
class IForestEditor : public IObjMapEditor
{
public:
	virtual  HMapObj AddTree(const TreeInfo & tree) =0;
	virtual  HMapObj SetTreeInfo(const HMapObj & hObj,const TreeInfo &info) = 0;
	virtual	 const TreeInfo * GetTreeInfo(const HMapObj & hObj) = 0;

	//shadow 
	virtual  BOOL  SetShadowLvl(const HMapObj & hObj,int lvl) = 0;
	virtual  int   GetShadowMapSize(const HMapObj & hObj) = 0;
	
	
	virtual IBrushLib * GetWindLib() = 0;

	virtual IBrushLib * GetSptLib() = 0;

	virtual  GlobalRenv * GetGlobalEnv() = 0;	

	virtual BOOL CheckVisible(const HMapObj &hObj) = 0;	//清除 无效不可见对象
};

struct SptDrawParam{
	SptDrawParam(void)
	{
		pos.set(0,0,0);
		rotY = 0;
		scale = 1.0f;
		dist = 0;
		bShowLeafCard = bShowLeafMesh = bShowTrunk = bShowFrond = TRUE;
	}
	i_math::vector3df pos;
	float rotY;
	float scale;
	float dist;
	BOOL bShowLeafCard;
	BOOL bShowLeafMesh;
	BOOL bShowTrunk;
	BOOL bShowFrond;
};

class IRenderPort;
class ISptDrawer
{
public:
	INTERFACE_REFCOUNT;
	virtual void Draw(ISpt *pSpt,SptDrawParam *param = NULL) = 0;
	virtual void SetRP(IRenderPort * rp) = 0;
};





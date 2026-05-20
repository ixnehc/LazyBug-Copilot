/********************************************************************
	created:	1:3:2009   9:15
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		cxi
	
	purpose:	defines & interfaces for Asset Eventer
*********************************************************************/
#pragma once

#include "class/class.h"

#include "gds/GObj.h"

#include "fastdelegate/FastDelegate.h"

#include "anim/animdefines.h"

#include "strlib/strlibdefines.h"

struct GObjBase;
struct AstEvent
{
	virtual CClass *GetClass()=0;
	virtual const char *GetName()=0;
	virtual DWORD GetID()=0;
	virtual GObjBase *GetGObj()	{		return NULL;	}

	AstEvent *Clone()
	{
		AstEvent*p=(AstEvent*)GetClass()->New();
		p->GetGObj()->Copy(GetGObj());
		return p;
	}
};


#define DEFINE_EVENT(eclass)																					\
	virtual const char *GetName()	{		return #eclass;	}											\
	virtual DWORD GetID(){	return ID_##eclass;}															\
	DEFINE_CLASS(eclass)


#define IMPLEMENT_EVENT(eclass) IMPLEMENT_CLASS(eclass)


//Note:the hook handler should return TRUE to let other hook handler continue to
//handle.
typedef fastdelegate::FastDelegate1<AstEvent&,BOOL> HookHandler;

#define HookPriorityLow(v) (0+(v))
#define HookPriorityMedium(v) (100+(v))
#define HookPriorityHi(v) (200+(v))
#define HookPriorityDefault HookPriorityMedium(50)

#define MAX_CLOCK 12

#define CLOCKCYCLE_TICK (ANIMTICK_PER_SECOND/20)	//50 ms





//////////////////////////////////////////////////////////////////////////
//All the events id
enum AstEventID
{
	ID_ENone=0,

	//Hook Event ID

	ID_HkCoreStart=1,

	ID_HkSetWorldCenter,
	ID_HkSaveMap,
	ID_HkUnLoadMap,
	ID_HkLoadMap,
	ID_HkReloadMap,
	ID_HkReloadTrrnBrushLib,
	ID_HkGetTerrain,
	ID_HkGetGlobalEvn,
	ID_HkSetCtrlOp,
	ID_HkGetCamera,
	ID_HkPostChangeMap,
	ID_HkChangeTrrnHeight,
	ID_HkResetConfig,
	ID_HkProgress,
	ID_HkGetEnvLight,
	ID_HKGetObjMapEditor,
	ID_HkGetWaterEnv,
	ID_HKGetNavService,
	ID_HkGetCameraAddOn,

	ID_HkUserStart=128,

	MAX_HOOKEVENT_ID=512,


	//General Event ID
	ID_EVENT_START=2048,

	ID_ETest,

	MAX_EVENT_ID,
};





//////////////////////////////////////////////////////////////////////////
//Hook Event

class CProgress;
struct HkSetWorldCenter:public AstEvent
{
	i_math::vector3df center;
	CProgress *prg;

	DEFINE_EVENT(HkSetWorldCenter);

	// GObj Defination --------------------------------------------------
    BEGIN_GOBJ_PURE(HkSetWorldCenter,1);
		GELEM_VAR_INIT(i_math::vector3df,center,i_math::vector3df(0,0,0));
		GELEM_VAR_INIT(CProgress *,prg,NULL);
    END_GOBJ();    

};

struct HkSaveMap:public AstEvent
{

	DEFINE_EVENT(HkSaveMap);

	// GObj Defination --------------------------------------------------
    BEGIN_GOBJ_PURE(HkSaveMap,1);
    END_GOBJ();    
};

struct HkUnLoadMap:public AstEvent
{
	DEFINE_EVENT(HkUnLoadMap);

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(HkUnLoadMap,1);
	END_GOBJ();    
};

class CProgress;
struct HkLoadMap:public AstEvent
{
	DEFINE_EVENT(HkLoadMap);

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(HkLoadMap,1);
		GELEM_VAR_INIT(CProgress *,prg,NULL);
	END_GOBJ();    
	CProgress *prg;
};


struct HkReloadMap:public AstEvent
{
	BOOL bReloadAll;
	std::vector<i_math::pos2di>blks;

	DEFINE_EVENT(HkReloadMap);

	// GObj Defination --------------------------------------------------
    BEGIN_GOBJ_PURE(HkReloadMap,1);
		GELEM_VAR_INIT(BOOL,bReloadAll,FALSE);
		GELEM_VARVECTOR(i_math::pos2di,blks);
    END_GOBJ();    
};

struct HkReloadTrrnBrushLib:public AstEvent
{
	DEFINE_EVENT(HkReloadTrrnBrushLib);

	// GObj Defination --------------------------------------------------
    BEGIN_GOBJ_PURE(HkReloadTrrnBrushLib,1);
    END_GOBJ();    
};


class ITrrnMap;
class ITrrnBrushLib;
struct HkGetTerrain:public AstEvent
{
	ITrrnMap *trrn;
	ITrrnBrushLib *lib;

	DEFINE_EVENT(HkGetTerrain);

	// GObj Defination --------------------------------------------------
    BEGIN_GOBJ_PURE(HkGetTerrain,1);
		GELEM_VAR_INIT(ITrrnMap*,trrn,NULL);
		GELEM_VAR_INIT(ITrrnBrushLib*,lib,NULL);
    END_GOBJ();    
};


class IObjMapEditor;
struct HKGetObjMapEditor: public AstEvent
{
	IObjMapEditor* editor;
	DWORD typeEditor;

	DEFINE_EVENT(HKGetObjMapEditor);
	
	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HKGetObjMapEditor,1);
		GELEM_VAR_INIT(IObjMapEditor *,editor,NULL);
		GELEM_VAR_INIT(DWORD,typeEditor,0);
	END_GOBJ();
};

class INavService;
struct HKGetNavService :public AstEvent
{
	INavService * service;
	DEFINE_EVENT(HKGetNavService);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HKGetNavService,1);
		GELEM_VAR_INIT(HKGetNavService *,service,NULL);
	END_GOBJ();
};

struct CtrlOp;
struct HkSetCtrlOp:public AstEvent
{
	CtrlOp *op;
	DEFINE_EVENT(HkSetCtrlOp);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkSetCtrlOp,1);
		GELEM_VAR_INIT(CtrlOp *,op,NULL);
	END_GOBJ();
};

class ICamera;
struct HkGetCamera:public AstEvent
{
	AnimTick t;//得到哪个时间的cam
	ICamera*cam;

	DEFINE_EVENT(HkGetCamera);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkGetCamera,1);
		GELEM_VAR_INIT(AnimTick,t,0);
		GELEM_VAR_INIT(ICamera*,cam,NULL);
	END_GOBJ();

};

struct HkGetCameraAddOn:public AstEvent
{
	AnimTick t;//得到哪个时间的cam
	ICamera*cam;

	i_math::vector3df eye;
	i_math::vector3df at;
	float wt;

	DEFINE_EVENT(HkGetCameraAddOn);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkGetCameraAddOn,1);
		GELEM_VAR_INIT(AnimTick,t,0);
		GELEM_VAR_INIT(ICamera*,cam,NULL);
		GELEM_VAR(i_math::vector3df ,eye);
		GELEM_VAR(i_math::vector3df ,at);
		GELEM_VAR_INIT(float,wt,0.0f);
	END_GOBJ();

};



struct HkPostChangeMap:public AstEvent
{
	CProgress *progress;

	DEFINE_EVENT(HkPostChangeMap);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkPostChangeMap,1);
		GELEM_VAR_INIT(CProgress *,progress,NULL);
	END_GOBJ();

};

class ITrrnMapEditor;
struct HkChangeTrrnHeight:public AstEvent
{
	DEFINE_EVENT(HkChangeTrrnHeight);

	ITrrnMapEditor * editor;
	std::vector<i_math::pos2di> blks;
	BOOL bSave;
	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkChangeTrrnHeight,1);
		GELEM_VAR_INIT(ITrrnMapEditor *,editor,NULL);
		GELEM_VAR_INIT(BOOL,bSave,FALSE);
		GELEM_VARVECTOR(i_math::pos2di,blks);
	END_GOBJ();
};

struct HkResetConfig:public AstEvent
{
	DEFINE_EVENT(HkResetConfig);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkResetConfig,1);
	END_GOBJ();
};

struct HkProgress:public AstEvent
{
	DEFINE_EVENT(HkProgress);

	enum Type
	{
		Start,//进度开始
		Clock,//进度中
		End,//进度结束
	};

	std::string name;//进度名称
	Type tp;
	float cur;//当前值,tp为Clock时有效,取值范围为[0.0,1.0]

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkProgress,1);
		GELEM_STRING(name);
		GELEM_VAR_INIT(Type,tp,Start);
		GELEM_VAR_INIT(float,cur,0.0f);
	END_GOBJ();

};

class IEnvLight;
struct HkGetEnvLight:public AstEvent
{
	IEnvLight*el;

	DEFINE_EVENT(HkGetEnvLight);

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(HkGetEnvLight,1);
	GELEM_VAR_INIT(IEnvLight*,el,NULL);
	END_GOBJ();    
};

struct WaterEnv;
struct HkGetWaterEnv:public AstEvent
{
	DEFINE_EVENT(HkGetWaterEnv);

	HkGetWaterEnv()
	{
		cam=NULL;
		env=NULL;
	}

	//参数
	ICamera *cam;

	//结果
	WaterEnv *env;
};

//////////////////////////////////////////////////////////////////////////
//General Events

struct ETest:public AstEvent
{
	int t1;
	std::string t2;

	DEFINE_EVENT(ETest);

	// GObj Defination --------------------------------------------------
    BEGIN_GOBJ_PURE(ETest,1);
		GELEM_VAR_INIT(int,t1,0);
		GELEM_STRING_INIT(t2,"ttt");
    END_GOBJ();    
};


class IAsset;
class IAssetEventer
{
public:
	void IncreaseTime(float fDelta);//in second
	virtual BOOL SendEvent(AstEvent &e,IAsset *ast)=0;
	virtual BOOL SendEvent(AstEvent &e,IAsset **asts,DWORD nAst)=0;

	virtual BOOL SendHook(AstEvent &e)=0;
	virtual BOOL SendHook(AstEvent &e,DWORD subid)=0;
	virtual BOOL RegisterHook(DWORD eid,IAsset *ast,HookHandler &dlgt,
		DWORD prior=HookPriorityMedium(50))=0;
	virtual BOOL RegisterHook(DWORD eid,DWORD subid,IAsset *ast,HookHandler &dlgt,
		DWORD prior=HookPriorityMedium(50))=0;

	//clock以CLOCKCYCLE_TICK 为周期调用
	//对于某个asset来说,在它的生命周期内,第一次调用RegisterClock(..)后,系统会保证这个asset在同一逻辑帧内的稍后时间收到一个OnClock()
	//但对之后的RegisterClock(),系统不能保证在同一逻辑帧内肯定收到OnClock()
	virtual BOOL RegisterClock(IAsset *ast)=0;
	virtual BOOL UnRegisterClock(IAsset *ast)=0;
	virtual void FlushNewClocks()=0;

	virtual void SendSignal(StringID nm,i_math::vector2df &pos,float radius)=0;
	virtual BOOL CheckSignal(StringID nm,i_math::vector2df &pos, AnimTick tSignalMin=0)=0;
	virtual void ClearAllSignals()=0;

};

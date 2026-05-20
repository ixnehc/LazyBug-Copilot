#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#include "spline/CubicSpline.h"




#define CLASSUID_LanternRoute 60



struct EoParamLanternRoute:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamLanternRoute);

	BEGIN_GOBJ_PURE(EoParamLanternRoute,1);

		GELEM_VAR_INIT(AnimTick,durBirth,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("出生时长",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"出生时长");
		GELEM_VAR_INIT(float,spdOpen,5.0f);
			GELEM_EDITVAR("打开移动速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"打开移动速度");
		GELEM_VAR_INIT(float,spdClose,10.0f);
			GELEM_EDITVAR("关闭移动速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"关闭移动速度");
		GELEM_VAR_INIT(unsigned __int64,mark,0);
			GELEM_EDITVAR("标记Proto",GVT_Bx8,GSem_ProtoPath,"标记Proto");
		GELEM_VAR_INIT( StringID,nmOpenSignal,StringID_Invalid);	
			GELEM_EDITVAR( "开门信号", GVT_U, GSem(GSem_StringID,"信号名称"), "开门信号" );
		GELEM_VAR_INIT( StringID,nmCloseSignal,StringID_Invalid);	
			GELEM_EDITVAR( "关门信号", GVT_U, GSem(GSem_StringID,"信号名称"), "关门信号" );

	END_GOBJ();

	AnimTick durBirth;
	float spdOpen;
	float spdClose;
	unsigned __int64 mark;
	StringID nmOpenSignal;
	StringID nmCloseSignal;


};

class EoLanternRoute:public CLoEffectObj
{
public:
	EoLanternRoute()
	{
		_bSyncDirty=FALSE;
		_idPathRes=RecordID_Invalid;
		_distPath=0.0f;
		_idPortal=LevelObjID_Invalid;
	}
	DEFINE_LEVELOBJ_CLASS(EoLanternRoute,CLASSUID_LanternRoute);

	struct State
	{
		State()
		{
			stage=None;
		}

		enum Stage
		{
			None,
			Birth,
			Opening,
			Aborting,
			Opened,
			Closing,
			Closed,
		};
		Stage stage;
		AnimTick tStageStart;

	};

	virtual const char *GetShowName()	{		return "灯塔路线";	}

	void SetPath(RecordID idPathRes);
	void SetPortalID(LevelObjID idPortal)	{		_idPortal=idPortal;	}

	void Stop();
	BOOL IsOpened()
	{
		return _state.stage==State::Opened;
	}

protected:

	void _OnPostCreate()override;
	void OnDestroy()override;


	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	void _OnPostWriteSync() override;

	void _OnUpdate()override;
	virtual BOOL _NeedOps()	{		return FALSE;	}

	void _WriteState(CBitPacket *bp);

	void _SetStage(State::Stage stage);

	State _state;
	BOOL _bSyncDirty;

	LevelObjID _idPortal;
	RecordID _idPathRes;
	float _distPath;

};

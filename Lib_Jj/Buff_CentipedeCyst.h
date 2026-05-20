#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"

#include "LevelStrike.h"

#include "behaviorgraph/BehaviorParam.h"

struct BuffParam_CentipedeCyst:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_CentipedeCyst);

	BEGIN_GOBJ_PURE(BuffParam_CentipedeCyst,1);

		GELEM_VAR_INIT(unsigned __int64,grow,0);
			GELEM_EDITVAR("生长Proto",GVT_Bx8,GSem_ProtoPath,"生长Proto");
		GELEM_VAR_INIT(unsigned __int64,activating,0);
			GELEM_EDITVAR("激活Proto",GVT_Bx8,GSem_ProtoPath,"激活Proto");

		GELEM_VAR_INIT(AnimTick,durGrow,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("生长时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"生长时间");

		GELEM_VAR_INIT(AnimTick,durWaitMin,ANIMTICK_FROM_SECOND(4.0f));
			GELEM_EDITVAR("生长后最小等待时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"生长后最小等待时间");
		GELEM_VAR_INIT(AnimTick,durWaitMax,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("生长后最大等待时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"生长后最大等待时间");

		GELEM_VAR_INIT(AnimTick,durActivating,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("激活时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"激活时间");

	END_GOBJ();

	unsigned __int64 grow;
	unsigned __int64 activating;
	AnimTick durGrow;
	AnimTick durWaitMin;
	AnimTick durWaitMax;
	AnimTick durActivating;
};


struct BuffArg_CentipedeCyst:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_CentipedeCyst);
	BuffArg_CentipedeCyst()
	{
	}
};

class Buff_CentipedeCyst:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_CentipedeCyst,47)

	Buff_CentipedeCyst()
	{
		_flag=BuffFlag_GhostCollide|BuffFlag_NotAttackable;
		_stage=Stage_None;
	}

	enum Stage
	{
		Stage_None,
		Stage_Growing,
		Stage_Waiting,
		Stage_Activating,
		Stage_Exploding,
	};

	static BuffFlag CalcFlag(AnimTick tAge,AnimTick durGrow,Stage stage);


	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端

	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnDestroy() override;
	virtual void _OnUpdate(AnimTick dt);
	virtual void HandleEvent(LevelEvent &e);

	//Factor Overriding
	BuffFlag GetFlags()	{		return _flag;	}

protected:

	void _SetStage(Stage stage);

	virtual void _WriteData(CBitPacket *dp);

	void _SetFlag(BuffFlag flag)
	{
		if (_flag!=flag)
		{
			_flag=flag;
			_buffs->MarkFlagsDirty();
		}
	}
	BuffFlag _flag;

	Stage _stage;
	AnimTick _durWaiting;

};


#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"

#include "LevelStrike.h"


struct BuffParam_Possess:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Possess);

	BEGIN_GOBJ_PURE(BuffParam_Possess,1);

		GELEM_VAR_INIT(unsigned __int64,effect,0);
			GELEM_EDITVAR("效果",GVT_Bx8,GSem_ProtoPath,"效果的Proto");
		GELEM_VAR_INIT(AnimTick,durEnter,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("进入时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"进入宿主体内时间");
	END_GOBJ();

	unsigned __int64 effect;
	AnimTick durEnter;
};


struct BuffArg_Possess:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Possess);
	BuffArg_Possess()
	{
		idTarget=LevelObjID_Invalid;
	}
	LevelObjID idTarget;
};

class Buff_Possess:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Possess,43)

	Buff_Possess()
	{
		_idTarget=LevelObjID_Invalid;
		_bEntered=FALSE;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端

	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnDestroy() override;
	virtual void _OnUpdate(AnimTick dt);

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_Pausing|BuffFlag_GhostCollide|BuffFlag_Invisible;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	CLevelObjPauser _pauser;

	LevelObjID _idTarget;

	BOOL _bEntered;

};


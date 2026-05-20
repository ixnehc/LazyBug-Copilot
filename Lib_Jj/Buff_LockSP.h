#pragma once

#include "LevelBuff.h"

struct BuffParam_LockSP:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_LockSP);

	BEGIN_GOBJ_PURE(BuffParam_LockSP,1);

		GELEM_VAR_INIT(AnimTick,durLock,ANIMTICK_FROM_SECOND(120.0f));
			GELEM_EDITVAR("LockSP时长",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"LockSP时长");
	END_GOBJ();

	AnimTick durLock;

};


struct BuffArg_LockSP
{
	DEFINE_CLASS(BuffArg_LockSP)
};


class Buff_LockSP:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_LockSP,51)

	Buff_LockSP()
	{
		_t=0;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//时间到结束时是否需要同步给客户端

	BOOL IsInLock();

	virtual void _OnCreate(LevelBuffArg *param)
	{
	}

	virtual void _OnUpdate(AnimTick dt);
	virtual void _WriteData(CBitPacket *dp) override;
	virtual void LoadTeleport(CLevelBuff *buffOrg);

	virtual void HandleEvent(LevelEvent &e);

public:
	AnimTick _t;


};


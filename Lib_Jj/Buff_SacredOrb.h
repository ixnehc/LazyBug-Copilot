#pragma once

#include "LevelBuff.h"

struct BuffParam_SacredOrb:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_SacredOrb);

	BEGIN_GOBJ_PURE(BuffParam_SacredOrb,1);
		GELEM_VAR_INIT(unsigned __int64,idTargetHit,0);
			GELEM_EDITVAR("目标命中效果",GVT_Bx8,GSem_ProtoPath,"目标命中效果");

	END_GOBJ();

	unsigned __int64 idTargetHit;

};


struct BuffArg_SacredOrb:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_SacredOrb)
};


class Buff_SacredOrb:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_SacredOrb,61)

	Buff_SacredOrb()
	{
		_bTriggered=FALSE;
		_bFired=FALSE;
		_tRecentCharged=ANIMTICK_INFINITE;
		_tRecentFire=ANIMTICK_INFINITE;
	}

	BOOL NeedSync() override	{		return TRUE;	}//是否需要同步给客户端


	void _OnCreate(LevelBuffArg *param) override
	{
	}

	BOOL CanDispel();

	BOOL IsCharged()	{		return _IsCharged();	}

protected:

	void HandleEvent(LevelEvent &e) override;

	void _OnUpdate(AnimTick dt) override;

	BOOL _IsCharged();

	BOOL _CanTrigger();

	BOOL _bTriggered;
	BOOL _bFired;

	AnimTick _tRecentCharged;
	AnimTick _tRecentFire;


};


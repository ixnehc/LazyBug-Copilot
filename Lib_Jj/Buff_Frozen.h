#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

struct BuffParam_Frozen:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Frozen);

	BEGIN_GOBJ_PURE(BuffParam_Frozen,1);

	END_GOBJ();
};


class Buff_Frozen:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Frozen,3)

	Buff_Frozen()
	{
		_idBroken=LevelSkillID_Invalid;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端


	virtual void _OnCreate(LevelBuffArg *param);

	BuffFlag GetFlags()	{		return BuffFlag_Pausing|BuffFlag_PausingAnim;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	LevelSkillID _idBroken;

};

struct BuffArg_Frozen:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Frozen);
};
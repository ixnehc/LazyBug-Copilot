#pragma once

#include "LevelBuff.h"

struct BuffParam_Petrified:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Petrified);

	BEGIN_GOBJ_PURE(BuffParam_Petrified,1);

	END_GOBJ();
};


class Buff_Petrified:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Petrified,18)

	Buff_Petrified()
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

struct BuffArg_Petrified:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Petrified);
};
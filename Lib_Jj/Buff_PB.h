#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"

#include "LevelStrike.h"


struct BuffParam_PB:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_PB);

	BEGIN_GOBJ_PURE(BuffParam_PB,1);

	END_GOBJ();
};


struct BuffArg_PB:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_PB);

	LevelPos posTarget;
	LevelFace face;
};

class Buff_PB:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_PB,27)

	Buff_PB()
	{
		_idBroken=LevelSkillID_Invalid;
		_idTeleport=LevelTeleportID_Invalid;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端


	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param);

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_Pausing;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	LevelSkillID _idBroken;
	LevelPos _target;//击退到的位置
	float _face;//击退到的朝向
	LevelTeleportID _idTeleport;


	CLevelObjPauser _pauser;


};


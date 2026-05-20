#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"

#include "LevelStrike.h"


struct BuffParam_KB:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_KB);

	BEGIN_GOBJ_PURE(BuffParam_KB,1);

	END_GOBJ();
};


struct BuffArg_KB:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_KB);
	LevelStrike strike;
};

class Buff_KB:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_KB,4)

	Buff_KB()
	{
		_idBroken=LevelSkillID_Invalid;
		_idTeleport=LevelTeleportID_Invalid;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端

	virtual LevelBuffMask GetForbiddingBuffs();
	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnDestroy() override;

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_Pausing;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	LevelSkillID _idBroken;
	LevelPos _target;//击退到的位置
	float _face;//击退到的朝向
	LevelTeleportID _idTeleport;

	LevelStrike _strike;

	CLevelObjPauser _pauser;


};


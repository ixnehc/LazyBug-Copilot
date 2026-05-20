#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"

#include "LevelStrike.h"


struct BuffParam_Blocking:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Blocking);

	BEGIN_GOBJ_PURE(BuffParam_Blocking,1);

	END_GOBJ();
};


struct BuffArg_Blocking:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Blocking);
	LevelStrike strike;
};

class Buff_Blocking:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Blocking,36)

	Buff_Blocking()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端

	virtual LevelBuffMask GetForbiddingBuffs();
	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnDestroy() override;

	//Factor Overriding
	BuffFlag GetFlags()	{		return 0;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	float _face;//朝向

	LevelStrike _strike;

};


#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"
#include "LevelStrike.h"

struct BuffParam_Invisible:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Invisible);

	BEGIN_GOBJ_PURE(BuffParam_Invisible,1);


	END_GOBJ();

};

struct BuffArg_Invisible:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Invisible);
};

struct BuffData_Invisible
{
	BuffData_Invisible()
	{
	}
	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);
};



class Buff_Invisible:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Invisible,21)

	Buff_Invisible()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Invisible的Buff并存的情况


	virtual LevelBuffMask GetReplaceBuffs();

	void _OnCreate(LevelBuffArg *param) override;
	void _OnDestroy() override;

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_NotAttackable|BuffFlag_GhostCollide|BuffFlag_Invisible;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	BuffData_Invisible _data;
};


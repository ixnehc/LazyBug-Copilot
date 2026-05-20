#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"
#include "LevelStrike.h"

struct BuffParam_GhostCollide:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_GhostCollide);

	BEGIN_GOBJ_PURE(BuffParam_GhostCollide,1);

	END_GOBJ();

};

struct BuffArg_GhostCollide:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_GhostCollide);
};

struct BuffData_GhostCollide
{
	BuffData_GhostCollide()
	{
	}
	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);
};



class Buff_GhostCollide:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_GhostCollide,42)

	Buff_GhostCollide()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个GhostCollide的Buff并存的情况


	virtual LevelBuffMask GetReplaceBuffs();
	virtual LevelBuffMask GetForbiddingBuffs();

	virtual void _OnCreate(LevelBuffArg *param);

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_GhostCollide;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	BuffData_GhostCollide _data;
};


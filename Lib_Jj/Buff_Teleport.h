#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"

struct BuffParam_Teleport:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Teleport);

	BEGIN_GOBJ_PURE(BuffParam_Teleport,1);

	END_GOBJ();

};


struct BuffArg_Teleport:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Teleport);
	LevelPos pos;
	float face;
};

struct BuffData_Teleport
{
	BuffData_Teleport()
	{
		memset(this,0,sizeof(*this));
	}
	LevelSkillID idBroken;//导致那个Skill被中断了
	LevelTeleportID idTeleport;
	LevelPos target;//在哪里
	float face;
};

class Buff_Teleport:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Teleport,13)

	Buff_Teleport()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return FALSE;	}

	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param);

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_Teleport|BuffFlag_GhostCollide|BuffFlag_NotAttackable|BuffFlag_Pausing;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	BuffData_Teleport _data;


};


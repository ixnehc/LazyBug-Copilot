#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"

struct BuffParam_TeleportOut:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_TeleportOut);

	BEGIN_GOBJ_PURE(BuffParam_TeleportOut,1);

		GELEM_VARVECTOR_INIT(unsigned __int64,effect,0);
			GELEM_EDITVAR("效果",GVT_Bx8,GSem_ProtoPath,"传送消失效果");
		GELEM_VARVECTOR_INIT(unsigned __int64,result,0);
			GELEM_EDITVAR("结果",GVT_Bx8,GSem_ProtoPath,"传送消失结果");

	END_GOBJ();

	std::vector<unsigned __int64> effect;
	std::vector<unsigned __int64> result;
};


struct BuffArg_TeleportOut:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_TeleportOut);
};

struct BuffData_TeleportOut
{
	BuffData_TeleportOut()
	{
		memset(this,0,sizeof(*this));
	}
	LevelSkillID idBroken;//导致那个Skill被中断了
	LevelTeleportID idTeleport;
	LevelPos target;//在哪里
	float face;
};

class Buff_TeleportOut:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_TeleportOut,13)

	Buff_TeleportOut()
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

	BuffData_TeleportOut _data;


};


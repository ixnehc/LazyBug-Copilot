#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"


struct BuffParam_ResideHole:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_ResideHole);

	BEGIN_GOBJ_PURE(BuffParam_ResideHole,1);

		GELEM_VAR_INIT(unsigned __int64,residing,0);
			GELEM_EDITVAR("效果",GVT_Bx8,GSem_ProtoPath,"驻留效果");
	END_GOBJ();

	unsigned __int64 residing;
};


struct BuffArg_Reside:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Reside);
	LevelObjID idTarget;
};

struct BuffData_ResideHole
{
	BuffData_ResideHole()
	{
		memset(this,0,sizeof(*this));
	}
	LevelObjID idTarget;
	LevelTeleportID idTeleport;
	LevelSkillID idBroken;
};

class Buff_ResideHole:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_ResideHole,8)

	Buff_ResideHole()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}


	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param);

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_NotAttackable|BuffFlag_Reside|BuffFlag_GhostCollide;	}

	LevelObjID GetResidingTarget()	{		return _data.idTarget;	}

protected:

	virtual void _WriteData(CBitPacket *dp);


	BuffData_ResideHole _data;

};


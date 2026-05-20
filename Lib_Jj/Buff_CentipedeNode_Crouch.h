#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"
#include "LevelStrike.h"

#include "behaviorgraph/BehaviorParam.h"

struct BuffParam_CentipedeNode_Crouch:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_CentipedeNode_Crouch);

	BEGIN_GOBJ_PURE(BuffParam_CentipedeNode_Crouch,1);

		GELEM_BEHAVIORMEM_OBJID(varCentipedeAgent,"记录蜈蚣对象的变量","CentipedeNode_Crouch单位里记录蜈蚣对象的变量");

		GELEM_VAR_INIT(unsigned __int64,crouch,0);
			GELEM_EDITVAR("匍匐效果",GVT_Bx8,GSem_ProtoPath,"变为隐身的效果");
		GELEM_VAR_INIT(unsigned __int64,standup,0);
			GELEM_EDITVAR("站立效果",GVT_Bx8,GSem_ProtoPath,"变为站立的效果");

	END_GOBJ();

	StringID varCentipedeAgent;

	unsigned __int64 crouch;
	unsigned __int64 standup;

};

struct BuffArg_CentipedeNode_Crouch:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_CentipedeNode_Crouch);
};


class Buff_CentipedeNode_Crouch:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_CentipedeNode_Crouch,41)

	Buff_CentipedeNode_Crouch()
	{
		_idAgent=LevelObjID_Invalid;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个CentipedeNode_Crouch的Buff并存的情况


	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param);

	//Factor Overriding
	BuffFlag GetFlags()	{		return 0;}//BuffFlag_NotAttackable;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	LevelObjID _idAgent;

};


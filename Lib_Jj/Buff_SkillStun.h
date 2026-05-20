#pragma once

#include "LevelBuff.h"
#include "LevelStrike.h"

struct BuffArg_SkillStun:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_SkillStun);
	BuffArg_SkillStun()
	{
	}

};

struct BuffParam_SkillStun:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_SkillStun);

	BEGIN_GOBJ_PURE(BuffParam_SkillStun,1);

	END_GOBJ();

};


class Buff_SkillStun:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_SkillStun,39)

	Buff_SkillStun()
	{
		_idBroken=LevelSkillID_Invalid;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUpIP()	{		return TRUE;	}


	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnDestroy() override;

	//Factor Overriding
	BuffFlag GetFlags()	{		return 0;	}


protected:

	virtual void _WriteData(CBitPacket *dp);

	LevelSkillID _idBroken;

};


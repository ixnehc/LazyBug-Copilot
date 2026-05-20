#pragma once

#include "LevelBuff.h"
#include "LevelStrike.h"

struct BuffArg_Stun:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Stun);
	LevelStrike strike;

};

enum StunPlayMode
{
	StunPlay_None,//不播放
	StunPlay_Single,//播放单一动画
	StunPlay_FrontBack,//播放前后动画

	StunPlay_ForceDword=0xffffffff,
};

struct BuffParam_Stun:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Stun);

	BEGIN_GOBJ_PURE(BuffParam_Stun,1);

		GELEM_VAR_INIT(DWORD,mode,StunPlay_Single);
			GELEM_EDITVAR("播放模式",GVT_U,GSem(GSem_Interger,"不播放,播放单一动画,播放前/后动画"),"播放动画模式");

	END_GOBJ();

	StunPlayMode mode;
};


class Buff_Stun:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Stun,1)

	Buff_Stun()
	{
		_idBroken=LevelSkillID_Invalid;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUpIP()	{		return TRUE;	}


	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnDestroy() override;

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_Pausing;	}


protected:

	virtual void _WriteData(CBitPacket *dp);

	LevelSkillID _idBroken;
	LevelStrike _strike;

};


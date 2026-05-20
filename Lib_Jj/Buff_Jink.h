#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"

#include "LevelStrike.h"


struct BuffParam_Jink:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Jink);

	BEGIN_GOBJ_PURE(BuffParam_Jink,1);

		GELEM_VAR_INIT(unsigned __int64,hide,0);
			GELEM_EDITVAR("隐身效果",GVT_Bx8,GSem_ProtoPath,"隐身效果的Proto");
		GELEM_VAR_INIT(unsigned __int64,show,0);
			GELEM_EDITVAR("显身效果",GVT_Bx8,GSem_ProtoPath,"显身效果的Proto");
		GELEM_OBJVECTOR(DealEntry,dealsShow);
			GELEM_EDITOBJ("显身时结算列表","多个结算");
		GELEM_VAR_INIT(unsigned __int64,move,0);
			GELEM_EDITVAR("移动效果",GVT_Bx8,GSem_ProtoPath,"移动效果的Proto");
		GELEM_VAR_INIT(AnimTick,durHide,ANIMTICK_FROM_SECOND(0.2f));
			GELEM_EDITVAR("隐身时长",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"隐身时长");
		GELEM_VAR_INIT(AnimTick,durShow,ANIMTICK_FROM_SECOND(0.4f));
			GELEM_EDITVAR("显身时长",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"显身时长");
		GELEM_VAR_INIT(float,spd,8.0f);
			GELEM_EDITVAR("移动速度",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"移动速度");
	END_GOBJ();

	unsigned __int64 hide;
	unsigned __int64 show;
	unsigned __int64 move;
	float spd;
	AnimTick durHide;
	AnimTick durShow;
	std::vector<DealEntry> dealsShow;
};


struct BuffArg_Jink:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Jink);
	BuffArg_Jink()
	{
		face=0.0f;
	}
	LevelPos pos;
	LevelFace face;
	LevelStrike strike;
};

class Buff_Jink:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Jink,37)

	Buff_Jink()
	{
		_idBroken=LevelSkillID_Invalid;
		_idTeleport=LevelTeleportID_Invalid;
		_flag=BuffFlag_Pausing|BuffFlag_GhostCollide|BuffFlag_Invisible;
		_bShowDealed=FALSE;
		_tFinish=0;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端

	virtual LevelBuffMask GetForbiddingBuffs();
	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnDestroy() override;
	virtual void _OnUpdate(AnimTick dt);

	//Factor Overriding
	BuffFlag GetFlags()	{		return _flag;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	void _SetFlag(BuffFlag flag)
	{
		if (_flag!=flag)
		{
			_flag=flag;
			_buffs->MarkFlagsDirty();
		}
	}
	BuffFlag _flag;

	CLevelObjPauser _pauser;

	LevelSkillID _idBroken;
	LevelTeleportID _idTeleport;

	LevelPos _pos;
	LevelFace _face;
	LevelStrike _strike;

	AnimTick _tFinish;

	BOOL _bShowDealed;


};


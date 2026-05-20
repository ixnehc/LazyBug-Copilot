#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"


struct BuffParam_ResideWT:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_ResideWT);

	BEGIN_GOBJ_PURE(BuffParam_ResideWT,1);

		GELEM_VAR_INIT(unsigned __int64,residing,0);
			GELEM_EDITVAR("效果",GVT_Bx8,GSem_ProtoPath,"驻留效果");
		GELEM_VAR_INIT(AnimTick,durEnter,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("进入时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"进入过程要花多久时间");
		GELEM_VAR_INIT(AnimTick,durExit,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("出来时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"出来过程要花多久时间");
	END_GOBJ();

	AnimTick durEnter;//进入过程要花多久时间
	AnimTick durExit;//出来过程要花多久时间

	unsigned __int64 residing;
};


struct BuffData_ResideWT
{
	BuffData_ResideWT()
	{
		memset(this,0,sizeof(*this));
	}
	LevelObjID idTarget;
	LevelTeleportID idTeleport;
	LevelSkillID idBroken;
	LevelPos pos;//记录角色的位置应该落在什么地方
	BYTE bCanceled;//这个Buff是否已经被cancel了
};

class Buff_ResideWT:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_ResideWT,9)

	Buff_ResideWT()
	{
		_flag=BuffFlag_Reside|BuffFlag_Pausing;
		_token=LevelObjSeatToken_Invalid;
		_durEnter=ANIMTICK_INFINITE;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}


	virtual LevelBuffMask GetReplaceBuffs();

	virtual void HandleEvent(LevelEvent &e0);

	virtual void _OnCreate(LevelBuffArg *param);
	virtual void _OnDestroy();

	virtual void _OnUpdate(AnimTick t);

	//Factor Overriding
	BuffFlag GetFlags()	{		return _flag;	}

	LevelObjID GetResidingTarget()	{		return _data.idTarget;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	void _DiscardSeatToken();

	void _CancelReside(LevelOpLink &link);

	void _SetFlag(BuffFlag flag)
	{
		if (_flag!=flag)
		{
			_flag=flag;
			_buffs->MarkFlagsDirty();
		}
	}

	BuffFlag _flag;

	LevelObjSeatToken _token;

	BuffData_ResideWT _data;
	DWORD _stateReside;

	LevelPos _posEntry;

	AnimTick _durEnter;

};


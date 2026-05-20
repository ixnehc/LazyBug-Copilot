#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"

struct BuffArg_Mount:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Mount);
	BuffArg_Mount()
	{
		idTarget=LevelObjID_Invalid;
		bNeedEnter=TRUE;
	}
	LevelObjID idTarget;
	BOOL bNeedEnter;//是否需要进入过程
};


struct BuffParam_Mount:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Mount);

	BEGIN_GOBJ_PURE(BuffParam_Mount,1);

		GELEM_VAR_INIT(unsigned __int64,mounting,0);
			GELEM_EDITVAR("效果",GVT_Bx8,GSem_ProtoPath,"驻留效果");
		GELEM_VAR_INIT(AnimTick,durEnter,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("进入时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"进入过程要花多久时间");
		GELEM_VAR_INIT(AnimTick,durExit,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("出来时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"出来过程要花多久时间");
	END_GOBJ();

	AnimTick durEnter;//进入过程要花多久时间
	AnimTick durExit;//出来过程要花多久时间

	unsigned __int64 mounting;
};


struct BuffData_Mount
{
	BuffData_Mount()
	{
		memset(this,0,sizeof(*this));
	}
	LevelObjID idTarget;
	LevelTeleportID idTeleport;
	LevelSkillID idBroken;
	BYTE bCanceled;//这个Buff是否已经被cancel了
	LevelPos posCancel;//记录角色的位置应该落在什么地方,只在bCanceled时有效
};

class Buff_Mount:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Mount,20)

	Buff_Mount()
	{
		_flag=BuffFlag_Mount|BuffFlag_Pausing;
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

	LevelObjID GetMountingTarget()	{		return _data.idTarget;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	void _CancelMount(LevelOpLink &link);

	void _SetFlag(BuffFlag flag)
	{
		if (_flag!=flag)
		{
			_flag=flag;
			_buffs->MarkFlagsDirty();
		}
	}

	BuffFlag _flag;

	BuffData_Mount _data;
	DWORD _stateMount;

	LevelPos _posEntry;

	AnimTick _durEnter;

};


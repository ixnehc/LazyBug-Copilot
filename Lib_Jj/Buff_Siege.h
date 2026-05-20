#pragma once

#include "LevelBuff.h"

#define MAX_PETRIFY_SLOW (0.5f)

struct BuffParam_Siege:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Siege);

	BEGIN_GOBJ_PURE(BuffParam_Siege,1);


	END_GOBJ();

};


struct BuffArg_Siege:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Siege)
	LevelObjID idTarget;
};


//控制角色姿势的Buff,基本上这个Buff只在Client起作用
class Buff_Siege:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Siege,19)

	Buff_Siege()
	{
		_idTarget=LevelObjID_Invalid;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//时间到结束时是否需要同步给客户端


	virtual void _OnCreate(LevelBuffArg *arg)
	{
		_idTarget=((BuffArg_Siege*)arg)->idTarget;
		_dur=ANIMTICK_INFINITE;
	}

	void Stop();

	virtual BuffFlag GetFlags()	{		return BuffFlag_SlideMove;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	LevelObjID _idTarget;

};


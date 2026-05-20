#pragma once

#include "LevelBuff.h"

struct BuffParam_General:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_General);

	BEGIN_GOBJ_PURE(BuffParam_General,1);

	END_GOBJ();
};


struct BuffArg_General:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_General)
};


class Buff_General:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_General,22)

	Buff_General()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端


	virtual void _OnCreate(LevelBuffArg *param)
	{
	}


protected:


};


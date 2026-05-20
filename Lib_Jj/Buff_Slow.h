=#pragma once

#include "LevelBuff.h"

struct BuffParam_Slow:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Slow);

	BEGIN_GOBJ_PURE(BuffParam_Slow,1);

	END_GOBJ();
};


struct BuffArg_Slow	`````
{
	DEFINE_CLASS(BuffArg_Slow)
	float str;//强度
};


//OverAll Speed
class Buff_Slow:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Slow,16)

	Buff_Slow()
	{
		_str=0.0f;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端


	virtual void _OnCreate(LevelBuffArg *param)
	{
		_str=((BuffArg_Slow*)param)->str;
	}

	virtual float GetIMS()	{		return -_str;	}

protected:

	virtual void _WriteData(CDataPacket *dp);

	float _str;

};


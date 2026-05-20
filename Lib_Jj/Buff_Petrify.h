#pragma once

#include "LevelBuff.h"

#define MAX_PETRIFY_SLOW (0.5f)

struct BuffParam_Petrify:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Petrify);

	BEGIN_GOBJ_PURE(BuffParam_Petrify,1);

		GELEM_VAR_INIT(float,dmp,0.4f);
			GELEM_EDITVAR("衰减速度",GVT_F,GSem(GSem_Float,"0,100,0.01"),"每秒衰减多少强度");

	END_GOBJ();

	float dmp;//衰减
};


struct BuffArg_Petrify:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Petrify)
	float str;//强度
};


//OverAll Speed
class Buff_Petrify:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Petrify,17)

	Buff_Petrify()
	{
		_str=0.0f;
		_bInc=FALSE;
		_bDmp=TRUE;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//时间到结束时是否需要同步给客户端


	virtual void _OnCreate(LevelBuffArg *param)
	{
		_str=((BuffArg_Petrify*)param)->str;
		_dur=ANIMTICK_INFINITE;

	}


	virtual float GetSlow()	
	{		
		float v=_str*MAX_PETRIFY_SLOW;
		if (v>MAX_PETRIFY_SLOW)
			v=MAX_PETRIFY_SLOW;
		return v;	
	}

	virtual void _OnUpdate(AnimTick dt);

	void IncStr(float str);

	void StopDamp()	{		_bDmp=FALSE;	}

	BOOL IsPetrified()
	{
		return _str>=1.0f;
	}


protected:

	virtual void _WriteData(CBitPacket *dp);

	BOOL _bInc;
	float _str;

	BOOL _bDmp;

};


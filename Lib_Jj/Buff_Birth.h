#pragma once

#include "LevelBuff.h"

struct BuffParam_Birth:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Birth);

	BEGIN_GOBJ_PURE(BuffParam_Birth,1);

	END_GOBJ();
};


struct BuffArg_Birth:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Birth);
	BuffArg_Birth()
	{
		eulerX=0.0f;
	}
	LevelOpDesc descOp;
	float eulerX;//转向

};

class Buff_Birth:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Birth,7)

	Buff_Birth()
	{
		_op=NULL;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Birth的Buff并存的情况
	virtual LevelOp_AddBuff *AccuireSyncOp()	{		return _op;	}


	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param);
	virtual void _OnDestroy();

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_NotAttackable|BuffFlag_Birth|BuffFlag_Pausing;	}

	virtual void _WriteData(CBitPacket *dp);



protected:

	LevelOp_AddBuff *_op;
	float _eulerX;

};


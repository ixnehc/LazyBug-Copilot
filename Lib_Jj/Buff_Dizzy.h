#pragma once

#include "LevelBuff.h"
#include "LevelStrike.h"

struct BuffArg_Dizzy:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Dizzy);

};


struct BuffParam_Dizzy:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Dizzy);

	BEGIN_GOBJ_PURE(BuffParam_Dizzy,1);


	END_GOBJ();
};


class Buff_Dizzy:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Dizzy,33)

	Buff_Dizzy()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUpIP()	{		return TRUE;	}


	virtual void _OnCreate(LevelBuffArg *param);
	virtual BOOL Merge(LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur)override;

	virtual LevelBuffMask GetForbiddingBuffs() override;

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_Pausing;	}


protected:

	virtual void _WriteData(CBitPacket *dp);


};


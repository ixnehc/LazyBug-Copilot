#pragma once

#include "LevelBuff.h"

struct BuffParam_AuraJoker:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_AuraJoker);

	BEGIN_GOBJ_PURE(BuffParam_AuraJoker,1);


	END_GOBJ();

};


struct BuffArg_AuraJoker
{
	DEFINE_CLASS(BuffArg_AuraJoker)
};


class Buff_AuraJoker:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_AuraJoker,58)

	Buff_AuraJoker()
	{
		_nRepeat=1;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//时间到结束时是否需要同步给客户端

	BOOL Merge(LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur) override;

	virtual void HandleEvent(LevelEvent &e);


	virtual void _OnCreate(LevelBuffArg *param)
	{
	}

	virtual void _OnUpdate(AnimTick dt);



protected:

	int _nRepeat;


};


#pragma once

#include "LevelBuff.h"

struct BuffParam_AuraLight:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_AuraLight);

	BEGIN_GOBJ_PURE(BuffParam_AuraLight,1);


	END_GOBJ();

};


struct BuffArg_AuraLight
{
	DEFINE_CLASS(BuffArg_AuraLight)
};


class Buff_AuraLight:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_AuraLight,55)

	Buff_AuraLight()
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


#pragma once

#include "LevelBuff.h"

struct BuffParam_AuraStrength:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_AuraStrength);

	BEGIN_GOBJ_PURE(BuffParam_AuraStrength,1);

		GELEM_VAR_INIT(float,rateDamage,0.2f);
			GELEM_EDITVAR("伤害增益",GVT_F,GSem(GSem_Float,"0.0,10.0,0.01"),"伤害增益");

	END_GOBJ();

	float rateDamage;

};


struct BuffArg_AuraStrength
{
	DEFINE_CLASS(BuffArg_AuraStrength)
};


class Buff_AuraStrength:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_AuraStrength,53)

	Buff_AuraStrength()
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


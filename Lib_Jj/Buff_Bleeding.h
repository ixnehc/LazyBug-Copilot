#pragma once

#include "LevelBuff.h"


struct BuffParam_Bleeding:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Bleeding);

	BEGIN_GOBJ_PURE(BuffParam_Bleeding,1);

		GELEM_VAR_INIT(AnimTick,gap,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("伤害间隔时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.1"),"伤害间隔时间");

		GELEM_VAR_INIT(float,nDmgPerSec,1.0f);
			GELEM_EDITVAR("每秒减多少HP",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"每秒减多少HP");

	END_GOBJ();

	float nDmgPerSec;
	AnimTick gap;//每次Deal的间隔时间

};

struct BuffArg_Bleeding:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Bleeding);
};



class Buff_Bleeding:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Bleeding,34)

	Buff_Bleeding()
	{
		_nDeal=0;
		_nDamaged=0;
	}

	virtual BOOL NeedSync()  override	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()  override	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Bleeding的Buff并存的情况

	virtual LevelBuffMask GetForbiddingBuffs()  override;

	virtual void _OnUpdate(AnimTick dt)  override;

protected:
	int _nDeal;
	int _nDamaged;

};


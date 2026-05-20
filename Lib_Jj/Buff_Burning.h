#pragma once

#include "LevelBuff.h"


struct BuffParam_Burning:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Burning);

	BEGIN_GOBJ_PURE(BuffParam_Burning,1);

		GELEM_VAR_INIT(AnimTick,gap,ANIMTICK_FROM_SECOND(0.1f));
			GELEM_EDITVAR("伤害间隔时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.1"),"伤害间隔时间");

	END_GOBJ();

	AnimTick gap;//每次Deal的间隔时间

};

struct BuffArg_Burning:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Burning);
};



class Buff_Burning:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Burning,26)

	Buff_Burning()
	{
		_nDeal=0;
	}

	virtual BOOL NeedSync()  override	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()  override	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Burning的Buff并存的情况

	virtual BOOL Merge(LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur) override;

	virtual LevelBuffMask GetReplaceBuffs()  override;

	virtual void _OnUpdate(AnimTick dt)  override;

protected:
	float _CalcDPS(LevelRecordBuff *rec);
	int _nDeal;

};


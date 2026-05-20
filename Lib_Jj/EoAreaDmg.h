#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_AreaDmg 19

struct EoParamAreaDmg:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamAreaDmg);

	BEGIN_GOBJ_PURE(EoParamAreaDmg,1);

		GELEM_VAR_INIT(float,radius,2.0f);
			GELEM_EDITVAR("作用范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"作用范围");

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"持续时间,如果为0,则只在一开始产生伤害");

		GELEM_VAR_INIT(float,dmgInitial,5.0f);
			GELEM_EDITVAR("初始伤害值",GVT_F,GSem(GSem_Float,"0,100,0.01"),"初始的伤害值");

		GELEM_VAR_INIT(float,dmgPerSec,1.0f);
			GELEM_EDITVAR("每秒伤害值",GVT_F,GSem(GSem_Float,"0,100,0.01"),"每秒伤害值");

	END_GOBJ();

	float radius;
	AnimTick dur;
	float dmgInitial;//一开始的伤害
	float dmgPerSec;//持续伤害,每秒钟的伤害
};



class EoAreaDmg:public CLoEffectObj
{
public:
	EoAreaDmg()
	{
		_bInitialDmg=FALSE;
		_nCommits=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoAreaDmg,CLASSUID_AreaDmg);

	virtual const char *GetShowName()	{		return "有毒区域";	}

protected:
	void _OnUpdate();

	BOOL _bInitialDmg;//初始的Damage有没有结算

	DWORD _nCommits;//已经结算了几次持续性伤害
};

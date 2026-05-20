#pragma once

#include "LevelReactor.h"

struct ReactorParam_DamageImmune:public LevelReactorParam
{
	DEFINE_REACTORPARAM_CLASS(ReactorParam_DamageImmune);

	enum Condition
	{
		Condition_Always=0,
		Condition_NotCastingSkillAndDmgFromThreat,//没有施放技能并且伤害来自BestThreat

		ForceDword,
	};

	BEGIN_GOBJ_PURE(ReactorParam_DamageImmune,1);

		GELEM_VAR_INIT(DWORD,condition,Condition_Always);
			GELEM_EDITVAR("条件",GVT_U,GSem(GSem_Interger,"任何时候,未释放技能时"),"起效条件");
		GELEM_VAR_INIT(float,possibility,1.0f);
			GELEM_EDITVAR("起效机率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.01"),"起效机率");
		GELEM_VAR_INIT(float,scaleReduced,0.1f);//减10%
			GELEM_EDITVAR("减少多少比率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.01"),"减少多少伤害比率");

	END_GOBJ();

	Condition condition;
	float possibility;

	float scaleReduced;
};


class Reactor_DamageImmune:public CLevelReactor
{
public:
	DEFINE_CLASS(Reactor_DamageImmune)

	Reactor_DamageImmune()
	{
	}


	virtual void _OnCreate();

	virtual void HandleEvent(LevelEvent &e);

protected:

};


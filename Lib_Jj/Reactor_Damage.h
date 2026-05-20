#pragma once

#include "LevelReactor.h"

struct ReactorParam_Damage:public LevelReactorParam
{
	DEFINE_REACTORPARAM_CLASS(ReactorParam_Damage);

	BEGIN_GOBJ_PURE(ReactorParam_Damage,1);

		GELEM_VAR_INIT(int,nMinDmg,0)
			GELEM_EDITVAR("最小伤害",GVT_S,GSem_Interger,"受到的最小伤害");
		GELEM_VAR_INIT(int,nMaxDmg,50000)
			GELEM_EDITVAR("最大伤害",GVT_S,GSem_Interger,"受到的最大伤害");

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,dealToMe,Deal_Null, "对自己的结算", "对自己的结算" );
			GELEMS_LEVELDEAL_CANDIDATES();

	END_GOBJ();

	int nMinDmg;
	int nMaxDmg;
	CLevelDeal *dealToMe;
};


class Reactor_Damage:public CLevelReactor
{
public:
	DEFINE_CLASS(Reactor_Damage)

	Reactor_Damage()
	{
	}


	virtual void _OnCreate();

	virtual void HandleEvent(LevelEvent &e);

protected:

};


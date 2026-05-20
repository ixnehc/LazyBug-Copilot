#pragma once

#include "LevelReactor.h"

struct ReactorParam_PostDamage:public LevelReactorParam
{
	DEFINE_REACTORPARAM_CLASS(ReactorParam_PostDamage);

	BEGIN_GOBJ_PURE(ReactorParam_PostDamage,1);

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,dealToMe,Deal_Null, "对自己的结算", "对自己的结算" );
			GELEMS_LEVELDEAL_CANDIDATES();

	END_GOBJ();

	CLevelDeal *dealToMe;
};


class Reactor_PostDamage:public CLevelReactor
{
public:
	DEFINE_CLASS(Reactor_PostDamage)

	Reactor_PostDamage()
	{
	}


	virtual void _OnCreate();

	virtual void HandleEvent(LevelEvent &e);

protected:

};


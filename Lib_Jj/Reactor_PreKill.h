#pragma once

#include "LevelReactor.h"

struct ReactorParam_PreKill:public LevelReactorParam
{
	DEFINE_REACTORPARAM_CLASS(ReactorParam_PreKill);

	BEGIN_GOBJ_PURE(ReactorParam_PreKill,1);

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,dealToMe,Deal_Null, "对自己的结算", "对自己的结算" );
			GELEMS_LEVELDEAL_CANDIDATES();
		GELEM_VAR_INIT(BOOL,bAbandon,FALSE);
			GELEM_EDITVAR("是否阻止Kill",GVT_S,GSem_Boolean,"是否阻止Kill");
	END_GOBJ();

	CLevelDeal *dealToMe;
	BOOL bAbandon;
};


class Reactor_PreKill:public CLevelReactor
{
public:
	DEFINE_CLASS(Reactor_PreKill)

	Reactor_PreKill()
	{
	}


	virtual void _OnCreate();

	virtual void HandleEvent(LevelEvent &e);

protected:

};


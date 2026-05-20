#pragma once

#include "LevelReactor.h"

struct ReactorParam_Stun:public LevelReactorParam
{
	DEFINE_REACTORPARAM_CLASS(ReactorParam_Stun);

	BEGIN_GOBJ_PURE(ReactorParam_Stun,1);

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,dealToMe,Deal_Null, "对自己的结算", "对自己的结算" );
			GELEMS_LEVELDEAL_CANDIDATES();
		GELEM_VAR_INIT(BOOL ,bDefered,FALSE);
			GELEM_EDITVAR("延后处理",GVT_S,GSem_Boolean,"延后一帧处理");
	END_GOBJ();

	CLevelDeal *dealToMe;
	BOOL bDefered;
};


class Reactor_Stun:public CLevelReactor
{
public:
	DEFINE_CLASS(Reactor_Stun)

	Reactor_Stun()
	{
		_bTriggered=FALSE;
	}


	virtual void _OnCreate();
	virtual void Update(AnimTick t);

	virtual void HandleEvent(LevelEvent &e);

protected:
	BOOL _bTriggered;

};


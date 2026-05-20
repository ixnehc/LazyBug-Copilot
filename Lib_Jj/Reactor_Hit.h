#pragma once

#include "LevelReactor.h"

struct ReactorParam_Hit:public LevelReactorParam
{
	DEFINE_REACTORPARAM_CLASS(ReactorParam_Hit);

	BEGIN_GOBJ_PURE(ReactorParam_Hit,1);

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,dealToMe,Deal_Null, "对自己的结算(obsolete)", "对自己的结算" );
			GELEMS_LEVELDEAL_CANDIDATES();
// 		GELEM_OBJVECTOR(DealEntry,dealsToMe);
// 			GELEM_EDITOBJ("对自己的结算列表","多个结算");
		GELEM_VAR_INIT(BOOL ,bDefered,FALSE);
			GELEM_EDITVAR("延后处理",GVT_S,GSem_Boolean,"延后一帧处理");
	END_GOBJ();

	CLevelDeal *dealToMe;
// 	std::vector<DealEntry> dealsToMe;
	BOOL bDefered;
};


class Reactor_Hit:public CLevelReactor
{
public:
	DEFINE_CLASS(Reactor_Hit)

	Reactor_Hit()
	{
		_bTriggered=FALSE;
	}


	virtual void _OnCreate();
	virtual void Update(AnimTick t);

	virtual void HandleEvent(LevelEvent &e);

protected:
	BOOL _bTriggered;

};


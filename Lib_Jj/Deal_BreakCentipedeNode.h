#pragma once

#include "LevelDeal.h"

class Deal_BreakCentipedeNode:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_BreakCentipedeNode);


	BEGIN_GOBJ_PURE(Deal_BreakCentipedeNode,1);

		GELEM_BEHAVIORMEM_OBJID(varCentipedeAgent,"蜈蚣对象","蜈蚣对象");

	END_GOBJ();

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

	StringID varCentipedeAgent;

};

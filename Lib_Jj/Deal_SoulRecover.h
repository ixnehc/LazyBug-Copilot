#pragma once

#include "LevelDeal.h"

class Deal_SoulRecover:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_SoulRecover);


	BEGIN_GOBJ_PURE(Deal_SoulRecover,1);


	END_GOBJ();

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

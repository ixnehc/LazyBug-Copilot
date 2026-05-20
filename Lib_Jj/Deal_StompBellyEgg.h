#pragma once

#include "LevelDeal.h"

class Deal_StompBellyEgg:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_StompBellyEgg);


	BEGIN_GOBJ_PURE(Deal_StompBellyEgg,1);

	END_GOBJ();

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result);


};

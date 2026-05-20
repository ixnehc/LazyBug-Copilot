#pragma once

#include "LevelDeal.h"

class Deal_CleanOwnerBuff:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_CleanOwnerBuff);


	BEGIN_GOBJ_PURE(Deal_CleanOwnerBuff,1);

	END_GOBJ();


	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg &arg,DealResult *result)override;

};

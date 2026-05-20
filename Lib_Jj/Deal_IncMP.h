#pragma once

#include "LevelDeal.h"

#include "LevelAttrs_DamageAttr.h"
#include "LevelAttrs_Weak.h"


class Deal_IncMP:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_IncMP);


	BEGIN_GOBJ_PURE(Deal_IncMP,1);


	END_GOBJ();


	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

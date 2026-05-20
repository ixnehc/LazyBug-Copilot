#pragma once

#include "LevelDeal.h"

class Deal_MakePainDrop:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_MakePainDrop);


	BEGIN_GOBJ_PURE(Deal_MakePainDrop,1);

	END_GOBJ();

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

#pragma once

#include "LevelDeal.h"

class Deal_KnockBack:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_KnockBack);


	BEGIN_GOBJ_PURE(Deal_KnockBack,1);

		GELEM_VAR_INIT(float,_str,1.0f);
			GELEM_EDITVAR("击退力量",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"击退力量");

	END_GOBJ();

	float _str;

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

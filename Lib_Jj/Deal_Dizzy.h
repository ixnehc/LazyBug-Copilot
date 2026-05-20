#pragma once

#include "LevelDeal.h"

class Deal_Dizzy:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_Dizzy);


	BEGIN_GOBJ_PURE(Deal_Dizzy,1);

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"Buff持续时间,0表示永远");

	END_GOBJ();

	AnimTick dur;

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

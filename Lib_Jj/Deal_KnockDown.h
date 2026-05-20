#pragma once

#include "LevelDeal.h"

class Deal_KnockDown:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_KnockDown);


	BEGIN_GOBJ_PURE(Deal_KnockDown,1);

		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"Buff持续时间,0表示永远");

	END_GOBJ();

	RecordID idBuff;
	AnimTick dur;

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

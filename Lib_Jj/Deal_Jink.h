#pragma once

#include "LevelDeal.h"

class Deal_Jink:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_Jink);


	BEGIN_GOBJ_PURE(Deal_Jink,1);

		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");

	END_GOBJ();

	RecordID idBuff;

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

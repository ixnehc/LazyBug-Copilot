#pragma once

#include "LevelDeal.h"

class Deal_Petrify:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_Petrify);


	BEGIN_GOBJ_PURE(Deal_Petrify,1);

		GELEM_VAR_INIT(RecordID,idPetrify,RecordID_Invalid);
			GELEM_EDITVAR("石化Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"石化的Buff");

		GELEM_VAR_INIT(RecordID,idPetrified,RecordID_Invalid);
			GELEM_EDITVAR("石化凝固Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"石化凝固的Buff");

		GELEM_VAR_INIT(float,str,0.1f);
			GELEM_EDITVAR("每次增加石化程度多少",GVT_F,GSem(GSem_Float,"0,100,0.01"),"每次增加石化程度多少");

	END_GOBJ();

	RecordID idPetrify;
	float str;

	RecordID idPetrified;
	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

#pragma once

#include "LevelDeal.h"

class Deal_Teleport:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_Teleport);


	BEGIN_GOBJ_PURE(Deal_Teleport,1);

		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");

		GELEM_VAR_INIT(float,radiusMin,1.0f);
			GELEM_EDITVAR("最小半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"最小半径");
		GELEM_VAR_INIT(float,radiusMax,3.0f);
			GELEM_EDITVAR("最大半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"最大半径");

	END_GOBJ();

	float radiusMin;
	float radiusMax;

	RecordID idBuff;

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

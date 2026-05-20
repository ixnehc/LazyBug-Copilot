#pragma once

#include "LevelDeal.h"

class Deal_RemoveBuff:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_RemoveBuff);


	BEGIN_GOBJ_PURE(Deal_RemoveBuff,1);

		GELEM_VAR_INIT(BOOL,bRemoveFromOwner,FALSE);
			GELEM_EDITVAR("给自己去除",GVT_S,GSem_Boolean,"是否在自己身上去除Buff");
		GELEM_VARVECTOR_INIT(RecordID,idsRequiredBuff,RecordID_Invalid);
			GELEM_EDITVAR("Required Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");

	END_GOBJ();

	BOOL bRemoveFromOwner;
	std::vector<RecordID> idsRequiredBuff;
	RecordID idBuff;

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

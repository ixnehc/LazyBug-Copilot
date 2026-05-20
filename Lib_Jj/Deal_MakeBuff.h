#pragma once

#include "LevelDeal.h"

class Deal_MakeBuff:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_MakeBuff);


	BEGIN_GOBJ_PURE(Deal_MakeBuff,1);

		GELEM_VAR_INIT(BOOL,bAddToOwner,FALSE);GELEM_UID(1);
			GELEM_EDITVAR("给自己加",GVT_S,GSem_Boolean,"是否给自己加Buff");
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);GELEM_UID(2);
			GELEM_EDITVAR("Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");
		GELEM_VARVECTOR_INIT(RecordID,idsBlockerBuff,RecordID_Invalid);GELEM_UID(3);
			GELEM_EDITVAR("Blocker Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");
		GELEM_VARVECTOR_INIT(RecordID,idsRequiredBuff,RecordID_Invalid);GELEM_UID(6);
			GELEM_EDITVAR("Required Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(0.0f));GELEM_UID(4);
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"Buff持续时间,0表示永远");
		GELEM_VAR_INIT(int,opOnSameBuff,0);GELEM_UID(5);
			GELEM_EDITVAR("对已存在的同样Buff如何操作",GVT_S,GSem(GSem_Interger,"放弃添加:0,时间叠加:1,照常添加:2,替换:3"),"对已存在的同样Buff如何操作");
	END_GOBJ();

	BOOL bAddToOwner;
	RecordID idBuff;
	std::vector<RecordID> idsBlockerBuff;
	std::vector<RecordID> idsRequiredBuff;
	AnimTick dur;
	int opOnSameBuff;

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

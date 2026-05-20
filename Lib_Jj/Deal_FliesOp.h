#pragma once

#include "LevelDeal.h"

class Deal_FliesOp:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_FliesOp);

	enum Op
	{
		OverrideEnchanted_On,
		OverrideEnchanted_Off,
		ClearOverrideEnchanted,
		SetForm_Default,
		SetForm_Scattered,

		ForceDword=0xffffffff,
	};


	BEGIN_GOBJ_PURE(Deal_FliesOp,1);

		GELEM_VAR_INIT(Op,op,OverrideEnchanted_On);
			GELEM_EDITVAR("操作",GVT_S,GSem(GSem_Interger,"重载为Enchanted,重载为Unenchanted,取消Enchanted的重载,设为缺省形态,设为扩散形态"),"操作类型");
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");

	END_GOBJ();

	Op op;
	RecordID idBuff;

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

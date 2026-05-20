#pragma once

#include "LevelDeal.h"

class Deal_StartBhvRelay:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_StartBhvRelay);


	BEGIN_GOBJ_PURE(Deal_StartBhvRelay,1);

		GELEM_VAR_INIT(int,tpBhv,0); GELEM_UID(1);
			GELEM_EDITVAR("Behavior类型",GVT_S,GSem(GSem_Interger,
				"目标对象的Buff:0"		","
				),"Behavior类型");
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);GELEM_UID(2);
			GELEM_EDITVAR("Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");

		GELEM_VAR_INIT( StringID,nmRelay,StringID_Invalid);	
			GELEM_EDITVAR( "中继名称", GVT_U, GSem(GSem_StringID,"行为图中继名称"), "中继的名称" );

	END_GOBJ();

	int tpBhv;
	RecordID idBuff;
	StringID nmRelay;


	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

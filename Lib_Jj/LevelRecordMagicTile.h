#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "gds/GObj.h"
#include "gds/GObjEx.h"


#include "records/records.h"

#include "MagicBoardDefines.h"



struct LevelRecordMagicTile:public CRecord
{
	DEFINE_CLASS(LevelRecordMagicTile);

	std::string Name;
	RecordID idUnit;
	RecordID idAgent;
	RecordID idEO;

	BOOL bCommitUnseal;

	RecordID idBirthBuff;

	unsigned __int64 bottom;

	MBResCost costCommit;

    BEGIN_GOBJ_PURE(LevelRecordMagicTile,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"MagicTile的名称");

		GELEM_VAR_INIT(BOOL,bCommitUnseal,0);
			GELEM_EDITVAR("翻开时立即创建",GVT_S,GSem_Boolean,"翻开时立即创建");
		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("Unit",GVT_U,GSem(GSem_RecordID,"units"),"召唤的单位");
		GELEM_VAR_INIT(RecordID,idAgent,RecordID_Invalid);
			GELEM_EDITVAR("Agent",GVT_U,GSem(GSem_RecordID,"agents"),"召唤的Agent");
		GELEM_VAR_INIT(RecordID,idEO,RecordID_Invalid);
			GELEM_EDITVAR("EO",GVT_U,GSem(GSem_RecordID,"eos"),"召唤的EO");

		GELEM_VAR_INIT(RecordID,idBirthBuff,RecordID_Invalid);
			GELEM_EDITVAR("出生Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"出生的Buff");

		GELEM_VAR_INIT(unsigned __int64,bottom,0);
			GELEM_EDITVAR("底",GVT_Bx8,GSem_ProtoPath,"格子底部的Proto路径");

		GELEM_OBJ(MBResCost,costCommit) GELEM_VERSION(2)
			GELEM_EDITOBJ("Commit消耗","Commit消耗的资源数量");
    END_GOBJ();    
};

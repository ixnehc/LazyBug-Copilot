#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "records/records.h"

enum NPCMode
{
	NPCMode_None=0,
	NPCMode_Standard=1,//NPC为每个玩家维护一个实例

	NPCMode_ForceDword=0xffffffff,
};


struct LevelNPCParam;
struct LevelRecordNPC:public CRecord
{
	DEFINE_CLASS(LevelRecordNPC);

	std::string Name;

	RecordID idUnit;
	int grdBase;

	BOOL bFreePos;//表示这个NPC的位置是否可以根据NPC的状态变化而发生变化,比如会变成Retinue,或者会NPC在NPC的某个状态下被设定到另一个位置上,

	NPCMode mode;

	BEGIN_GOBJ_PURE(LevelRecordNPC,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"单位的名称");

		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位",GVT_U,GSem(GSem_RecordID,"units"),"单位ID");

		GELEM_VAR_INIT(int,grdBase,1);
			GELEM_EDITVAR("等级",GVT_S,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"单位的等级");

		GELEM_VAR_INIT(BOOL,bFreePos,FALSE);
			GELEM_EDITVAR("位置可自由变动",GVT_S,GSem_Boolean,"NPC的位置可以根据NPC的状态变化而自由变动");

		GELEM_VAR_INIT(NPCMode,mode,NPCMode_Standard);
			GELEM_EDITVAR("NPC模式",GVT_S,GSem(GSem_Interger,"n/a,标准模式"),"NPC的模式");


	END_GOBJ();


};

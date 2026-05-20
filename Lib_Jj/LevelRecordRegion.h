#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "records/records.h"

//Agent的分布信息
struct AgentDistributeInfo
{
	RecordID idAgent;//那个单位
	float rateAppear;//出现的几率
	LevelGrade grdRef;//参考等级

	BEGIN_GOBJ_PURE(AgentDistributeInfo,1);

		GELEM_VAR_INIT(RecordID,idAgent,RecordID_Invalid);
			GELEM_EDITVAR("Agent的ID",GVT_U,GSem(GSem_RecordID,"agents"),"哪个Agent");

		GELEM_VAR_INIT(float,rateAppear,0.5f);
			GELEM_EDITVAR("出现几率",GVT_F,GSem(GSem_Float,"0,1,0.01"),"出现几率");

		GELEM_VAR_INIT(int,grdRef,1);
			GELEM_EDITVAR("参考等级",GVT_S,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"参考等级");


	END_GOBJ();

};


struct LevelRecordRegion:public CRecord
{
	DEFINE_CLASS(LevelRecordRegion);

	std::string Name;
	DWORD col;

	std::vector<AgentDistributeInfo> adis;//spawner reference info

	BEGIN_GOBJ_PURE(LevelRecordRegion,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"区域的名称");
		GELEM_VAR_INIT(DWORD,col,0xffffffff);
			GELEM_EDITVAR("颜色",GVT_Bx4,GSem_ColorAlphaU,"代表颜色");

		GELEM_OBJVECTOR(AgentDistributeInfo,adis)
			GELEM_EDITOBJ("Agent分布信息","控制Agent分布的信息");

	END_GOBJ();


};
#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "records/records.h"

#include "LevelDefines.h"


struct LevelRecordPosture:public CRecord
{
	DEFINE_CLASS(LevelRecordPosture);

	std::string Name;
	LevelPostureType tp;

	unsigned __int64 switcher;//切换动作

	BEGIN_GOBJ_PURE(LevelRecordPosture,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"名称");
		GELEM_VAR_INIT(LevelPostureType,tp,LevelPosture_None);
			GELEM_EDITVAR("姿势类型",GVT_S,GSem(GSem_Interger,"n/a,徒手,单手盾,单手短兵器+盾,单手短兵器,单手长兵器+盾,单手长兵器,双手剑,弓,弩,双手矛,双手斧,双手杖,双手拖刀,双持剑,单手魔法物件+盾,单手魔法物件"),"姿势的类型");//XXXXX:more LevelPostureType
		GELEM_VAR_INIT(unsigned __int64,switcher,0);
			GELEM_EDITVAR("切换动作Proto",GVT_Bx8,GSem_ProtoPath,"进入该姿势需要播放那个切换动作");
	END_GOBJ();


};
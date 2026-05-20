#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "records/records.h"

#include "LevelItemBuffType.h"



struct LevelRecordItemSeed:public CRecord
{
	DEFINE_CLASS(LevelRecordItemSeed);

	std::string Name;

	EItemBuffType tp;//A EItemBuffType value
	int base;//值,
	int vary;//浮动值
	int grdRequire;//道具的等级要求

	BEGIN_GOBJ_PURE(LevelRecordItemSeed,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"名称");

		GELEM_VAR_INIT(EItemBuffType,tp,ItemBuff_None);
			GELEM_EDITVAR("类型",GVT_S,GSem(GSem_Interger,Enums_GetGSemStr(EItemBuffType)),"类型");

		GELEM_VAR_INIT(int,base,0);
			GELEM_EDITVAR("基础值",GVT_S,GSem_Interger,"基础值");
		GELEM_VAR_INIT(int,vary,0);
			GELEM_EDITVAR("浮动值",GVT_S,GSem_Interger,"浮动值");

		GELEM_VAR_INIT(int,grdRequire,1);
			GELEM_EDITVAR("等级要求",GVT_S,GSem_Interger,"等级要求");

	END_GOBJ();


};
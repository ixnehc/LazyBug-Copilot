#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "records/records.h"

#include "anim/animdefines.h"

struct LevelRecordFlock:public CRecord
{
	DEFINE_CLASS(LevelRecordFlock);

	std::string Name;


	BEGIN_GOBJ_PURE(LevelRecordFlock,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"单位的名称");
	END_GOBJ();


};

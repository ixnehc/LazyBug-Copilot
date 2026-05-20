#pragma once

#include "gds/GObj.h"

struct LevelUpgradableValue
{
	float base;
	float perGrade;

	BEGIN_GOBJ_PURE(LevelUpgradableValue,1);
		GELEM_VAR_INIT(float,base,0.0f);
			GELEM_EDITVAR("基础值",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"基础值");
		GELEM_VAR_INIT(float,perGrade,0.0f);
			GELEM_EDITVAR("每级增加值",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"每级增加值");
	END_GOBJ();

};
#pragma once

#include "LevelDefines.h"

#include "gds/GObj.h"


//消耗
struct LevelCost
{

	DWORD sp;
	float spMax;
	DWORD gold;
	DWORD gem;
	DWORD soul;
	DWORD crystal_;
	DWORD mp;

	BEGIN_GOBJ_PURE(LevelCost,1);

		GELEM_VAR_INIT(DWORD,sp,0);
			GELEM_EDITVAR("精力消耗",GVT_U,GSem_Interger,"精力消耗");

		GELEM_VAR_INIT(float,spMax,0.0f);
			GELEM_EDITVAR("精力上限消耗",GVT_U,GSem(GSem_Float,"0.00f,100.0f,0.01f"),"精力上限消耗");

		GELEM_VAR_INIT(DWORD,gold,0);
			GELEM_EDITVAR("金子消耗",GVT_U,GSem_Interger,"金子消耗");

		GELEM_VAR_INIT(DWORD,gem,0);
			GELEM_EDITVAR("宝石消耗",GVT_U,GSem_Interger,"宝石消耗");

		GELEM_VAR_INIT(DWORD,soul,0);
			GELEM_EDITVAR("魔血消耗",GVT_U,GSem_Interger,"魔血消耗");

		GELEM_VAR_INIT(DWORD,crystal_,0);
			GELEM_EDITVAR("魔晶消耗",GVT_U,GSem_Interger,"魔晶消耗");

		GELEM_VAR_INIT(DWORD,mp,0);
			GELEM_EDITVAR("魔力消耗",GVT_U,GSem_Interger,"魔力消耗");

	END_GOBJ();


};


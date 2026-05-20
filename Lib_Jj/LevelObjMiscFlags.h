#pragma once

#include "gds/GObj.h"



#define LoMiscFlag_AllowBloodTeeth 1

struct LoMiscFlags
{
	LoMiscFlags()
	{
		flags=0;
	}


	BOOL GetAllowBloodTeeth()
	{
		return flags&LoMiscFlag_AllowBloodTeeth;
	}
	DWORD flags;
};

struct LoUnitMiscFlags:public LoMiscFlags
{
	BEGIN_GOBJ_PURE(LoUnitMiscFlags,1);

		GELEM_VAR_INIT(DWORD,flags,LoMiscFlag_AllowBloodTeeth);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"允许血牙剑抽血:1"),"标志");

	END_GOBJ();

};

struct LoAgentMiscFlags:public LoMiscFlags
{
	BEGIN_GOBJ_PURE(LoAgentMiscFlags,1);

		GELEM_VAR_INIT(DWORD,flags,0);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"允许血牙剑抽血:1"),"标志");

	END_GOBJ();
};

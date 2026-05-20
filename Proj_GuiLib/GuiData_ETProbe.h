#pragma once

#include "GuiLib.h"

#include "WorldSystem/IETProbe.h"

#include "editor/editor.h"

#include "class/class.h"

struct GuiLib_Api GuiData_ETProbe:public GeData
{
	virtual const char *GetName()	{		return "etprobe";	}

	GuiData_ETProbe()
	{
		Zero();
	}
	~GuiData_ETProbe()
	{
		Clear();
	}
	void Zero()
	{
		pES = NULL;
		bOnCreate = FALSE;
	}
	void Clear()
	{
		Zero();
	}

	IETProbeEditor * GetEditor();
	IEntitySystem *pES;
	IUtilRS * pUtils;
	//选中状态
	std::vector<HMapObj> hObjSels;
	BOOL bOnCreate;
	DWORD ver;

	std::vector<HMapObj> nodes;
};






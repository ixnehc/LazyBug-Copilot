#pragma once

#include "GuiLib.h"

#include "WorldSystem/IShore.h"

#include "editor/editor.h"

#include "WorldSystem/IBrushLib.h"

struct GuiLib_Api GuiData_Shore:public GeData
{
	virtual const char *GetName()	{		return "shore";	}

	GuiData_Shore()
	{
		Zero();
	}

	~GuiData_Shore()
	{
		Clear();
	}

	void Zero()
	{
		pES = NULL;
		brID = INVALID_BRUID;
		bOnCreate = FALSE;
		ver = 0;
		bShowWireframe = TRUE;
		hObjSel = INVALID_HMAPOBJ;
	}
	
	void Clear()
	{
		Zero();
	}

	IShoreEditor * GetEditor();
	IEntitySystem *pES;

	//选中状态
	HMapObj hObjSel;
	std::vector<DWORD>   idxSelCPs;

	ShoreInfo info;
	BRUID brID;
	BOOL bOnCreate;
	BOOL bShowWireframe;
	DWORD ver;
};






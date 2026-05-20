#pragma once

#include "GuiLib.h"

#include "WorldSystem/IRoad.h"

#include "editor/editor.h"

#include "class/class.h"

struct GuiLib_Api GuiData_Road:public GeData
{
	enum State
	{
		Creating,
		Idle,
	};

	virtual const char *GetName()	{		return "road";	}

	GuiData_Road()
	{
		Zero();
	}
	~GuiData_Road()
	{
	}
	void Zero()
	{
		stateWork = Idle;
		pES = NULL;
		hObjSel = INVALID_HMAPOBJ;

        bPaintingOpacity = FALSE;
        radiusPaintOpacityInner = 0.2f;
        radiusPaintOpacityOutter = 0.5f;
        deltaPaintOpacity = 0.05f;
	}

	void Clear()
	{
		Zero();
	}

	IRoadEditor * GetEditor();
	IEntitySystem *pES;

	//选中状态
	State stateWork;
	HMapObj hObjSel;
	RoadProp roadProp;
	std::vector<DWORD> selKeys; //选中对象上的节点信息

    BOOL bPaintingOpacity;
    float radiusPaintOpacityInner;
    float radiusPaintOpacityOutter;
    float deltaPaintOpacity;

    DWORD ver;
};






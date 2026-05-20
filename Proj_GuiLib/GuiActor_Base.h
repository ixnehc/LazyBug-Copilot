#pragma once

#include "GuiLib.h"


#include "GuiData.h"
#include "GuiEditor.h"

#include "GuiAgent_general.h"




class GuiLib_Api CGuiActor_Base:public CGuiActor
{
public:
	CGuiActor_Base()
	{
	}

	virtual const char *GetName()	{		return "base";	}

	virtual void UpdateUI();
	virtual void OnDetachView(CGeView *view,DWORD iLevel);

	void DoAttach();

protected:
	virtual const char *_GetModMgrName()	{		return "world";	}



};

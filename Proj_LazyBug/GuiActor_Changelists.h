#pragma once

#include "GuiLib.h"
#include "GuiEditor.h"

#include "GuiAgent_Changelists.h"

class CGuiActor_Changelists: public CGuiActor
{
public:
	CGuiActor_Changelists();
	virtual const char *GetName()	{		return "Changelists";	}

	virtual CWnd *GetWnd();

	virtual void Reset();

	virtual void UpdateUI();

	virtual void DoCommand( DWORD idCmd );

	virtual void UpdateCommandUI( DWORD idCmd, void *param );

protected:
	virtual const char *_GetModMgrName()	{		return "Changelists";	}

	CGuiAgent_2DTransform _agTransform;
};


#pragma once

#include "GuiLib.h"
#include "GuiEditor.h"

#include "GuiAgent_ValueSet.h"

class GuiLib_Api CGuiActor_ValueSet : public CGuiActor
{
public:
	CGuiActor_ValueSet();
	virtual const char *GetName()	{		return "ValueSet";	}

	virtual CWnd *GetWnd();

	virtual void Reset();

	virtual void UpdateUI();

	virtual void DoCommand( DWORD idCmd );

	virtual void UpdateCommandUI( DWORD idCmd, void *param );

	BOOL Fit(const char *path);
protected:
	virtual const char *_GetModMgrName()	{		return "ValueSet";	}

	CGuiAgent_ValueSet2DTransform _agTransform;
};


#pragma once
#include "GuiLib.h"

#include "GuiEditor.h"

#include "WorldSystem/IDebugger.h"

#include "DebugVarGrid.h"

class GuiLib_Api CDbgPanel_Watch:public CGuiPanel
{

public:
	CDbgPanel_Watch(CWnd* pParent = NULL);
	virtual const char *GetName()	{		return "dbg_watch";	}

	BOOL Create(CWnd *pParent);

	virtual void UpdateUI();

	virtual BOOL OnInitDialog();

	void Reset();

protected:


	void _RecalcLayout();

	CDebugVarGrid _grid;

	BreakID _breakid;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
};



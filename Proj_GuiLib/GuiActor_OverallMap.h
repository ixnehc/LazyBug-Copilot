#pragma once
#include  "GuiEditor.h"
#include "ToolContainer.h"

class ToolContainer;
class CGuiActor_OverallMap :public CGuiPanel
{
public:
	CGuiActor_OverallMap(CWnd * pParent = NULL);
	~CGuiActor_OverallMap(void);
	virtual const char *GetName(){return "overallmap";}
	virtual const char *_GetModMgrName()	{		return "world";	}
	BOOL Create(CWnd * pParent);	
	BOOL OnInitDialog();
	virtual void OnLeaveActivity();
	virtual void OnEnterActivity();
protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnToolChange();

	void _UpdateTools();
	void _UpdateMode();

	ToolContainer  _tools;
};

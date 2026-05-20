#pragma once
#include "GuiLib.h"

#include "GuiEditor.h"

#include "WorldSystem/IDebugger.h"

class GuiLib_Api CDbgPanel_CallStack:public CGuiPanel
{

public:
	CDbgPanel_CallStack(CWnd* pParent = NULL);
	virtual const char *GetName()	{		return "dbg_callstack";	}

	BOOL Create(CWnd *pParent);

	virtual void UpdateUI();

	virtual BOOL OnInitDialog();

	void Reset();

protected:
	void _AddStackInfo(DebugStackInfo *info,int iStack);

	void _UpdateIndicator();

	void _RecalcLayout();

	CXTPOfficeBorder<CXTPReportControl,false> _report;
	CImageList _imagelist;



	BOOL _bRunning;
	BreakID _breakid;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnReportItemDblClick(NMHDR * pNotifyStruct, LRESULT * result);
};



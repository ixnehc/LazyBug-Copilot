#pragma once
#include "GuiLib.h"

#include "GuiEditor.h"

#include "ScintillaWnd.h"

#include "WorldSystem/IDebugger.h"

struct DebugOutput;


class COutputWnd:public CScintillaWnd
{
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT flag,CPoint pt);
	afx_msg void OnLButtonDblClk(UINT flag,CPoint pt);

};



class GuiLib_Api CDbgPanel_Output:public CGuiPanel
{
public:
	struct LineInfo
	{
		LineInfo()
		{
			protoid=ProtoID_Null;
			nodeid=ProtoNodeID_Null;
			iLine=-1;
		}
		ProtoID protoid;
		ProtoNodeID nodeid;
		int iLine;
	};


	CDbgPanel_Output(CWnd* pParent = NULL);
	virtual const char *GetName()	{		return "dbg_output";	}

	BOOL Create(CWnd *pParent);

	void ClearOutput();

	virtual void UpdateUI();

	virtual BOOL OnInitDialog();

	void Reset();

	void OnDblClickLine(int line);

	void OnDebugOutput(DebugOutput &o);


protected:


	void _SetDefaultFormat();

	void _RecalcLayout();


	COutputWnd*_wnd;

	std::vector<LineInfo> _lines;


	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnDblClick(NMHDR*, LRESULT*);
	afx_msg void OnContextMenu( CWnd *, CPoint );
	afx_msg void OnClear();
};




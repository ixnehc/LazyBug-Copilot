#pragma once
#include "GuiLib.h"

#include "GuiEditor.h"

#include "ScintillaWnd.h"


#include "WorldSystem/IDebugger.h"

class ILuaMachine;

class CHelp:public CScintillaWnd
{
public:

protected:

};


class GuiLib_Api CDbgPanel_LuaHelp:public CGuiPanel
{

public:
	CDbgPanel_LuaHelp(CWnd* pParent = NULL);
	virtual const char *GetName()	{		return "dbg_luahelp";	}

	BOOL Create(CWnd *pParent);

	virtual void UpdateUI();

	virtual BOOL OnInitDialog();

	void Reset();

protected:

	void _SetDefaultFormat(CScintillaWnd *wnd);

	void _SetKeywords(CScintillaWnd *wnd,ILuaMachine *lm);

	void _CollectKeys();

	void _UpdateHelp();
	void _UpdateHotSpotClick();

	void _RecalcLayout();

	ILuaMachine *_GetLM();
	IProtoLib *_GetProtoLib();

	std::string _keyHelp;
	std::vector<std::string> _keyhistory;

	DWORD _verHelp;

	CHelp *_wndHelp;

	std::vector<int> _funclib;
	std::vector<std::string> _funcnames;
	std::vector<std::string> _typenames;

	BOOL _bHotSpotClick;
	std::string _strClick;
	int _posClick;



	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
public:
	afx_msg void OnCbnSelchangeLib();
	afx_msg void OnHotSpotClick(NMHDR*, LRESULT*);
	afx_msg void OnBnClickedLib();
	afx_msg void OnLibCommand(UINT idCmd);
	afx_msg void OnBnClickedBack();

};



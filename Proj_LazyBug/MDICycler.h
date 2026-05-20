// MainFrm.h : interface of the CMainFrame class
//


#pragma once

//这个类提供了类似VisualStudio一样的窗口切换功能
class CMDICycler
{
public:
	CMDICycler()
	{
		_swap=1;
	}
	void SetMDIHwnd(CMDIFrameWnd* wnd)	{		_wnd=wnd;	}
	void Next();
	void Rest();
protected:
	void _EnumChilds(CWnd *wnd);
	void _UpdateQueue();
	CMDIFrameWnd*_wnd;//主窗口
	std::vector<HWND>_queue;
	std::vector<HWND>_childs;
	DWORD _swap;

};


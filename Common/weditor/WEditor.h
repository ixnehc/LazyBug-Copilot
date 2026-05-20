
#pragma once

#include "engine/Engine.h"

#include "DockPaneWnd.h"
#include "GuiEditor.h"


class CGuiPanelTabWnd;
class CWorldEditor
{
public:
	CWorldEditor()
	{
		_panemgr=NULL;
		_tabwndMain=NULL;
	}

	virtual BOOL Create(CXTPDockingPaneManager *panemgr)	{		_panemgr=panemgr;return TRUE;}
	virtual void Destroy()	{	}
	virtual BOOL LoadContent()		{ return TRUE;}
	virtual void ResetContent()	{	}
	virtual CGuiMgr*GetMgr()	{		return &_mgr;	}

	void OnDockPanel(CXTPDockingPane*pane);


	static void SetEngineSS();
	static void SetWorldSS();


protected:
	struct _PanelInfo
	{
		CWnd *panel;
		int id;
		int icon;
		CWnd *wnd;
	};
	template<typename T_panel>
	BOOL _AddPanel(CWnd *panel,int id,int icon,int dockdir=xtpPaneDockLeft,DWORD option=xtpPaneNoFloatable)
	{
		if (!_panemgr)
			return FALSE;
		CXTPDockingPane *pwndPane= _panemgr->CreatePane((UINT)id,CRect(0, 0,200, 120), (XTPDockingPaneDirection)dockdir);
		pwndPane->SetOptions(option);
		CDockPaneWnd<T_panel> *p=(CDockPaneWnd<T_panel> *)panel;
		if (!p->GetSafeHwnd())
			p->Create(_panemgr->GetParent());
		p->EnableWindow(FALSE);
		pwndPane->Attach(p);

		_PanelInfo pi;
		pi.panel=p;
		pi.id=id;
		pi.icon=icon;
		pi.wnd=p;

		_pis.push_back(pi);
		return TRUE;
	}
	template<typename T_panel>
	BOOL _AddSlidePanel(CWnd *panel,int id,int icon,int dockdir=xtpPaneDockLeft,DWORD option=xtpPaneNoFloatable)
	{
		if (!_panemgr)
			return FALSE;
		CXTPDockingPane *pwndPane= _panemgr->CreatePane((UINT)id,CRect(0, 0,200, 120), (XTPDockingPaneDirection)dockdir);
		pwndPane->SetOptions(option);
		CDockSlidePane<T_panel> *p=(CDockSlidePane<T_panel> *)panel;
		p->Create(_panemgr->GetParent(),id);
		p->EnableWindow(FALSE);
		pwndPane->Attach(p);

		_PanelInfo pi;
		pi.panel=p;
		pi.id=id;
		pi.icon=icon;
		pi.wnd=p->GetContainee();

		_pis.push_back(pi);
		return TRUE;
	}

	BOOL _SetMainTabWnd(CGuiPanelTabWnd*tab,int id,int icon,int idTabIcons,int dockdir=xtpPaneDockLeft,DWORD option=xtpPaneNoFloatable);

	
	void _ClearPanels();

	BOOL _SetPanelIcons(UINT idBmp);


	CXTPDockingPaneManager *_panemgr;

	std::vector<_PanelInfo> _pis;

	CGuiPanelTabWnd*_tabwndMain;

	CGuiMgr _mgr;


};


//用来响应打开各个view的命令
class CPanelViewer
{
public:
	CPanelViewer()
	{
		_panemgr=NULL;
	}
	void SetMgr(CXTPDockingPaneManager *mgr)
	{
		_panemgr=mgr;
	}
	BOOL Handle(UINT idCmd,BOOL bTest);//返回是否handled
	void Register(UINT idCmd,UINT idPanel);

protected:

	std::map<UINT,UINT>_cmdmap;//COMMAND ID到Panel ID的映射关系

	CXTPDockingPaneManager *_panemgr;

};


extern void Ssc_Connect();
extern void Ssc_Disconnect();
extern void Ssc_Change();
extern void Ssc_Change2();




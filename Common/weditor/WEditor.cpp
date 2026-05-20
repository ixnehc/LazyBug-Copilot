/********************************************************************
	created:	2007/2/15   13:04
	filename: 	e:\IxEngine\Proj_WorldEditor\WEditor.cpp
	author:		cxi
	
	purpose:	world editor base
*********************************************************************/

#include "stdh.h"
#include "commondefines/general_stl.h"
#include "RenderSystem/IRenderSystem.h"

#include "resource.h"

#include "ResTree.h"
#include "ResAnchor.h"
#include "RichGridTexItem.h"
#include "GObjGrid.h"

#include "GuiPanelTabWnd.h"

#include "WEditor.h"

#include "DockPaneWnd.h"

#include "editor/editor.h"

#include "GuiData.h"


//////////////////////////////////////////////////////////////////////////
//CWEGuiMgr


//////////////////////////////////////////////////////////////////////////
//CWorldEditor
BOOL CWorldEditor::_SetPanelIcons(UINT idBmp)
{
	if (!_panemgr)
		return FALSE;

	DWORD nIcons=0;
	for (int i=0;i<_pis.size();i++)
	{
		if (_pis[i].icon+1>nIcons)
			nIcons=_pis[i].icon+1;
	}
	std::vector<int>icons;
	icons.resize(nIcons);
	VEC_SET(icons,0);

	for (int i=0;i<_pis.size();i++)
		icons[_pis[i].icon]=_pis[i].id;

	if (icons.size()>0)
		_panemgr->SetIcons(idBmp,icons.data(),icons.size(),RGB(255, 0, 255));

	return TRUE;
}

void CWorldEditor::OnDockPanel(CXTPDockingPane*pane)
{
	if (pane->IsValid())
		return;

	for (int i=0;i<_pis.size();i++)
	{
		if (pane->GetID()==_pis[i].id)
			pane->Attach(_pis[i].panel);
	}
}

void CWorldEditor::_ClearPanels()
{
	for (int i=0;i<_pis.size();i++)
		_pis[i].panel->DestroyWindow();
	_tabwndMain=NULL;
}

extern CCurrentUserRegistry g_reg;
extern CSscSystemWrapper g_ssc;
extern CSscSystemWrapper g_ssc2;

void CWorldEditor::SetEngineSS()
{
	//设置GuiLib的SS
	g_ssGuiLib.pFS=g_Engine.GetFS();
}

void CWorldEditor::SetWorldSS()
{
}



BOOL CWorldEditor::_SetMainTabWnd(CGuiPanelTabWnd*tab,int id,int icon,int idTabIcons,int dockdir,DWORD option)
{
	if (_tabwndMain)
		return FALSE;

	if (FALSE==_AddPanel<CGuiPanelTabWnd>((CWnd*)tab,id,icon,dockdir,option))
		return FALSE;

	_tabwndMain=tab;

	_tabwndMain->GetImageManager()->SetIcons(idTabIcons, NULL, 0, CSize(16, 16), xtpImageNormal);
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CPanelViewer

//返回是否handled
void CPanelViewer::Register(UINT idCmd,UINT idPanel)
{
	_cmdmap[idCmd]=idPanel;
}

BOOL CPanelViewer::Handle(UINT idCmd,BOOL bTest)
{
	if (!_panemgr)
		return FALSE;
	std::map<UINT,UINT>::iterator it=_cmdmap.find(idCmd);
	if (it==_cmdmap.end())
		return FALSE;
	if (bTest)
		return TRUE;
	UINT idPanel=(*it).second;
	if (_panemgr->IsPaneHidden(idPanel)||_panemgr->IsPaneClosed(idPanel)||(!_panemgr->IsPaneSelected(idPanel)))
		_panemgr->ShowPane(idPanel,TRUE);

	return TRUE;

}


//一些Ssc相关的函数
// 
// void Ssc_Connect()
// {
// 	g_ssc.Init(g_Engine.GetSS(),g_Engine.GetFS());
// 	g_ssc.PromptConnect(g_reg,TRUE);
// 	g_ssc2.Init(g_Engine.GetSS2(),g_Engine.GetFS());
// 	if (g_ssc2.HasConfig(g_reg))
// 		g_ssc2.PromptConnect(g_reg,TRUE);
// 
// 	//连上ssc后,得一下StrLib的最新版本
// 	extern BOOL GetLatestStrLib(ISscSystem *pSS,CStrLib &strlib);
// 	extern BOOL LoadStrLib(CStrLib &strlib,IFileSystem *pFS);
// 	ISscSystem *pSS=g_ssc2.HasConfig(g_reg)?g_ssc2.GetSS():g_ssc.GetSS();
// 	if (GetLatestStrLib(pSS,*g_Engine.GetStrLib()))
// 		LoadStrLib(*g_Engine.GetStrLib(),g_Engine.GetFS());
// }
// 
// void Ssc_Disconnect()
// {
// 	g_ssc.Disconnect();
// 	g_ssc2.Disconnect();
// }
// 
// void Ssc_Change()
// {
// 	g_ssc.PromptConnect(g_reg,FALSE);
// }
// 
// void Ssc_Change2()
// {
// 	g_ssc2.PromptConnect(g_reg,FALSE);
// }
// 

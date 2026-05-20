#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>


#include "interface/interface.h"

#include "assert.h"
#include ".\restreepanel.h"

#include "FileSystem/FileSystemDefines.h"
#include "RenderSystem/IUtilRS.h"
#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IWorldSystem.h"

#include "Log/LogFile.h"

#include "resdata/ResData.h"

#include "stringparser/stringparser.h"

#include "Registry/Registry.h"


#include "TreeCtrlBase.h"

#include "resource.h"

#pragma warning(disable:4018)

#define ID_SETHISTORY 3721


//////////////////////////////////////////////////////////////////////////
//CResTreePanel
BEGIN_MESSAGE_MAP(CResTreePanel, CWnd)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_SETPATH, OnSetFolderPath)
	ON_NOTIFY(XTP_FN_SETFORMAT, ID_SETHISTORY, OnSetHistory)
	ON_WM_TIMER()
END_MESSAGE_MAP()

extern CCurrentUserRegistry g_reg;
void CResTreePanel::_LoadHistory()
{
	std::string s=g_reg.ReadString("ResEditor","ResPathHistory","");
	SplitStringBy(",",s,&_historyPath);
}

void CResTreePanel::_SaveHistory()
{
	std::string s;
	LinkStringBy(",",s,&_historyPath);

	g_reg.WriteString("ResEditor","ResPathHistory",s.c_str());
}

void CResTreePanel::_RefreshHistory()
{
	CXTPControlComboBox* pCombo=(CXTPControlComboBox*)_toolbar.GetControl(1);
	if (pCombo)
	{
		CString s=pCombo->GetText();
		pCombo->ResetContent();

		int iSel=-1;
		for (int i=0;i<_historyPath.size();i++)
		{
			pCombo->AddString(fromMBCS(_historyPath[i].c_str()));
			if (_historyPath[i] == toMBCS((LPCTSTR)s))
				iSel=i;
		}

		pCombo->SetCurSel(0);
	}

	if (_historyPath.size()>0)
		_sSel=_historyPath[0];
	else
		_sSel="";
}

void CResTreePanel::_AddToHistory(const char *path0)
{
	std::string pathRoot=g_ssGuiLib.pWS->GetPath(WSPath_DataRoot);
	std::string path=path0;
	if (CheckPathContaining(pathRoot.c_str(),path.c_str()))
		path=CutHeadPath(path.c_str(),pathRoot.c_str());
	for (int i=0;i<_historyPath.size();i++)
	{
		if (_historyPath[i]==path)
		{
			_historyPath.erase(_historyPath.begin()+i);
			break;
		}
	}

	_historyPath.insert(_historyPath.begin(),std::string(path));

	if (_historyPath.size()>20)
		_historyPath.resize(20);
}




BOOL CResTreePanel::Create(CWnd *pParent,RECT &rc,UINT id)
{
	if (FALSE==CWnd::CreateEx(0,_T("STATIC"), NULL, WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, CRect(0, 0, 1, 1),pParent, 0))
		return FALSE;

	return TRUE;

}


int CResTreePanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	_tree.Create(this,CRect(0,0,0,0),1000);

	//	//	COLORREF clrMask = XTPImageManager()->SetMaskColor(RGB(0, 255, 0));
	VERIFY(_toolbar.CreateToolBar(WS_TABSTOP|WS_VISIBLE|WS_CHILD|CBRS_TOOLTIPS, this));
	VERIFY(_toolbar.LoadToolBar(IDR_RESTREEBAR));
	//	//	XTPImageManager()->SetMaskColor(clrMask); 

	if (TRUE)
	{
		CXTPControlComboBox* pCombo= new CXTPControlComboBox();
		pCombo->SetDropDownListStyle(FALSE);

		pCombo->SetID(ID_SETHISTORY);

		pCombo->SetWidth(240);
		pCombo->SetDropDownWidth(400);

		_toolbar.GetControls()->Add(pCombo);
	}

	_LoadHistory();
	_RefreshHistory();


	_idTimer=(UINT)SetTimer((UINT_PTR)1,100,NULL);


	return 0;
}

CResTree *CResTreePanel::GetTree()
{
	return &_tree;
}
	
void CResTreePanel::OnDestroy()
{
	KillTimer(_idTimer);
	CWnd::OnDestroy();
}


void CResTreePanel::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	int nTop = 0;

	if (_toolbar.GetSafeHwnd())
	{
		CSize sz = _toolbar.CalcDockingLayout(cx, /*LM_HIDEWRAP|*/ LM_HORZDOCK|LM_HORZ | LM_COMMIT);

		_toolbar.MoveWindow(0, nTop, cx, sz.cy);
		_toolbar.Invalidate(FALSE);
		nTop += sz.cy;
	}
	if (_tree.GetSafeHwnd())
	{
		_tree.MoveWindow(0, nTop, cx, cy - nTop);
		_tree.Invalidate(FALSE);
	}
}

void CResTreePanel::OnSetFolderPath()
{
	CXTBrowseDialog dlg;
	dlg.SetTitle(_T("Select the resource root path:"));

	IRenderSystem *pRS=g_ssGuiLib.pRS;

	std::string path=_tree.GetRootPath();
	if (path=="")
		path=pRS->GetPath(Path_Res);

	dlg.SetSelPath(fromMBCS(path.c_str()));
	dlg.SetOptions(BIF_DONTGOBELOWDOMAIN);
	
	if (dlg.DoModal() == IDOK)
	{
		SetRootPathAndHistory(toMBCS(dlg.GetSelPath()));
	}
}

void CResTreePanel::SetRootPathAndHistory(const char *path)
{
	_AddToHistory(path);
	_RefreshHistory();
	_SaveHistory();
	_tree.SetContent(path);
}


void CResTreePanel::SetRootPath(const char *path)
{
	_tree.SetContent(path);
}

const char *CResTreePanel::GetRootPath()
{
	return _tree.GetRootPath();
}


void CResTreePanel::OnTimer(UINT_PTR idEvent)
{
	CXTPControlComboBox* pCombo=(CXTPControlComboBox*)_toolbar.GetControl(1);
	if (pCombo)
	{
		if (!pCombo->GetPopuped())
		{
			CString s=pCombo->GetText();
			if (s!=_sSel.c_str())
			{
				_AddToHistory(toMBCS((LPCTSTR)s));
				_RefreshHistory();
				_SaveHistory();
				std::string path=g_ssGuiLib.pWS->GetPath(WSPath_DataRoot);
				path = path + "\\" + toMBCS((LPCTSTR)s);
				_tree.SetContent(path.c_str());
			}
		}
	}

	_tree.Update();
}

void CResTreePanel::OnSetHistory(NMHDR* pNMHDR, LRESULT* pRes)
{
}

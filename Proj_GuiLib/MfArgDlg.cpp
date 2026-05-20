/********************************************************************
	created:	15:1:2009   13:16
	filename: 	d:\IxEngine\Proj_GuiLib\MfArgDlg.cpp
	author:		chenxi
	
	purpose:	dialog to edit map file args
*********************************************************************/

#include "stdh.h"
#include "resource.h"

#include "FileSystem/IMapFile.h"
#include "FileSystem/IFileSystem.h"

#include "MfArgDlg.h"

#include "WndBase.h"
#include ".\mfargdlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CMfArgDlg::CMfArgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_ASCFGDLG, pParent)
{

	_mf=NULL;

	_bNewAndAdd=FALSE;
}

BEGIN_MESSAGE_MAP(CMfArgDlg, CDialog)
	ON_BN_CLICKED(IDC_NEWANDADD, OnBnClickedNewandadd)
END_MESSAGE_MAP()


BOOL CMfArgDlg::Create(CWnd *pParent)
{
	return CDialog::Create(IDD_ASCFGDLG,pParent); 
}

// CMfArgDlg 消息处理程序

BOOL CMfArgDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	std::string path;
	_arg.GClear();

	CRect rc;
	GET_CONTROL_RECT(this,IDC_LIST,rc);
	HIDE_CONTROL(this,IDC_LIST);

	_grid.Create(rc,this,1);

	_grid.Bind(_arg.GetGObj());

	_grid.ExpandAll();

	HIDE_CONTROL(this,IDC_SSCBTN);

	CWnd *wnd=GetDlgItem(IDOK);
	wnd->SetWindowText(_T("New"));

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CMfArgDlg::GetArg(MapFileArgs &arg)
{
	arg=_arg.arg;
}

const char *CMfArgDlg::GetPath()
{
	return _arg.path.c_str();
}

void CMfArgDlg::OnOK()
{
	if (_arg.path=="")
	{
		AfxMessageBox(_T("Map file path not selected!"));
		return;
	}
	_bNewAndAdd=FALSE;
	return CDialog::OnOK();
}


void CMfArgDlg::OnBnClickedNewandadd()
{
	if (_arg.path=="")
	{
		AfxMessageBox(_T("Map file path not selected!"));
		return;
	}
	_bNewAndAdd=TRUE;

	return CDialog::OnOK();
}


//////////////////////////////////////////////////////////////////////////
//CMfArgDlg2

CMfArgDlg2::CMfArgDlg2(CWnd* pParent /*=NULL*/)
: CDialog(IDD_ASCFGDLG, pParent)
{
	_mf=NULL;

	_idTimer=0;
}

BEGIN_MESSAGE_MAP(CMfArgDlg2, CDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CMfArgDlg2::Create(CWnd *pParent)
{
	return CDialog::Create(IDD_ASCFGDLG,pParent); 
}

// CMfArgDlg2 消息处理程序

BOOL CMfArgDlg2::OnInitDialog()
{
	CDialog::OnInitDialog();

	assert(_mf);

	std::string path;
	MapFile_LoadUniqueGObj(_mf,MfArg_Name,_arg);
	path=_mf->GetUniqueSscPath(MfArg_Name);

	CRect rc;
	GET_CONTROL_RECT(this,IDC_LIST,rc);
	HIDE_CONTROL(this,IDC_LIST);

	HIDE_CONTROL(this,IDOK);

	_grid.Create(rc,this,1);
	_grid.Bind(_arg.GetGObj());
	_grid.ExpandAll();


	if (TRUE)
	{
		CXTPPropertyGridItem *item;
#define DISABLE_ITEM(path)															\
		item=_grid.ItemFromPath(path);											\
		if (item)																					\
		item->SetReadOnly();

		//the path format is so weird,huh?
		DISABLE_ITEM("\\Map File Path\\MapFileArgs");
		DISABLE_ITEM("\\Map Rect\\MapFileArgs");
		DISABLE_ITEM("\\Left\\Map Rect\\MapFileArgs");
		DISABLE_ITEM("\\Top\\Map Rect\\MapFileArgs");
		DISABLE_ITEM("\\Right\\Map Rect\\MapFileArgs");
		DISABLE_ITEM("\\Bottom\\Map Rect\\MapFileArgs");

		HIDE_CONTROL(this,IDC_NEWANDADD);

		RECT rc;
		GET_CONTROL_RECT(this,IDC_SSCBTN,rc);
		HIDE_CONTROL(this,IDC_SSCBTN);
		_btnSsc.Create(_T(""),WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,rc,this,IDC_SSCBTN);
		_btnSsc.SetFont(GetFont(),TRUE);
		_btnSsc.Bind(path.c_str(),g_ssGuiLib.ssc);

		SscBtnCallback dlgt;
		dlgt.bind(this,&CMfArgDlg2::_OnSave);
		_btnSsc.BindNotifySave(dlgt);
		dlgt.bind(this,&CMfArgDlg2::_OnLoad);
		_btnSsc.BindNotifyLoad(dlgt);

		CWnd *wnd=GetDlgItem(IDCANCEL);
		wnd->SetWindowText(_T("关闭"));
	}

	_idTimer=(UINT)SetTimer(2,10,NULL);

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CMfArgDlg2::OnDestroy()
{
	KillTimer(_idTimer);
	_OnSave();
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}

void CMfArgDlg2::OnTimer(UINT_PTR nIDEvent)
{
	CDialog::OnTimer(nIDEvent);
	assert(_mf);
	if (File_Default!=g_ssGuiLib.pFS->GetFileAttrAbs(_mf->GetUniqueSscPath(MfArg_Name)))
		_grid.EnableWindow(FALSE);
	else
		_grid.EnableWindow(TRUE);
}

BOOL CMfArgDlg2::_OnSave()
{
	MapFile_SaveUniqueGObj(_mf,MfArg_Name,_arg);
	return TRUE;
}

BOOL CMfArgDlg2::_OnLoad()
{
	MapFile_LoadUniqueGObj(_mf,MfArg_Name,_arg);

	_grid.Bind(_arg.GetGObj());
	_grid.ExpandAll();
	return TRUE;
}

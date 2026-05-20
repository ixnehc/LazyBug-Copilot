/********************************************************************
	created:	2007/2/6   14:22
	filename: 	e:\IxEngine\Proj_GuiLib\TrrnBrushLibDlg.cpp
	author:		cxi
	
	purpose:	Terrain Brush Lib Dialog
*********************************************************************/


#include "stdh.h"
#include "TrrnBrushLibDlg.h"
#include ".\trrnbrushlibdlg.h"

#include "WorldSystem/IWorldSystem.h"

#include "Registry/Registry.h"
#include "Log/LogFile.h"

#include "stringparser/stringparser.h"


#include "WndBase.h"

#include "ImageBase.h"

#include <assert.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#endif




//////////////////////////////////////////////////////////////////////////
//CTrrnBrushLibDlg

CTrrnBrushLibDlg::CTrrnBrushLibDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTrrnBrushLibDlg::IDD, pParent)
{
	m_hIcon = NULL;

	_pWS=NULL;
	_brlib=NULL;

	_bModified=FALSE;

	_bInSync=FALSE;


}

void CTrrnBrushLibDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//	DDX_Control(pDX, IDC_TABSLOT, _tabctrl);
}

BEGIN_MESSAGE_MAP(CTrrnBrushLibDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CTrrnBrushLibDlg::Create(CWnd *pParent)
{

	return CDialog::Create(IDD,pParent); 
}

// CTrrnBrushLibDlg 消息处理程序

BOOL CTrrnBrushLibDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	_bModified=FALSE;

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	assert(_pWS);
	_brlib=_pWS->CreateTrrnBrushLib();

	_imagelib.SetBrLib(_brlib);
	_imagelib.SetRS(_pWS->GetRS(),_pWS->GetUtilRS());

	CRect rc;
	GET_CONTROL_RECT(this,IDC_TABSLOT,rc);
	HIDE_CONTROL(this,IDC_TABSLOT);

	_tabctrl.Create(WS_VISIBLE|WS_CHILD|SS_NOTIFY|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
							rc, this,IDC_TABSLOT);


	_tabctrl.GetPaintManager()->SetAppearance(xtpTabAppearancePropertyPage2003);
	_tabctrl.GetPaintManager()->m_bHotTracking = TRUE;
	_tabctrl.GetPaintManager()->m_bShowIcons = TRUE;
	_tabctrl.GetPaintManager()->DisableLunaColors(FALSE);

	_texset.SetOwner(this);
	_brush.SetOwner(this);

	_texset.Create(&_tabctrl);
	_brush.Create(&_tabctrl);


	_tabctrl.InsertItem(0, _T("Tex Sets"), _texset.GetSafeHwnd());
	_tabctrl.InsertItem(1, _T("Brushes"),_brush.GetSafeHwnd());

	_SetTitle();

	if (!_Load())
		EndDialog(0);

	_idTimer=(UINT)SetTimer(2,10,NULL);
	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTrrnBrushLibDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CTrrnBrushLibDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
 


BOOL CTrrnBrushLibDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return CDialog::OnEraseBkgnd(pDC);
}

BOOL CTrrnBrushLibDlg::_Load()
{
	std::string s=_pWS->GetPath(WSPath_TrrnBrushLib);
	if (CheckPathContaining(s.c_str(),_filepath.c_str()))
	{
		s=CutHeadPath(_filepath.c_str(),s.c_str());
		_brlib->SetPath(s.c_str());
	}

	if (FALSE==_brlib->Load())
	{
		LogFile::Prompt("Failed to load brush lib file \"%s\"!",s.c_str());
		return FALSE;
	}
	_SetTitle();

	_imagelib.SyncForAll();

	Refresh(TRUE);
	return TRUE;
}


void CTrrnBrushLibDlg::OnDestroy()
{
	KillTimer(_idTimer);
	_imagelib.Clear();
	SAFE_RELEASE(_brlib);
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}


void CTrrnBrushLibDlg::Refresh(BOOL bReset)
{
	_texset.Refresh(bReset);
	_brush.Refresh(bReset);
}

void CTrrnBrushLibDlg::UpdateDueToPSTSChange()
{
	_brush.OnBrushSelChange();
}

void CTrrnBrushLibDlg::_SetTitle()
{
	std::string s="Trrn Brush Editor  ";
	if (_filepath!="")
		s=s+" - "+_filepath;
	else
		s=s+" - [No Name]";
	SetWindowText(fromMBCS(s.c_str()));
}


void CTrrnBrushLibDlg::OnOK()
{
	if (_brlib)
	{
		if (_brlib->IsModified())
			_brlib->Save();
	}

	CDialog::OnOK();
}

void CTrrnBrushLibDlg::OnCancel()
{
	OnOK();
}

void CTrrnBrushLibDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialog::OnTimer(nIDEvent);

	if (_brlib)
	{
		if (_brlib->IsModified())
		{
			_bModified=TRUE;
			_brlib->Save();
		}
	}

}


void CTrrnBrushLibDlg::SetLevelCombo(int src,int iLevel)
{
	if (_bInSync)
		return;
	_bInSync=TRUE;
	if (src==0)
		_brush.SetLevelCombo(iLevel);
	else
		_texset.SetLevelCombo(iLevel);
	_bInSync=FALSE;
}

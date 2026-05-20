/********************************************************************
	created:	2007/2/6   14:22
	filename: 	e:\IxEngine\Proj_GuiLib\TrrnBrushLibDlg.cpp
	author:		cxi
	
	purpose:	Terrain Brush Lib Dialog
*********************************************************************/


#include "stdh.h"
#include "TrrnBrushSelDlg.h"

#include <assert.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#endif




//////////////////////////////////////////////////////////////////////////
//CTrrnBrushSelDlg

CTrrnBrushSelDlg::CTrrnBrushSelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTrrnBrushSelDlg::IDD, pParent)
{
	_brlib=NULL;

	_sel=BRUSHID_INVALID;
}

void CTrrnBrushSelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//	DDX_Control(pDX, IDC_TABSLOT, _tabctrl);
}

BEGIN_MESSAGE_MAP(CTrrnBrushSelDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CTrrnBrushSelDlg 消息处理程序

BOOL CTrrnBrushSelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_BRUSHLIST);

	if (_brlib)
	{
		DWORD c=_brlib->GetBrushCount();
		for (int i=0;i<c;i++)
		{
			BrushID id=_brlib->GetBrushID(i);
			const char *nm=_brlib->GetBrushName(id);

			int idx = pCB->AddString(fromMBCS(nm));
			pCB->SetItemData(idx,id);
		}
	}

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTrrnBrushSelDlg::OnPaint() 
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
HCURSOR CTrrnBrushSelDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
 
void CTrrnBrushSelDlg::OnOK()
{
	if (_brlib)
	{
		CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_BRUSHLIST);
		if (pCB)
		{
			int idx=pCB->GetCurSel();
			if (idx>=0)
				_sel=(BrushID)pCB->GetItemData(idx);
		}
	}

	CDialog::OnOK();
}

void CTrrnBrushSelDlg::OnCancel()
{
	OnOK();
}

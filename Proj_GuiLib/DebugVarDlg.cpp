/********************************************************************
	created:	7:4:2009   13:16
	filename: 	d:\IxEngine\Proj_GuiLib\DebugVarDlg.cpp
	author:		chenxi
	
	purpose:	quick view dialog
*********************************************************************/

#include "stdh.h"

#include "commondefines/general_stl.h"
#include "resource.h"
#include "WndBase.h"
#include "DebugVarDlg.h"

#include "WorldSystem/IDebugger.h"

#include "stringparser/stringparser.h"



CDebugVarDlg::CDebugVarDlg(CWnd* pParent /*=NULL*/)
	: CXTPDialog(IDD_DEBUGVARDLG, pParent)
{
	m_hIcon = NULL;

	_dbgr=NULL;

}

BOOL CDebugVarDlg::Create(CWnd *pParent)
{

	return CXTPDialog::Create(IDD_DEBUGVARDLG,pParent); 
}


void CDebugVarDlg::DoDataExchange(CDataExchange* pDX)
{
	CXTPDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDebugVarDlg, CXTPDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(XTP_NM_REPORT_SELCHANGED, 0,OnReportSelChanged)
END_MESSAGE_MAP()


// CDebugVarDlg 消息处理程序

BOOL CDebugVarDlg::OnInitDialog()
{
	CXTPDialog::OnInitDialog();

	CRect rc;
	GET_CONTROL_RECT(this,IDC_LIST,rc);
	HIDE_CONTROL(this,IDC_LIST);

	if(FALSE==_grid.Create(rc,this,0))
		return FALSE;

	RemoveHeadBlank(_intialvar);
	RemoveTailBlank(_intialvar);


	_UpdateHistory();

	ShowWindow(SW_SHOWMAXIMIZED);

	CComboBox *pCB=(CComboBox*)GetDlgItem(IDC_COMBO);
	pCB->SetWindowText(fromMBCS(_intialvar.c_str()));


	OnOK();

	if (_intialvar=="")
		pCB->SetFocus();

// 	_RecalcLayout();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	return FALSE;  // 除非设置了控件的焦点，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDebugVarDlg::OnPaint() 
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
		CXTPDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CDebugVarDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
 

void CDebugVarDlg::OnDestroy()
{
	CXTPDialog::OnDestroy();

	// TODO: Add your message handler code here
}

void CDebugVarDlg::OnClose()
{
	//Clean the history

	CXTPDialog::OnClose();
}

void CDebugVarDlg::_RecalcLayout()
{
	i_math::recti rc,rc2,rc3;
	GetClientRect((LPRECT)&rc);

	rc.cutout(SIDE_TOP,32,rc2);


	rc2.cutout(SIDE_RIGHT,80,rc3);
	rc3.inflate(-4,-4,-4,-4);
	::SetWindowPos(GetDlgItem(IDC_ADDWATCH),rc3);

	rc2.cutout(SIDE_RIGHT,80,rc3);
	rc3.inflate(-4,-4,-4,-4);
	::SetWindowPos(GetDlgItem(IDOK),rc3);

	if (TRUE)
	{
		rc2.inflate(-4,-4,-4,-4);
		CComboBox *pCB=(CComboBox*)GetDlgItem(IDC_COMBO);
		if (pCB)
		{

			i_math::recti rcT;
			pCB->GetWindowRect((LPRECT)&rcT);
			rc2.Bottom()=rc2.Top()+rcT.getHeight();
			::SetWindowPos(pCB,rc2);
		}

//		pCB->SetDroppedWidth(80);
	}

	rc.inflate(-4,-4,-4,-4);
	::SetWindowPos(&_grid,rc);

}

void CDebugVarDlg::OnSize(UINT nType, int cx, int cy)
{
	CXTPDialog::OnSize(nType, cx, cy);

	_RecalcLayout();
}

void CDebugVarDlg::_UpdateHistory()
{
	CComboBox *pCB=(CComboBox*)GetDlgItem(IDC_COMBO);
	CString t;
	pCB->GetWindowText(t);
	pCB->ResetContent();

	for (int i=_history.size()-1;i>=0;i--)
		pCB->AddString(fromMBCS(_history[i].c_str()));

	pCB->SetWindowText(t);
}


void CDebugVarDlg::OnOK()
{
	CComboBox *pCB=(CComboBox*)GetDlgItem(IDC_COMBO);

	CString s;
	pCB->GetWindowText(s);

	_grid.ResetContent();

	DebugVarDesc* var = _dbgr->GetVar(toMBCS((LPCTSTR)s));

	if (var)
	{
		_grid.Add(*var);
		_varpath=var->path;

		//更新history
		if (TRUE)
		{
			std::string ss = toMBCS(s);
			VEC_REMOVE(_history,ss);
			_history.push_back(ss);
			_UpdateHistory();
		}


	}
	else
		_grid.AddInvalid(toMBCS((LPCTSTR)s));

	_grid.Populate();

	_grid.ExpandAll();

	_grid.SetFocus();

}

void CDebugVarDlg::Show(const char *var)
{
	_intialvar=var;
	DoModal();
}

void CDebugVarDlg::OnReportSelChanged(NMHDR* pNMHDR, LRESULT*result)
{
	std::string s=_grid.GetSelectVarPath();
	CComboBox *pCB=(CComboBox*)GetDlgItem(IDC_COMBO);
	if (s!="")
	{
		if (_varpath!="")
			s=_varpath+"."+s;
		pCB->SetWindowText(fromMBCS(s.c_str()));
	}

}

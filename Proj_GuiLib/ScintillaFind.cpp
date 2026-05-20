/********************************************************************
	created:	30:1:2010   10:19
	file path:	d:\IxEngine\Proj_GuiLib
	author:		chenxi
	
	purpose:	Scintilla Wnd的Find窗口
*********************************************************************/
#include "stdh.h"
#include "resource.h"
#include "ScintillaFind.h"

#include "WMGuiLib.h"



CScintillaFind::CScintillaFind(CWnd* pParent /*=NULL*/)
	: CXTPDialog(IDD_DIALOG_FIND, pParent)
{
	_bStartShow=FALSE;
	_bShow=FALSE;
}

void CScintillaFind::DoDataExchange(CDataExchange* pDX)
{
	CXTPDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CScintillaFind, CDialog)
// 	ON_WM_TIMER()
	//}}AFX_MSG_MAP
//  	ON_WM_DESTROY()
// 	ON_WM_CLOSE()
 	ON_BN_CLICKED(IDC_MATCHCASE, OnMatchCase)
	ON_BN_CLICKED(IDC_MATCHWORD, OnMatchWord)
	ON_BN_CLICKED(IDC_FINDINPROTO, OnFindInProto)
	ON_BN_CLICKED(IDC_FINDINALL, OnFindInAll)
END_MESSAGE_MAP()


BOOL CScintillaFind::Create(CWnd *pParent)
{

	return CXTPDialog::Create(IDD_DIALOG_FIND,pParent); 
}

// CScintillaFind 消息处理程序

BOOL CScintillaFind::OnInitDialog()
{
	CXTPDialog::OnInitDialog();



	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CScintillaFind::_CollectCmd(FindCmd &cmd)
{
	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_COMBO);
	CString s;
	pCB->GetWindowText(s);

	cmd.str = toMBCS((LPCTSTR)s);
	cmd.arg=_arg;

}


void CScintillaFind::OnOK()
{
	FindCmd cmd;
	_CollectCmd(cmd);
	cmd.op=FindCmd::Find;

	GetParent()->SendMessage(GLM_DoFindCommand,(WPARAM)&cmd,0);

//	return CDialog::OnOK();
}

void CScintillaFind::OnCancel()
{
	_bShow=FALSE;
	ShowWindow(SW_HIDE);
}

void CScintillaFind::SetText(const char *str)
{
	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_COMBO);
	pCB->SetWindowText(fromMBCS(str));
}

void CScintillaFind::_UpdateArg()
{
	CButton *btn;
	btn=(CButton *)GetDlgItem(IDC_MATCHCASE);
	BOOL bMatchCase=(btn->GetCheck()!=0);
	if (bMatchCase!=_arg.bMatchCase)
		btn->SetCheck(_arg.bMatchCase);
	btn=(CButton *)GetDlgItem(IDC_MATCHWORD);
	BOOL bMatchWord=(btn->GetCheck()!=0);
	if (bMatchWord!=_arg.bMatchWord)
		btn->SetCheck(_arg.bMatchWord);
}



void CScintillaFind::Update(CWnd *owner)
{
	if (!GetSafeHwnd())
		return;
	if (!_bShow)
		ShowWindow(SW_HIDE);
	else
	{
		if (!owner)
			ShowWindow(SW_HIDE);
		else
		{
			ShowWindow(SW_SHOW);
			CRect rc;
			owner->GetClientRect(&rc);
			CRect rcMe;
			GetClientRect(&rcMe);
			rc.left=rc.right-32-rcMe.Width();
			owner->ClientToScreen(&rc);
			ClientToScreen(rcMe);
			if ((rcMe.left!=rc.left)||(rcMe.top!=rc.top))
				SetWindowPos(NULL,rc.left,rc.top,0,0,SWP_NOZORDER|SWP_NOSIZE);

			_UpdateArg();
		}
	}

	if (_bStartShow)
	{
		SetFocus();
		_bStartShow=FALSE;
	}
}

void CScintillaFind::OnMatchCase()
{
	CButton *btn=(CButton*)GetDlgItem(IDC_MATCHCASE);
	_arg.bMatchCase=(btn->GetCheck()!=0);
}

void CScintillaFind::OnMatchWord()
{
	CButton *btn=(CButton*)GetDlgItem(IDC_MATCHWORD);
	_arg.bMatchWord=(btn->GetCheck()!=0);
}

void CScintillaFind::OnFindInProto()
{
	FindCmd cmd;
	_CollectCmd(cmd);
	cmd.op=FindCmd::FindInProto;

	GetParent()->SendMessage(GLM_DoFindCommand,(WPARAM)&cmd,0);

}

void CScintillaFind::OnFindInAll()
{
	FindCmd cmd;
	_CollectCmd(cmd);
	cmd.op=FindCmd::FindInAll;

	GetParent()->SendMessage(GLM_DoFindCommand,(WPARAM)&cmd,0);

}

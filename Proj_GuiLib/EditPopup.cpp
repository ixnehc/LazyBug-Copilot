/********************************************************************
	created:	2006/8/27   17:51
	filename: 	d:\IxEngine\Proj_GuiLib\AnchorPopup.cpp
	author:		ixnehc

	purpose:	popup for an edit control
	*********************************************************************/


#include "stdh.h"
#include "resource.h"
#include "EditPopup.h"

#include "WMGuiLib.h"

#include "WndBase.h"


CEditPopup::CEditPopup(CWnd* pParent /*=NULL*/)
	: CXTPDialog(IDD_EDITPOPUP, pParent)
{
}

void CEditPopup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEditPopup, CXTPDialog)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_KILLFOCUS()
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()



// CEditPopup 消息处理程序

const char *CEditPopup::Popup(int x,int y,const char *str)
{
	_x=x;
	_y=y;
	_str=str;
	DoModal();

	return _str.c_str();
}


BOOL CEditPopup::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rc;
	GetClientRect(&rc);
	CWnd *wnd=GetDlgItem(IDC_EDIT);
	::SetWindowPos(wnd,(i_math::recti&)rc);

	wnd->SetWindowText(fromMBCS(_str.c_str()));
	((CEdit*)wnd)->SetSel(0,-1);
	wnd->SetFocus();

	SetWindowPos(NULL,_x,_y,0,0,SWP_NOZORDER|SWP_NOSIZE);

	
	return FALSE;  // 除非设置了控件的焦点，否则返回 TRUE
}

 

void CEditPopup::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}

void CEditPopup::OnClose()
{
	//Clean the history

	CDialog::OnClose();
}


BOOL CEditPopup::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
	return CDialog::OnEraseBkgnd(pDC);
}

void CEditPopup::OnKillFocus(CWnd* pNewWnd)
{
	CDialog::OnKillFocus(pNewWnd);

	// TODO: Add your message handler code here
}

void CEditPopup::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
//	if (nState==WA_INACTIVE)
//		OnCancel();
	// TODO: Add your message handler code here
}


void CEditPopup::OnOK()
{
	CWnd *wnd=GetDlgItem(IDC_EDIT);
	CString s;
	wnd->GetWindowText(s);
	_str = toMBCS((LPCTSTR)s);

	CXTPDialog::OnOK();
}

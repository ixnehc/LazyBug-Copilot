// DlgCheckoutPrompt.cpp : implementation file
//

#include "stdh.h"
#include "DlgSscChoose.h"
#include ".\dlgsscchoose.h"


// CDlgSscChoose dialog
IMPLEMENT_DYNAMIC(CDlgSscChoose, CXTPDialog)
CDlgSscChoose::CDlgSscChoose(CWnd* pParent /*=NULL*/)
: CXTPDialog(CDlgSscChoose::IDD, pParent)
{
}

CDlgSscChoose::~CDlgSscChoose()
{
}

void CDlgSscChoose::DoDataExchange(CDataExchange* pDX)
{
	CXTPDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSscChoose, CXTPDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgSscChoose message handlers

BOOL CDlgSscChoose::OnInitDialog()
{
	CXTPDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetDlgItemText(IDC_CONTENT, _strContent);
	((CButton*) GetDlgItem(IDC_LEAVE))->SetCheck(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSscChoose::OnBnClickedOk()
{
	_bLeave = (IDC_LEAVE == GetCheckedRadioButton(IDC_LEAVE, IDC_REPLACE));
	_bApplyAllItems = (((CButton*) GetDlgItem(IDC_APPLYALL))->GetCheck() == 1);
	
	CXTPDialog::OnOK();
}

// DlgCheckoutPrompt.cpp : implementation file
//

#include "stdh.h"
#include "DlgVssChoose.h"
#include ".\dlgvsschoose.h"


// CDlgVssChoose dialog
BOOL CDlgVssChoose::ms_bLeave			= FALSE;
BOOL CDlgVssChoose::ms_bApplyAllItems	= FALSE;

IMPLEMENT_DYNAMIC(CDlgVssChoose, CXTPDialog)
CDlgVssChoose::CDlgVssChoose(CWnd* pParent /*=NULL*/)
: CXTPDialog(CDlgVssChoose::IDD, pParent)
{
}

CDlgVssChoose::~CDlgVssChoose()
{
}

void CDlgVssChoose::DoDataExchange(CDataExchange* pDX)
{
	CXTPDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgVssChoose, CXTPDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgVssChoose message handlers

BOOL CDlgVssChoose::OnInitDialog()
{
	CXTPDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetDlgItemText(IDC_CONTENT, _strContent);
	((CButton*) GetDlgItem(IDC_LEAVE))->SetCheck(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgVssChoose::OnBnClickedOk()
{
	ms_bLeave = (IDC_LEAVE == GetCheckedRadioButton(IDC_LEAVE, IDC_REPLACE));
	ms_bApplyAllItems = (((CButton*) GetDlgItem(IDC_APPLYALL))->GetCheck() == 1);
	
	CXTPDialog::OnOK();
}

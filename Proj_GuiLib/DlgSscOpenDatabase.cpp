// DlgSscOpenDatabase.cpp : implementation file
//

#include "stdh.h"
#include "DlgSscOpenDatabase.h"
#include ".\dlgsscopendatabase.h"


// CDlgSscOpenDatabase dialog

IMPLEMENT_DYNAMIC(CDlgSscOpenDatabase, CXTPDialog)
CDlgSscOpenDatabase::CDlgSscOpenDatabase(CWnd* pParent /*=NULL*/)
	: CXTPDialog(CDlgSscOpenDatabase::IDD, pParent)
	, _strDatabase(_T(""))
	, _strUser(_T(""))
	, _strPassword(_T(""))
{
}

CDlgSscOpenDatabase::~CDlgSscOpenDatabase()
{
}

void CDlgSscOpenDatabase::DoDataExchange(CDataExchange* pDX)
{
	CXTPDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DATABASE, _strDatabase);
	DDX_Text(pDX, IDC_USER, _strUser);
	DDX_Text(pDX, IDC_PASSWORD, _strPassword);
}


BEGIN_MESSAGE_MAP(CDlgSscOpenDatabase, CXTPDialog)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgSscOpenDatabase message handlers

void CDlgSscOpenDatabase::OnBnClickedBrowse()
{
	CFileDialog dlg(TRUE, _T(".INI"), _T("c:\\vss.ini"), OFN_HIDEREADONLY, _T("SourceSafe Database (*.ini)|*.ini|All Files (*.*)|*.*||"), this);
//	dlg.m_ofn.Flags         &= ~OFN_EXPLORER;
	if (dlg.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_DATABASE, dlg.GetPathName());
	}
}

void CDlgSscOpenDatabase::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	if (_strDatabase.IsEmpty() || _strUser.IsEmpty())
	{
		// Failed
		LPCTSTR DOD_CAPTION = _T("Open SourceSafe Database");
		LPCTSTR DOD_NOTEMPTY = _T("The Database and user are not empty, please re-enter.");
		MessageBox(DOD_NOTEMPTY, DOD_CAPTION, MB_OK);
	}
	else
	{
		CXTPDialog::OnOK();
	}
}
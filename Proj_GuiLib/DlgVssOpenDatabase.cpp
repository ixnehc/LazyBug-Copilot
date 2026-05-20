// DlgVssOpenDatabase.cpp : implementation file
//

#include "stdh.h"
#include "DlgVssOpenDatabase.h"
#include ".\dlgvssopendatabase.h"


// CDlgVssOpenDatabase dialog

IMPLEMENT_DYNAMIC(CDlgVssOpenDatabase, CXTPDialog)
CDlgVssOpenDatabase::CDlgVssOpenDatabase(CWnd* pParent /*=NULL*/)
	: CXTPDialog(CDlgVssOpenDatabase::IDD, pParent)
	, _strDatabase(_T(""))
	, _strUser(_T(""))
	, _strPassword(_T(""))
{
}

CDlgVssOpenDatabase::~CDlgVssOpenDatabase()
{
}

void CDlgVssOpenDatabase::DoDataExchange(CDataExchange* pDX)
{
	CXTPDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DATABASE, _strDatabase);
	DDX_Text(pDX, IDC_USER, _strUser);
	DDX_Text(pDX, IDC_PASSWORD, _strPassword);
}


BEGIN_MESSAGE_MAP(CDlgVssOpenDatabase, CXTPDialog)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgVssOpenDatabase message handlers

void CDlgVssOpenDatabase::OnBnClickedBrowse()
{
	CFileDialog dlg(TRUE, ".INI", NULL, OFN_HIDEREADONLY, "SourceSafe Database (*.ini)|*.ini|All Files (*.*)|*.*||", this);
	if (dlg.DoModal() == IDOK)
	{
		CString strPath = dlg.GetPathName();
		int nPos = strPath.ReverseFind('\\');
		if (nPos > 0)
		{
			SetDlgItemText(IDC_DATABASE, strPath.Left(nPos));
		}
	}
}

void CDlgVssOpenDatabase::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	if (_strDatabase.IsEmpty() || _strUser.IsEmpty())
	{
		// Failed
		LPCTSTR DOD_CAPTION = "Open SourceSafe Database";
		LPCTSTR DOD_NOTEMPTY = "The Database and user are not empty, please re-enter.";
		MessageBox(DOD_NOTEMPTY, DOD_CAPTION, MB_OK);
	}
	else
	{
		CXTPDialog::OnOK();
	}
}
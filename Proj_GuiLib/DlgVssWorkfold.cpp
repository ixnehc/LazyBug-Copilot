// DlgVssWorkfold.cpp : implementation file
//

#include "stdh.h"
#include "DlgVssWorkfold.h"
#include ".\dlgvssworkfold.h"
#include "stringparser/stringparser.h"
#include "WndBase.h"
#include "TreeCtrlBase.h"

extern HINSTANCE g_hInstance;

// CDlgVssWorkfold dialog

IMPLEMENT_DYNAMIC(CDlgVssWorkfold, CXTPDialog)
CDlgVssWorkfold::CDlgVssWorkfold(CWnd* pParent /*=NULL*/)
: CXTPDialog(CDlgVssWorkfold::IDD, pParent)
	, _strProject(_T(""))
	, _strWorkingFolder(_T(""))
	, _bEditableWorkingFolder(FALSE)
{
}

CDlgVssWorkfold::~CDlgVssWorkfold()
{
}

void CDlgVssWorkfold::DoDataExchange(CDataExchange* pDX)
{
	CXTPDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_WORKINGFOLDER, _strWorkingFolder);
	DDX_Control(pDX, IDC_PROJECTS, _tvProjects);
}


BEGIN_MESSAGE_MAP(CDlgVssWorkfold, CXTPDialog)
	ON_BN_CLICKED(IDC_APPLY, OnBnClickedApply)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
	ON_NOTIFY(NM_CLICK, IDC_PROJECTS, OnNMClickProjects)
END_MESSAGE_MAP()


// CDlgVssWorkfold message handlers

// From: CViewPackageDlg
HTREEITEM ChildItemFromName(CTreeCtrl *pTreeCtrl,const char *name,HTREEITEM hItem0)
{
	HTREEITEM hItem = pTreeCtrl->GetChildItem(hItem0);
	CString ss;
	ss=name;
	ss.MakeUpper();

	while (hItem != NULL)
	{
		CString s;
		s=pTreeCtrl->GetItemText(hItem);
		s.MakeUpper();

		if (s==ss)
			return hItem;
		hItem = pTreeCtrl->GetNextSiblingItem(hItem);
	}

	return hItem;
}

void CDlgVssWorkfold::SetProjectItems(const char**& items, int& nCount)
{
	for (int i = 0; i < nCount; i++)
	{
		_items.push_back(std::string(items[i]));
	}
}

BOOL CDlgVssWorkfold::OnInitDialog()
{
	CXTPDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CreateImageList(_imageset, IDB_FILESAVE, 16, 16);
	_tvProjects.SetImageList(&_imageset, TVSIL_NORMAL);

	if (!_bEditableWorkingFolder)
	{
		GetDlgItem(IDC_WORKINGFOLDER)->EnableWindow(FALSE);
		GetDlgItem(IDC_BROWSE)->EnableWindow(FALSE);
	}

	std::string sep = "/";
	int nCount = static_cast<int>(_items.size());
	for (int i = 0; i < nCount; i++)
	{
		std::vector<std::string> vecTemp;
		SplitStringBy("/", _items[i], &vecTemp);

		int count = static_cast<int>(vecTemp.size());
		HTREEITEM hItem = TVI_ROOT;
		for (int ii = 0; ii < count; ii++)
		{
			if (vecTemp[ii].empty())
				continue;

			HTREEITEM hItemSub;
			hItemSub = ChildItemFromName(&_tvProjects,vecTemp[ii].c_str(),hItem);

			if (hItemSub==NULL)
			{
				//if (ii < (count - 1))
					hItemSub=_tvProjects.InsertItem(vecTemp[ii].c_str(),0,0,hItem);
				//else
				//	hItemSub=_tvProjects.InsertItem(vecTemp[ii].c_str(),0,0,hItem);
			}

			hItem=hItemSub;
		}
		//if (!_items[i].empty())
		//	InsertItemByPath(&_tvProjects, _items[i], sep);
	}
	_tvProjects.Expand(_tvProjects.GetRootItem(), TVE_EXPAND);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgVssWorkfold::OnBnClickedApply()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (_bEditableWorkingFolder && _strWorkingFolder.IsEmpty())
	{
		const char* const NOTEMPTY	= "The working folder is not empty, please re-enter.";
		MessageBox(NOTEMPTY, DSWF_CAPTION, MB_OK);
		return;
	}

	HTREEITEM hSelItem = _tvProjects.GetSelectedItem();
	if (hSelItem)
	{
		std::string proj = PathFromItem(&_tvProjects, hSelItem, "/");
		_strProject = proj.c_str();		

		CXTPDialog::OnOK();
	}
}

void CDlgVssWorkfold::OnBnClickedBrowse()
{
	// TODO: Add your control notification handler code here
	CXTBrowseDialog dlg;
	dlg.SetTitle((char*) DSWF_CAPTION);
	if (dlg.DoModal() == IDOK)
	{
		_strWorkingFolder = dlg.GetSelPath();
		SetDlgItemText(IDC_WORKINGFOLDER, _strWorkingFolder);
	}
}

void CDlgVssWorkfold::OnNMClickProjects(NMHDR *pNMHDR, LRESULT *pResult)
{
	CPoint pt;
	GetCursorPos(&pt);
	_tvProjects.ScreenToClient(&pt);

	UINT uFlags = 0;
	HTREEITEM hSelItem = _tvProjects.HitTest(pt, &uFlags);
	if ((hSelItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		std::string proj = PathFromItem(&_tvProjects, hSelItem, "/");
		SetDlgItemText(IDC_CURPROJECT, proj.c_str());
	}	
}
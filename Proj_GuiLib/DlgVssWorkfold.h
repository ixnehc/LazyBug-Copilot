#pragma once
#include "afxcmn.h"
//#include <XTToolkitPro.h>       // Xtreme Toolkit support
#include <vector>

// CDlgVssWorkfold dialog
const TCHAR* DSWF_CAPTION	= _T("Set the working folder");

class CDlgVssWorkfold : public CXTPDialog
{
	DECLARE_DYNAMIC(CDlgVssWorkfold)

public:
	CDlgVssWorkfold(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgVssWorkfold();

// Dialog Data
	enum { IDD = IDD_VSS_WORKFOLD };

public:
	void SetProjectItems(const char**& items, int& nCount);

public:
	const CString& GetCurrentProject() const
	{
		return _strProject;
	}
	void EnableWorkingFolderEditable(bool bSetting)
	{
		_bEditableWorkingFolder = bSetting;
	}
	void SetWorkingFolder(const char* workingFolder)
	{
		_strWorkingFolder = workingFolder;
	}
	const char* GetWorkingFolder() const
	{
		return _strWorkingFolder;
	}

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnNMClickProjects(NMHDR *pNMHDR, LRESULT *pResult);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	std::vector<std::string> _items;
	BOOL _bEditableWorkingFolder;
	CString _strProject;
	CString _strWorkingFolder;
	CTreeCtrl _tvProjects;
	CImageList _imageset;
};

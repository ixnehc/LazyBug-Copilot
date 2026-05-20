#pragma once

#include "resource.h"


// CRefResDlg dialog

class CRefResDlg : public CXTPDialog
{
	DECLARE_DYNAMIC(CRefResDlg)

public:
	CRefResDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRefResDlg();

	void Set(std::vector<std::string> &buf)
	{
		_buf=buf;
	}

// Dialog Data
	enum { IDD = IDD_REFRESDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	afx_msg void OnCheckIn();
	afx_msg void OnCheckOut();
	afx_msg void OnGet();
	afx_msg void OnSelectAll();
	afx_msg void OnDeselectAll();

	void SelectAll();
	void DeselectAll();

protected:
	BOOL _CanOp();
	void _AddRecord(const char *path,const char *pathAbs);
	void _UpdateState();

	BOOL _UpdateRef();//返回是否有新增的路径

	std::vector<std::string> _buf;

	CImageList _imglist;
	CXTPReportControl _list;

};

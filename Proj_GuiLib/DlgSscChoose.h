#pragma once

#include "resource.h"


// CDlgCheckoutPrompt dialog

class CDlgSscChoose : public CXTPDialog
{
	DECLARE_DYNAMIC(CDlgSscChoose)

public:
	CDlgSscChoose(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSscChoose();

	// Dialog Data
	enum { IDD = IDD_SSC_CHOOSE };

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedOk();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CString _strContent;

	BOOL _bLeave;
	BOOL _bApplyAllItems;
};

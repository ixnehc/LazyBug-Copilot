#pragma once


// CDlgCheckoutPrompt dialog

class CDlgVssChoose : public CXTPDialog
{
	DECLARE_DYNAMIC(CDlgVssChoose)

public:
	CDlgVssChoose(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgVssChoose();

	// Dialog Data
	enum { IDD = IDD_VSS_CHOOSE };

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedOk();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CString _strContent;

	static BOOL ms_bLeave;
	static BOOL ms_bApplyAllItems;
};

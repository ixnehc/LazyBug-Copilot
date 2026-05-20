
#pragma once
#include "GuiLib.h"

#include "DebugVarGrid.h"

class IDebugger;


// CDebugVarDlg 对话框
class GuiLib_Api CDebugVarDlg : public CXTPDialog
{
// 构造
public:
	CDebugVarDlg(CWnd* pParent = NULL);	// 标准构造函数
	BOOL Create(CWnd *pParent);

	void SetDebugger(IDebugger *dbgr)	{		_dbgr=dbgr;	}
	void Show(const char *var);

// 对话框数据
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	virtual void OnOK();
// 	virtual void OnCancel(){ShowWindow(SW_HIDE);	}


// 实现
protected:

	void _RecalcLayout();
	void _UpdateHistory();

	CDebugVarGrid _grid;

	IDebugger *_dbgr;

	std::string _intialvar;

	std::string _varpath;

	std::vector<std::string>_history;

	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void OnReportSelChanged(NMHDR* pNMHDR, LRESULT*result);
};

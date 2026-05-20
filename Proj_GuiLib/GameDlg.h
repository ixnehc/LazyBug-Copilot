
#pragma once
#include "GuiLib.h"

#include "GObjGrid.h"


// CGameDlg 对话框
class GuiLib_Api CGameDlg : public CDialog
{
// 构造
public:
	CGameDlg(CWnd* pParent = NULL);	// 标准构造函数

	void BindGObj(GObjBase *obj)	{		_obj=obj;	}

// 对话框数据
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	CGObjGrid _grid;
	GObjBase *_obj;

	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnClose();
};

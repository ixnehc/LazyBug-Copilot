
#pragma once
#include "GuiLib.h"


#include "ResTree.h"

class CResAnchorBase;
// CEditPopup 对话框
class GuiLib_Api CEditPopup : public CXTPDialog
{
// 构造
public:
	CEditPopup(CWnd* pParent = NULL);	// 标准构造函数

	const char *Popup(int x,int y,const char *str);


// 对话框数据
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual void OnOK();


// 实现
protected:

	int _x,_y;

	std::string _str;


	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
};


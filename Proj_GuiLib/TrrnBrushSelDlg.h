
#pragma once
#include "GuiLib.h"

#include "resource.h"

#include "WorldSystem/ITrrn.h"


// CTrrnBrushSelDlg 对话框
class GuiLib_Api CTrrnBrushSelDlg : public CDialog
{
// 构造
public:
	CTrrnBrushSelDlg(CWnd* pParent = NULL);	// 标准构造函数
	
	void SetBrushLib(ITrrnBrushLib *brlib)
	{
		_brlib=brlib;
	}

	BrushID GetSel()	{		return _sel;	}

// 对话框数据
	enum { IDD = IDD_TRRNBRUSHSELDLG};

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	virtual void OnOK();
	virtual void OnCancel();



// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:

	BOOL _Load();

	ITrrnBrushLib *_brlib;
	BrushID _sel;
	

public:
protected:

public:
};

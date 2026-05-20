
#pragma once
#include "GuiLib.h"

#include "RichGrid.h"

#include "config/config.h"

class CCfgGrid:public CRichGrid
{
public:
	void Bind(CConfig *cfg);
protected:
	void _Insert(CConfig*cfg,std::vector<std::string>&pieces,const char *entry);

};

class IMapFile;
// CCfgDlg 对话框
class GuiLib_Api CCfgDlg : public CDialog
{
// 构造
public:
	CCfgDlg(CWnd* pParent = NULL);	// 标准构造函数

	BOOL Create(CWnd *pParent);

	void SetCfg(CConfig *cfg)	{		_cfg=cfg;	}


// 对话框数据
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual void OnOK();

// 实现
protected:
	CConfig *_cfg;
	CConfig _cfg2;

	CCfgGrid _grid;
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

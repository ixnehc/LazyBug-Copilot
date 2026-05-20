
#pragma once
#include "GuiLib.h"

#include "resource.h"


#include "TexWnd.h"

#include "TBLTexSetDlg.h"
#include "TBLBrushDlg.h"

#include "TBLImageLib.h"


// CTrrnBrushLibDlg 对话框
class GuiLib_Api CTrrnBrushLibDlg : public CDialog
{
// 构造
public:
	CTrrnBrushLibDlg(CWnd* pParent = NULL);	// 标准构造函数
	BOOL Create(CWnd *pParent);

	void SetWS(IWorldSystem *pWS)	{		_pWS=pWS;	}
	void SetPath(const char *path)	{		_filepath=path;	}//完全路径,在DoModal()之前设

	BOOL IsModified()	{		return _bModified;	}//在DoModal()后调
		
	ITrrnBrushLib *GetBrLib()	{		return _brlib;	}
	CTBLImageLib *GetImageLib()	{		return &_imagelib;	}

	void Refresh(BOOL bReset);
	void UpdateDueToPSTSChange();

	void SetLevelCombo(int src,int iLevel);


// 对话框数据
	enum { IDD = IDD_TRRNBRUSHLIBDLG};

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

	IWorldSystem *_pWS;
	ITrrnBrushLib *_brlib;
	CTBLImageLib _imagelib;

	CXTPTabControl _tabctrl;

	CTBLTexSetDlg _texset;
	CTBLBrushDlg _brush;

	BOOL _bInSync;


public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
protected:
	void _SetTitle();
	std::string _filepath;
	UINT _idTimer;

	BOOL _bModified;


public:
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedLoad();
	afx_msg void OnBnClickedSaveas();
	afx_msg void OnBnClickedNew();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

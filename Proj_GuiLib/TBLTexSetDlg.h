
#pragma once
#include "GuiLib.h"

#include "resource.h"

#include "TBLBaseDlg.h"

#include "EditListBoxEx.h"

class CxImage;

class CTexSetList:public CXTEditListBoxEx
{
public:
	CTexSetList()
	{
	}
	void Refresh(BOOL bReset);

	virtual void OnNewItem(int iItem,const char *name);
	virtual void OnDeleteItem();
	virtual void OnMoveItemUp();
	virtual void OnMoveItemDown();
	virtual void OnSelChange();

	DWORD GetTTSID();

protected:
	virtual BOOL _CheckItemName(int iItem,const char *name);
	std::string _nameSel;

	ITrrnBrushLib *_GetBrLib()	{	return ((CTBLBaseDlg*)GetParent())->GetBrLib();	}
	CTBLImageLib *_GetImageLib()	{	return ((CTBLBaseDlg*)GetParent())->GetImageLib();	}
	int _GetBrushLevel();

	friend class CTBLPttnSetDlg;
};

class CTexCtrl:public CWnd
{
public:
	CTexCtrl();
	~CTexCtrl();
	void SetTexSet(DWORD idPS);

	void Create(RECT &rc,UINT id,CWnd *pParent);
	void EnableEdit(BOOL bEnable)	{		_bEnableEdit=bEnable;	}
	void EnableNS(BOOL bNS)	{		_bSupportNS=bNS;	}

protected:
	void _Draw(CDC *pDC);
	virtual CTBLImageLib *_GetImageLib()	{	return ((CTBLBaseDlg*)GetParent())->GetImageLib();	}
	void _LoadImages();
	DWORD _idTS;
	int _iTex;

	CBitmap _bmpCompDC;

	CxImage *_imageBlank;

	BOOL _bEnableEdit;

	BOOL _bSupportNS;


public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnAdd();
	afx_msg void OnReplace();
	afx_msg void OnReplaceNS();
	afx_msg void OnRemoveNS();
	afx_msg void OnDelete();

protected:
	void _OnReplace(BOOL bNS);

	friend class CTBLPttnSetDlg;
};


// CTBLTexSetDlg 对话框
class CTBLTexSetDlg : public CTBLBaseDlg
{
// 构造
public:
	CTBLTexSetDlg(CWnd* pParent = NULL);	// 标准构造函数

	BOOL Create(CWnd *pParent);

	virtual void Refresh(BOOL bReset);//due to the brush lib modifying
	int GetBrushLevel()	{		return _iBrushLevel;	}

	void OnTexSetSelChange();

	virtual void SetLevelCombo(int iLevel);

	virtual void OnOK();


// 对话框数据
	enum { IDD = IDD_TBL_TEXSET};

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

protected:
	CTexSetList _list;
	int _iBrushLevel;

	CTexCtrl _texctrl;
	int _iTex;

	UINT _idTimer;

	int _clock;



// 实现
protected:

	HICON m_hIcon;


	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nID);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnCbnSelchangeLevelcombo();
	afx_msg void OnCbnSelchangeLenslot();
};

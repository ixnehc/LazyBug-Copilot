
#pragma once
#include "GuiLib.h"

#include "resource.h"

#include "WorldSystem/ITrrn.h"

#include "TBLBaseDlg.h"

#include "TBLTexSetDlg.h"


class CChangeBrIDDlg : public CDialog
{
	DECLARE_DYNAMIC(CChangeBrIDDlg)

public:
	CChangeBrIDDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChangeBrIDDlg();

	// Dialog Data
	enum { IDD = IDD_CHANGEBRUSHIDDLG };

	void SetInfo(BYTE idBr,ITrrnBrushLib *brlib)
	{
		_idBr=idBr;
		_brlib=brlib;
	}
	BYTE GetBrushID()	{		return _idBr;	}

protected:
	BYTE _idBr;
	ITrrnBrushLib *_brlib;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();


	afx_msg void OnCbnSelchangeCombo();
};


class CBrushList:public CXTEditListBoxEx
{
public:
	CBrushList()
	{
	}
	void Refresh(BOOL bReset);

	virtual void OnNewItem(int iItem,const char *name);
	virtual void OnDeleteItem();
	virtual void OnMoveItemUp();
	virtual void OnMoveItemDown();
	virtual void OnSelChange();

	BYTE GetBrushID();

protected:
	virtual BOOL _CheckItemName(int iItem,const char *name);
	std::string _nameSel;

	ITrrnBrushLib *_GetBrLib()	{	return ((CTBLBaseDlg*)GetParent())->GetBrLib();	}
	CTBLImageLib *_GetImageLib()	{	return ((CTBLBaseDlg*)GetParent())->GetImageLib();	}
	int _GetBrushLevel();

	friend class CTBLPttnSetDlg;
};


// CTBLBrushDlg 对话框
class CTBLBrushDlg : public CTBLBaseDlg
{
// 构造
public:
	CTBLBrushDlg(CWnd* pParent = NULL);	// 标准构造函数

	BOOL Create(CWnd *pParent);

// 对话框数据
	enum { IDD = IDD_TBL_BRUSH};

	virtual void Refresh(BOOL bReset);//due to the brush lib modifying

	void OnBrushSelChange();
	int GetBrushLevel()	{		return _iBrushLevel;	}

	virtual void SetLevelCombo(int iLevel);

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


protected:
	void _RefreshFallBack(BrushID idSel);
	int _iBrushLevel;
	CBrushList _list;

	CTexCtrl _texctrl;
	CTexCtrl _texctrl2;


// 实现
protected:

	HICON m_hIcon;


	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnCbnSelchangeLevelcombo();
	afx_msg void OnCbnSelchangeTexsetcombo();
	afx_msg void OnCbnSelchangeRepeatcombo();
	afx_msg void OnCbnSelchangeFallBackcombo();
	afx_msg void OnBnClickedChangebrid();
};

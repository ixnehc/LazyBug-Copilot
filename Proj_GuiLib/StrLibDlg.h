
#pragma once
#include "GuiLib.h"

#include "strlib/strlib.h"

#include "EditListBoxEx.h"

#include <map>


class CStrLibDlg;
class CStrLibList;

class CReportItemCheck : public CXTPReportRecordItem
{
	DECLARE_SERIAL(CReportItemCheck)
public:
	// Constructs record item with the initial checkbox value.
	CReportItemCheck(BOOL bCheck = FALSE);


	// Provides custom records comparison by this item based on checkbox value, 
	// instead of based on captions.
	virtual int Compare(CXTPReportColumn* pColumn, CXTPReportRecordItem* pItem);
};

class CReportItemGroups:public CXTPReportRecordItemText
{
	DECLARE_SERIAL(CReportItemGroups)
public:
	CReportItemGroups()
	{
		_owner=NULL;
		_iCategory=STRLIB_CATEGORY_DEFAULT;
	}
	// Constructs record item with the initial checkbox value.
	CReportItemGroups(const char *str);

	virtual void OnInplaceButtonDown(CXTPReportInplaceButton* pButton);

	CStrLibList*_owner;
	DWORD _iCategory;

	std::vector<StringID>_grps;

	friend class CStrLibList;

};

class CReportRecord:public CXTPReportRecord
{
public:
	void GetItemMetrics (XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs, XTP_REPORTRECORDITEM_METRICS* pItemMetrics)
	{
		int nIndex = pDrawArgs->pRow->GetIndex();

		if (nIndex % 2)
		{
			pItemMetrics->clrBackground = RGB(245, 245, 245);
		}
	}

};


class CStrLibList:public CXTPReportControl
{
public:
	CStrLibList()
	{
		_owner=NULL;
		_curgrp=StringID_Invalid;
		_iCategory=STRLIB_CATEGORY_DEFAULT;
	}

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);

	void Refresh(LPCTSTR lpszFilter = NULL);

	StringID GetStringID(int iSel);
	StringID GetStringID(CXTPReportRecord *record);
	DWORD GetCount();

	void SetCategory(DWORD iCategory)	{		_iCategory=iCategory;	}

	void SetCurGrp(StringID idGrp)	{		_curgrp=idGrp;	}
	StringID GetCurGrp()	{		return _curgrp;	}

	void SetCurSel(StringID idSel);
	StringID GetCurSel();

	DWORD IsSel();

	void OnGroupCheck(CXTPReportRecord *record);
	void OnSortChange();
	void OnChange(const char *str);
	void OnNew(const char *str);
	void OnDelete();

protected:

	void _AddRecord(StringID id);

	void _BuildGrpLookup(std::multimap<StringID,StringID>&lookup);

	void _RefreshGroups(StringID id);

	BOOL _VerifyGrpUnique(const char *str,StringID idGrp);//返回是否Unique

	CXTPReportRecord*_GetCurSel();

	CStrLibDlg *_owner;
	StringID _curgrp;//表示当前显示的是哪个group中的字符串,如果为StringID_Invalid,则显示全部的字符串
	DWORD _iCategory;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnGroupRange(UINT idCmd);

	friend class CStrLibDlg;
	friend class CReportItemGroups;

};



// CStrLibDlg 对话框
class ISscSystem;
class GuiLib_Api CStrLibDlg : public CXTPDialog
{
// 构造
public:
	CStrLibDlg(CWnd* pParent = NULL);	// 标准构造函数

	BOOL Create(CWnd *pParent);

	//Call these functions BEFORE DoModal()
	void SetSscSystem(ISscSystem *pSS)	{		_pSS=pSS;	}
	void BindSel(StringID idSel)	{		_idSel=idSel;	}
	void SetCurGrp(StringID idGrp)	{		_list.SetCurGrp(idGrp);	}
	void SetCategory(DWORD iCategory)	{		_iCategory=iCategory;	}
	//

	StringID GetSel()	{		return _idSel;	}
	void UpdateEdit();
	void UpdateBtns();
	void UpdateGrpCombo();

	void SelectEdit();

// 对话框数据
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	virtual void OnOK();
	virtual void OnCancel();


// 实现
protected:
	void _Finish();

	BOOL _bReadOnly;

	CStrLibList _list;
	ISscSystem *_pSS;

	DWORD _iCategory;

	StringID _idSel;

	StringID _idEdit;

	UINT _idTimer;

	CString m_strSearchFilter;

	BOOL _bEditModified;

	void OnReportCheckItem(NMHDR*  pNotifyStruct, LRESULT* );
	void OnReportSortChange(NMHDR*  pNotifyStruct, LRESULT* );

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEnChangeEdit();
	afx_msg void OnEnChangeSearch();
	afx_msg void OnEmpty();
	afx_msg void OnChange();
	afx_msg void OnNew();
	afx_msg void OnDelete();
	afx_msg void OnDblClick(NMHDR * pNotifyStruct, LRESULT * result);
};


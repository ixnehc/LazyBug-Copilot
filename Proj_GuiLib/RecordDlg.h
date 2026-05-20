
#pragma once
#include "GuiLib.h"

#include "records/records.h"

#include "gds/GDefines.h"



class GuiLib_Api CXTPReportRecordItemInt : public CXTPReportRecordItem
{
	DECLARE_SERIAL(CXTPReportRecordItemInt)
public:
	CXTPReportRecordItemInt(int value= 0);
	CString GetCaption(CXTPReportColumn* pColumn);

	int GetValue();

	void SetValue(int value)
	{
		m_value=value;
	}

	virtual void OnEditChanged(XTP_REPORTRECORDITEM_ARGS* pItemArgs, LPCTSTR szText);

	int Compare(CXTPReportColumn* pColumn, CXTPReportRecordItem* pItem);

	virtual void DoPropExchange(CXTPPropExchange* pPX);
protected:
	int m_value;    // Cell value.
};

class GuiLib_Api CXTPReportRecordItemFloat : public CXTPReportRecordItem
{
	DECLARE_SERIAL(CXTPReportRecordItemFloat)
public:
	CXTPReportRecordItemFloat(float value= 0);
	CString GetCaption(CXTPReportColumn* pColumn);

	int GetValue();

	void SetValue(float value)
	{
		m_value=value;
	}

	virtual void OnEditChanged(XTP_REPORTRECORDITEM_ARGS* pItemArgs, LPCTSTR szText);

	int Compare(CXTPReportColumn* pColumn, CXTPReportRecordItem* pItem);

	virtual void DoPropExchange(CXTPPropExchange* pPX);
protected:
	float m_value;    // Cell value.
};


GuiLib_Api GVarType ParseRecordValue(GElemBase *elem,CRecord *record,int &vInt,float &vFloat,std::string &s);
GuiLib_Api BOOL GetRecordName(CRecord* rec, CString& sName);
GuiLib_Api void InitFilterCombo(CComboBox& comboFilter, CRecords* records);
GuiLib_Api BOOL CullRecordNameKey(const char* nm, std::string& key);


class CRecordSelListRecord:public CXTPReportRecord
{
public:
	CRecordSelListRecord()
	{
		_id=RecordID_Invalid;
		_ver=0;
	}
	void UpdateItem(DWORD idx,const char *s);
	void UpdateItem(DWORD idx,int v);
	void UpdateItem(DWORD idx,float v);
	void GetItemMetrics (XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs, XTP_REPORTRECORDITEM_METRICS* pItemMetrics);
	RecordID GetID()	{		return _id;	}
	DWORD GetVer()	{		return _ver;	}
	void SetID(RecordID id)	{		_id=id;	}
	void SetVer(DWORD ver)	{		_ver=ver;	}
protected:

	RecordID _id;
	DWORD _ver;

};




class CRecordSelList:public CXTPReportControl
{
public:
	CRecordSelList()
	{
		_columnSortIdx=NULL;
		_records=NULL;
	}

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);

	void SetRecords(CRecords *records)	{		_records=records;	}

	void Refresh();

	void Sel(RecordID idSel);

	RecordID GetSingleSel();


protected:

	void _AddColumns(CRecords *records);

	CXTPReportColumn* _columnSortIdx;


	void _UpdateRecord(CRecordSelListRecord* r,CRecord *record);
	void _UpdateRecordSortIdx(CRecordSelListRecord* r,CRecord *record);//更新序号

	BOOL _IsFilteredOut(CRecord* record);

	void _OnClick(CPoint point,BOOL bDblClk);

	CRecords *_records;
	std::string _s;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	friend class CRecordDlg;
};


// CRecordDlg 对话框
class GuiLib_Api CRecordDlg : public CXTPDialog
{
// 构造
public:
	CRecordDlg(CWnd* pParent = NULL);	// 标准构造函数

	BOOL Create(CWnd *pParent);

	void SetSel(RecordID idSel)	{		_idSel=idSel;	}

	void SetRecords(CRecords *records)	{		_list.SetRecords(records);	}

	RecordID GetSel()	{		return _idSel;	}

	void ShowSelNone()	{		_bShowSelNone=TRUE;	}

	virtual void OnOK();
	virtual void OnCancel();

// 对话框数据
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:

	CRecordSelList _list;

	CComboBox _comboFilter; 
	CString _filter;        
	void _InitFilterCombo();
	void _UpdateFilterCombo();
	afx_msg void OnCbnSelchangeFilterCombo();
	afx_msg void OnCbnEditchangeFilterCombo();

	RecordID _idSel;


	BOOL _bShowSelNone;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSelnone();

	friend class CRecordSelList;
};




#pragma once


class CCustomItemCheckBox;

class CInplaceCheckBox : public CButton
{
public:
	afx_msg LRESULT OnCheck(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT /*nCtlColor*/);

	DECLARE_MESSAGE_MAP()
protected:
	CCustomItemCheckBox* m_pItem;
	COLORREF m_clrBack;
	CBrush m_brBack;


	friend class CCustomItemCheckBox;
};

class CCustomItemCheckBox : public CXTPPropertyGridItem
{
protected:

public:
	CCustomItemCheckBox(CString strCaption);

	BOOL GetBool();
	void SetBool(BOOL bValue);

protected:
	virtual void OnDeselect();
	virtual void OnSelect();
	virtual CRect GetValueRect();
	virtual BOOL OnDrawItemValue(CDC& dc, CRect rcValue);

protected:
	CInplaceCheckBox m_wndCheckBox;
	BOOL m_bValue;

	friend class CInplaceCheckBox;
};

class CRichGrid_BoolItem:public CXTPPropertyGridItemBool
{
public:
	CRichGrid_BoolItem(CString strCaption):CXTPPropertyGridItemBool(strCaption)
	{
		_b=NULL;
		_b2=NULL;
		_flag=NULL;
		_mask=0;
	}
	void Bind(BOOL*b,const char *constraint);
	void Bind(BYTE*b,const char *constraint);
	void Bind(DWORD *flag,DWORD mask,const char *constraint);


	virtual void OnValueChanged(CString strValue);

protected:
	void _ParseConstraint(const char *contraint);

	void _ApplyCaptioonHides(BOOL b);

	std::vector<std::string>_hides;

	BOOL *_b;
	BYTE *_b2;
	DWORD *_flag;
	DWORD _mask;

};






#pragma once
#include "GuiLib.h"

#include "SlideSpin.h"

class CSpinEdit;
//A SpinEdit internally used edit
class CSEInplaceEdit : public CXTFlatEdit
{
public:

	CSEInplaceEdit();

	virtual ~CSEInplaceEdit();

private:

	CSpinEdit *m_pSpinEdit;
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnEnKillfocus();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
private:

	DECLARE_MESSAGE_MAP()

	friend class CSpinEdit;
};

class CSpinEdit:public CWnd
{
public:
	CSpinEdit()
	{
		m_bFloatMode=FALSE;
	}

	BOOL Create(CWnd *pParent,CRect &rc,UINT id);

	void EnableFloatMode(BOOL bEnable)	{		m_bFloatMode=bEnable;	}

	void SetRange(SlideSpinValue low,SlideSpinValue high);
	void GetRange(SlideSpinValue &low,SlideSpinValue &high);
	void SetValue(SlideSpinValue v);
	SlideSpinValue GetValue();
	void SetSpinSpeed(float speed);

	virtual void OnBeginValueChange(){}
	virtual void OnEndValueChange(){}

	virtual void OnValueChange(SlideSpinValue v){}


protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnSlideSpinNotify(WPARAM wParam,LPARAM lParam);

	void OnEditChange();//called from the edit when it's content is changed
	void ValueToString(SlideSpinValue v,CString &str);

	BOOL m_bFloatMode;
	CSlideSpin _spin;
	CSEInplaceEdit _edit;

	static CFont _fontEdit;

	friend class CSEInplaceEdit;

};


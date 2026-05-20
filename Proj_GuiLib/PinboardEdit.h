#pragma once

#include "PinWndBase.h"

class  GuiLib_Api CPinboardEdit : public CPinboardWndBase<CEdit>
{
public:
	CPinboardEdit(void);
	~CPinboardEdit(void);

	virtual float GetFVal();
	virtual void OnSetValue(int value,ChangeState state);
	virtual void OnSetValue(float value,ChangeState state);
	virtual void OnEnable(BOOL bOnoff);
	
protected:
	void _validate();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
private:
	CString _strContent;
	BOOL _bLockSet;  //在获得焦点时不更新
};

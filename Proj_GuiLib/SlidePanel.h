
#pragma once
#include "GuiLib.h"

class GuiLib_Api CSlidePanel : public CDialog
{
public:
	CSlidePanel()
	{
		_ptOff.x=0;
		_ptOff.y=0;
		_bDrag=FALSE;
	}
	virtual BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL);
protected:
	BOOL _bDrag;
	CPoint _ptDrag;
	CPoint _ptStart;
	CSize _sz;
	CPoint _ptOff;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
	virtual void OnCancel(){}

	friend class CSlideTab;
};

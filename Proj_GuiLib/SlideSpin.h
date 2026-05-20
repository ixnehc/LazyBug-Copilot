
#pragma once
#include "GuiLib.h"

typedef double SlideSpinValue;



class CSlideSpin:public CSpinButtonCtrl
{
public:
	CSlideSpin()
	{
		m_bCapture=FALSE;
		m_bSliding=FALSE;
		m_low=m_high=0;
		m_v=0;
		m_speed=1.0f;
	}


	void GetRange(SlideSpinValue &low,SlideSpinValue &high);//inclusive
	void SetRange(SlideSpinValue low,SlideSpinValue high);//inclusive
	void SetValue(SlideSpinValue f);
	SlideSpinValue GetValue()	{		return m_v;	}

	void SetSlideSpeed(SlideSpinValue speed);//default is 1.0f

	virtual void OnBeginValueChange()	{}
	virtual void OnEndValueChange()	{}
	virtual void OnValueChange(SlideSpinValue vNew)	{}

protected:
	SlideSpinValue m_low,m_high;
	SlideSpinValue m_v;
	BOOL m_bCapture;
	BOOL m_bSliding;
	CPoint m_ptStart;
	SlideSpinValue m_vStart;
	SlideSpinValue m_speed;

	void Notify(int code,SlideSpinValue v);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDeltapos(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);

};


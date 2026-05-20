#pragma once

#include "PinWndBase.h"

class CPinSpinner :public CPinWndBase<CSpinButtonCtrl>
{
public:
	CPinSpinner(void);
	~CPinSpinner(void);
//
	virtual float GetFVal();

	virtual void OnSetValue(float value,ChangeState state);
	virtual void OnSetLimits(float minValue,float maxValue);
	virtual void OnEnable(BOOL bOnoff);

	virtual void SetAccel(float value,bool bAuto);	
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDeltaPos(NMHDR * pNMHR,LRESULT * pResult);
	
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);

protected:
	float _range_scale;

	BOOL m_bCapture;
	BOOL m_bSliding;
	CPoint m_ptStart;
	float m_vStart;
	float m_speed;
	
	float _range_min,_range_max;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

#pragma once

#include "PinWndBase.h"

class GuiLib_Api CPinSlider :public CPinWndBase<CSliderCtrl>
{
public:
	CPinSlider(void);
	~CPinSlider(void);

	// override
	virtual float GetFVal();

	virtual void OnSetValue(float value,ChangeState state);
	virtual void OnEnable(BOOL bOnoff);
	virtual void OnSetLimits(float minValue,float maxValue);

	void SetInstant(BOOL bInstant){m_bInstant = bInstant;}

	//
protected:

	DECLARE_MESSAGE_MAP()
	
	afx_msg void OnCaptureChanged(CWnd* pWnd);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	
protected:
	float _range_scale;
	BOOL  m_bInstant;	
	int m_pos;
	BOOL m_bOp;

	float _range_min,_range_max;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};



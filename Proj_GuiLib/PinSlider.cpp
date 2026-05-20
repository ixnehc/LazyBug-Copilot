
#include "stdh.h"

#include "PinSlider.h"

BEGIN_MESSAGE_MAP(CPinSlider,CSliderCtrl)
	ON_WM_CAPTURECHANGED()
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

CPinSlider::CPinSlider(void)
{
	m_bInstant = TRUE;
	m_bOp = FALSE;
}

CPinSlider::~CPinSlider(void)
{
}
void CPinSlider::OnCaptureChanged(CWnd* pWnd)
{
	if(pWnd!=this){
		m_bOp = FALSE;
		ReleaseCapture();
		NotifyEndChange();
	}
	CSliderCtrl::OnCaptureChanged(pWnd);
}
float CPinSlider::GetFVal()
{
	int pos = GetPos();
	float value = float(pos)*_range_scale + _range_min;
	value = i_math::clamp_f(value,_range_min,_range_max);
	return value;
}
void CPinSlider::OnSetValue(float value,ChangeState state)
{	
	float v = value - _range_min;
	int pos = int(v/_range_scale);
	SetPos(pos);
}

void CPinSlider::OnSetLimits(float minValue,float maxValue)
{	
	SetRange(0,10000);
	_range_min = minValue;
	_range_max = maxValue;
	_range_scale = (maxValue - minValue)/10000.0f;
}
void CPinSlider::OnEnable(BOOL bOnoff)
{
	EnableWindow(bOnoff);
}
//////////////////////////////////////////////////////////////////////////

int CPinSlider::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSliderCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}
void CPinSlider::OnMouseMove(UINT nFlags, CPoint point)
{	
	CSliderCtrl::OnMouseMove(nFlags, point);

	int pos = GetPos();

	if(!m_bOp) 
		return;

	m_pos = pos;
	NotifyOnChange();
}

void CPinSlider::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bOp = TRUE;
	SetCapture();
	CSliderCtrl::OnLButtonDown(nFlags, point);
}

void CPinSlider::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bOp = FALSE;
	ReleaseCapture();
	CSliderCtrl::OnLButtonUp(nFlags, point);
}

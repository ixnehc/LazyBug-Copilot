
#include "stdh.h"

#include "PinSpinner.h"

CPinSpinner::CPinSpinner(void)
{
	m_speed = 0.01f;
	m_bCapture = FALSE;
}

CPinSpinner::~CPinSpinner(void)
{
}
//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPinSpinner,CSpinButtonCtrl)
	ON_NOTIFY_REFLECT(UDN_DELTAPOS,OnDeltaPos)
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()

void CPinSpinner::OnDeltaPos(NMHDR * pNMHR,LRESULT * pResult)
{
	NotifyOnChange();
}
void CPinSpinner::OnCaptureChanged(CWnd *pWnd)
{
	if (!m_bCapture)
		return;
	
	if(pWnd!=this){
		m_bSliding=FALSE;
		m_bCapture = FALSE;
		NotifyEndChange();
		ReleaseCapture();
	}
}
float CPinSpinner::GetFVal()
{
	int pos = GetPos() - 65536;

	float value = float(pos)*_range_scale + _range_min;
	value = i_math::clamp_f(value,_range_min,_range_max);

	return value;
}
void CPinSpinner::OnSetValue(float value,ChangeState state)
{
	int pos = int((value-_range_min)/_range_scale);
	SetPos(pos);
}
void CPinSpinner::OnSetLimits(float minValue,float maxValue)
{
	SetRange(0,10000);
	_range_min = minValue;
	_range_max = maxValue;
	_range_scale = (maxValue - minValue)/10000.0f;
	SetAccel(_range_scale*100.0f,true);
}
void CPinSpinner::SetAccel(float value,bool bAuto)
{
	float step = (_range_max - _range_min)/10000.0f;
	if(step==0.0f)
		return;

	int tick = int(value/step);
	
	UDACCEL accels[4];
	accels[0].nSec = 0;
	accels[0].nInc = tick;
	accels[1].nSec = 2;
	accels[1].nInc = 5*tick;
	accels[2].nSec = 4;
	accels[2].nInc = 10*tick;
	accels[3].nSec = 8;
	accels[3].nInc = 40*tick;
	
	if(bAuto)	
		CSpinButtonCtrl::SetAccel(4,accels);
	else
		CSpinButtonCtrl::SetAccel(1,accels);
	
}
void CPinSpinner::OnEnable(BOOL bOnoff)
{
	EnableWindow(bOnoff);
}
//////////////////////////////////////////////////////////////////////////
int CPinSpinner::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSpinButtonCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}
void CPinSpinner::OnMouseMove(UINT nFlags, CPoint point)
{
	CSpinButtonCtrl::OnMouseMove(nFlags,point);
	
	if (!m_bCapture)
		return;

	if (!m_bSliding)
	{
		m_bSliding=TRUE;
		m_vStart=GetFVal();
	}

	ClientToScreen(&point);

	const int tip=10;
	int hScreen;
	hScreen=GetSystemMetrics(SM_CYFULLSCREEN);
	if (point.y>hScreen-10)
	{
		m_vStart=GetFVal();
		point.y=tip+1;
		m_ptStart=point;
		SetCursorPos(point.x,point.y);
	}
	if (point.y<tip)
	{
		m_vStart=GetFVal();
		point.y=hScreen-tip-1;
		m_ptStart=point;
		SetCursorPos(point.x,point.y);
	}

	float v;
	v=m_vStart+(float)(m_speed*(float)(m_ptStart.y-point.y));
	
	OnSetValue(v,OnChange);
	
	NotifyOnChange();
}
void CPinSpinner::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	m_bSliding=FALSE;
	m_bCapture=TRUE;
	m_ptStart=point;
	ClientToScreen(&m_ptStart);
	
	SetCapture();

	NotifyBeginChange();

	CSpinButtonCtrl::OnLButtonDown(nFlags,point);
}
void CPinSpinner::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bCapture = FALSE;
	
	NotifyEndChange();

	ReleaseCapture();
	CSpinButtonCtrl::OnLButtonUp(nFlags,point);
}

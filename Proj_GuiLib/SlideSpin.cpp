/********************************************************************
	created:	2006/9/9   13:59
	filename: 	d:\IxEngine\Proj_GuiLib\SlideSpin.cpp
	author:		cxi
	
	purpose:	a slidable spin control 
*********************************************************************/
#include "stdh.h"
#include "SlideSpin.h"

#include "WMGuiLib.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FLOAT_CONVERT_RATE (100000.0)


////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSlideSpin, CSpinButtonCtrl)
	ON_NOTIFY_REFLECT(UDN_DELTAPOS, OnDeltapos)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()

void CSlideSpin::Notify(int code,SlideSpinValue v)
{
	CWnd *pParent;
	pParent=GetParent();
	if (!pParent)
		return;

	if (code==SSN_Changing)
		pParent->SendMessage(GLM_SlideSpin_Notify,(WPARAM)code,(LPARAM)&v);
	else
		pParent->SendMessage(GLM_SlideSpin_Notify,(WPARAM)code,(LPARAM)0);
}


void CSlideSpin::SetRange(SlideSpinValue low,SlideSpinValue high)
{
	CSpinButtonCtrl::SetRange32(0,1);
	m_low=low;
	m_high=high;
}

void CSlideSpin::GetRange(SlideSpinValue &low,SlideSpinValue &high)
{
	low=m_low;
	high=m_high;
}

void CSlideSpin::SetValue(SlideSpinValue v)
{
	if (v<m_low)		v=m_low;
	if (v>m_high)		v=m_high;

	m_v=v;
}

//default is 1.0f
void CSlideSpin::SetSlideSpeed(double speed)
{
	m_speed=speed;
}

void CSlideSpin::OnDeltapos(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	*pResult = 1;

	if (m_bSliding||(!m_bCapture))
		return;

	SlideSpinValue v;
	v=m_v+m_speed*(SlideSpinValue)pNMUpDown->iDelta;
	if (v<m_low)		v=m_low;
	if (v>m_high)		v=m_high;

	if (v==m_v)
		return;//no change

	m_v=v;

	Notify(SSN_Changing,m_v);
	OnValueChange(m_v);
}


void CSlideSpin::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	m_bSliding=FALSE;
	m_bCapture=TRUE;
	m_ptStart=point;
	ClientToScreen(&m_ptStart);
	Notify(SSN_BeginChange,0);
	OnBeginValueChange();
	CSpinButtonCtrl::OnLButtonDown(nFlags,point);
}
void CSlideSpin::OnLButtonUp(UINT nFlags, CPoint point)
{
// 	m_bSliding=FALSE;
// 	m_bCapture=FALSE;

	CSpinButtonCtrl::OnLButtonUp(nFlags,point);
// 	OnEndValueChange();
// 	Notify(SSN_EndChange,0);
}

void CSlideSpin::OnMouseMove(UINT nFlags, CPoint point)
{
	CSpinButtonCtrl::OnMouseMove(nFlags,point);
	if (!m_bCapture)
		return;
	if (!m_bSliding)
	{
		m_bSliding=TRUE;
		m_vStart=m_v;
	}

	ClientToScreen(&point);

	const int tip=10;
	int hScreen;
	hScreen=GetSystemMetrics(SM_CYFULLSCREEN);
	if (point.y>hScreen-10)
	{
		m_vStart=m_v;
		point.y=tip+1;
		m_ptStart=point;
		SetCursorPos(point.x,point.y);
	}
	if (point.y<tip)
	{
		m_vStart=m_v;
		point.y=hScreen-tip-1;
		m_ptStart=point;
		SetCursorPos(point.x,point.y);
	}


	SlideSpinValue v;
	v=m_vStart+(SlideSpinValue)(m_speed*(SlideSpinValue)(m_ptStart.y-point.y));

	if (v<m_low)		v=m_low;
	if (v>m_high)		v=m_high;

	if(v!=m_v)
	{
		m_v=v;
		Notify(SSN_Changing,m_v);
		OnValueChange(m_v);
	}
}

void CSlideSpin::OnCaptureChanged(CWnd *pWnd)
{
	if (!m_bCapture)
		return;
	m_bSliding=FALSE;
	m_bCapture=FALSE;
	Notify(SSN_EndChange,0);
	OnEndValueChange();

}

void CSlideSpin::OnLButtonDblClk(UINT nFlags, CPoint point)
{
}

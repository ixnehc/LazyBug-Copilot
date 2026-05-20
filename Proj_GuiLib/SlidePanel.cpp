/********************************************************************
	created:	2006/9/4   18:17
	filename: 	e:\IxEngine\Proj_GuiLib\SlidePanel.cpp
	author:		cxi
	
	purpose:	a panel that could be slide within a SlideTab
*********************************************************************/
#include "stdh.h"
#include ".\SlidePanel.h"
#include "SlideTab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



BEGIN_MESSAGE_MAP(CSlidePanel, CDialog)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CAPTURECHANGED()
	ON_WM_MOUSEWHEEL()
	ON_WM_NCLBUTTONDOWN()
END_MESSAGE_MAP()


BOOL CSlidePanel::Create(UINT nIDTemplate, CWnd* pParentWnd)
{
	if (FALSE==CDialog::Create(nIDTemplate,pParentWnd))
		return FALSE;

	CRect rc;
	GetWindowRect(&rc);
	_sz.cx=rc.Width();
	_sz.cy=rc.Height();

	return TRUE;
}

void CSlidePanel::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	_bDrag=TRUE;
	_ptDrag=point;
	ClientToScreen(&_ptDrag);
	_ptStart=_ptOff;
	SetCapture();
}

void CSlidePanel::OnMouseMove(UINT nFlags, CPoint point)
{
	if (_bDrag)
	{
		ClientToScreen(&point);
		CPoint ptOff;
		ptOff=_ptOff;
		ptOff.x=_ptStart.x+point.x-_ptDrag.x;
		if (ptOff!=_ptOff)
		{
			_ptOff=ptOff;
			((CSlideTab*)GetParent())->Reposition();
		}
	}
}
void CSlidePanel::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
}

void CSlidePanel::OnCaptureChanged(CWnd *pWnd)
{
	// TODO: Add your message handler code here
	_bDrag=FALSE;

	CDialog::OnCaptureChanged(pWnd);
}

BOOL CSlidePanel::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default

	_ptOff.x+=zDelta;
	((CSlideTab*)GetParent())->Reposition();

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CSlidePanel::OnOK()
{
}
BOOL CSlidePanel::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message == WM_KEYDOWN )&&((pMsg->wParam == 'Z')||(pMsg->wParam == 'Y')))
	{
		BOOL bEditFocus=FALSE;
		CWnd *pWnd=GetFocus();
		if (pWnd)
		{
			TCHAR szTemp[32];
			::GetClassName(pWnd->GetSafeHwnd(), szTemp, ARRAY_SIZE(szTemp));
			if (_tcscmp(szTemp,_T("Edit"))==0)
				bEditFocus=TRUE;
		}
		if (!bEditFocus)
			return FALSE;
	}

	if (pMsg->message==WM_MOUSEWHEEL)
		return FALSE;

	return CDialog::PreTranslateMessage(pMsg);

}

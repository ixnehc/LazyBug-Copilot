/********************************************************************
	created:	2008/07/21
	created:	21:7:2008   9:56
	filename: 	e:\IxEngine\Proj_GuiLib\SptViewPanel.cpp
	file path:	e:\IxEngine\Proj_GuiLib
	file base:	SptViewPanel
	file ext:	cpp
	author:		star
	purpose:	view spt model
*********************************************************************/

#include "stdh.h"

#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/ISpeedTree.h"

#include ".\SptViewPanel.h"

#include "WorldSystem/IStdRes.h"

#include "WorldSystem/IWorldSystem.h"

CSptViewPanel::CSptViewPanel()
{
	_pSpt = NULL;
	_sptDrawer = NULL;
	_cameraState = Camera_Still;
}
CSptViewPanel::~CSptViewPanel()
{
}
BEGIN_MESSAGE_MAP(CSptViewPanel,CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()

void CSptViewPanel::OnInit3D()
{
	_sptDrawer = g_ssGuiLib.pWS->CreateSptDrawer();
}
void CSptViewPanel::OnClear3D()
{
	SAFE_RELEASE(_sptDrawer);
}
void CSptViewPanel::SetSpt(ISpt * pSpt)
{
	Invalidate();
	_pSpt = pSpt;
}
void CSptViewPanel::OnDraw(IRenderPort *rp)
{
	ICamera * camera = rp->GetCamera();
	_ccler.UpdateCamera(camera);
	if(_pSpt){
		_pSpt->Touch();
		_sptDrawer->SetRP(rp);
		_sptDrawer->Draw(_pSpt,NULL);
	}
}

//////////////////////////////////////////////////////////////////////////
void CSptViewPanel::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	if(_cameraState==Camera_Still)
	{
		_ccler.DragBegin(point.x,point.y);
		_cameraState = Camera_Move;
		SetCapture();
	}
	CWnd::OnLButtonDown(nFlags,point);
}
void CSptViewPanel::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(_cameraState == Camera_Move)
		ReleaseCapture();
	CWnd::OnLButtonUp(nFlags,point);
}
void CSptViewPanel::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	if(_cameraState==Camera_Still)
	{
		_ccler.DragBegin(point.x,point.y);
		_cameraState = Camera_Rotate;
		SetCapture();
	}
	CWnd::OnRButtonDown(nFlags,point);
}
void CSptViewPanel::OnRButtonUp(UINT nFlags, CPoint point)
{
	if(_cameraState==Camera_Rotate)
		ReleaseCapture();
	CWnd::OnRButtonUp(nFlags,point);
}
void CSptViewPanel::OnMouseMove(UINT nFlags, CPoint point)
{
	if(_cameraState==Camera_Move)
	{
		int x,y;
		
		x = point.x;
		y = point.y;

		_ccler.DragMove(x,y);
		
		Invalidate();
	}
	else if(_cameraState == Camera_Rotate)
	{
		int x,y;
		
		x = point.x;
		y = point.y;

		_ccler.DragRotate(x,y);

		Invalidate();
	}

	CWnd::OnMouseMove(nFlags,point);
}

BOOL CSptViewPanel::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect rc;
	GetClientRect(rc);
	ScreenToClient(&pt);

	if(rc.PtInRect(pt))
	{
		_ccler.ZoomIn(-zDelta);
		Invalidate();
		return TRUE;
	}

	return CWnd::OnMouseWheel(nFlags,zDelta,pt);
}
void CSptViewPanel::OnCaptureChanged(CWnd* pWnd)
{
	_cameraState = Camera_Still;
	CWnd::OnCaptureChanged(pWnd);
}








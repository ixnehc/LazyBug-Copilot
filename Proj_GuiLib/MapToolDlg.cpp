#include "stdh.h"
#include ".\maptooldlg.h"
#include "resource.h"
#include "graphicsgraph.h"

CMapToolDlg::CMapToolDlg(CWnd * pParent/*= NULL*/)
:CDialog(IDD_DIALOG_MAPTOOLS,pParent),_viewDraw(this)
{
	_mgrGui = NULL;
	_bInit = FALSE;
}
CMapToolDlg::~CMapToolDlg(void)
{
}
//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CMapToolDlg,CDialog)
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CMapToolDlg::Create(CWnd * pParent,CGuiMgr * mgr)
{	
	if(!mgr||FALSE == CDialog::Create(IDD_DIALOG_MAPTOOLS,pParent))
		return FALSE;

	_mgrGui = mgr;

	CRect rc;
	GetClientRect(rc);

	rc.right -= 180;
	_viewDraw.Create(NULL,NULL,WS_VISIBLE|WS_CHILD,rc,this,0);
	
	return TRUE;
}
BOOL CMapToolDlg::OnInitDialog()
{
	if(FALSE == CDialog::OnInitDialog())
		return FALSE;

	CWnd * pWnd = GetDlgItem(IDC_TOOLAREAS);
	_actorMap.Create(pWnd);

	return TRUE;
}
void CMapToolDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
	if(_bInit)
	{
		CGuiView * view = (CGuiView *)_mgrGui->FindView("overallmap");
		if(view)
			view->Draw();
	}
}

LRESULT CMapToolDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(_mgrGui)
	{	
		CGuiView * view = (CGuiView *)_mgrGui->FindView("overallmap");
		if(view&&_viewDraw.GetSafeHwnd())
		{
			CRect rc;
			_viewDraw.GetClientRect(&rc);
			view->SetWnd(&_viewDraw,i_math::recti(rc.left,rc.top,rc.right,rc.bottom));
			CPoint point;
			GetCursorPos(&point);
			ScreenToClient(&point);
			LRESULT ret = 0;
			if(TRUE == view->RespondMsg(point,message,wParam,lParam,ret)) 
				return ret;
		}
	}
	return CDialog::WindowProc(message,wParam,lParam);
}
void CMapToolDlg::_Init()
{
	_mgrGui->RegisterActor(&_actorMap);
	_actorMap.OnEnterActivity();
	_bInit = TRUE;
}
//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CMapToolDlg::CMapView, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

LRESULT CMapToolDlg::CMapView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CGuiView * view = (CGuiView *)_owner->_mgrGui->FindView("overallmap");
	if(view)
	{
		CRect rc;
		GetClientRect(&rc);
		view->SetWnd(this,i_math::recti(rc.left,rc.top,rc.right,rc.bottom));
		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);
		LRESULT ret = 0;

		if(!_owner->_bInit)
			_owner->_Init();

		if(TRUE == view->RespondMsg(point,message,wParam,lParam,ret)) 
			return ret;
	}

	return CWnd::WindowProc(message,wParam,lParam);
}
void CMapToolDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType,cx,cy);

	CRect rc;
	GetClientRect(rc);
	rc.right = cx - 182;
	
	if(_viewDraw.GetSafeHwnd())
		_viewDraw.MoveWindow(rc);

	rc.left = rc.right+2;
	rc.right = cx;
	
	CWnd * pWnd = GetDlgItem(IDC_TOOLAREAS);
    if(pWnd->GetSafeHwnd())
		pWnd->MoveWindow(rc);
}





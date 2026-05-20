/********************************************************************
created:	2008/02/26
created:	26:2:2008   9:38
filename: 	e:\IxEngine\Proj_GuiLib\SlideContainer.cpp
file path:	e:\IxEngine\Proj_GuiLib
file base:	ScrollWnd
file ext:	cpp
author:		star
purpose:	a scrool window container ,supports scrooling function.
*********************************************************************/

#include "stdh.h"
#include ".\slidecontainer.h"
#include "assert.h"
BEGIN_MESSAGE_MAP(CSlideContainer,CWnd)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_CAPTURECHANGED()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CSlideContainer::SetControl(CWnd * pControl,CRect &rc)
{
#pragma warning(disable:4244)
	if(!pControl)
		return FALSE;
	assert(pControl->GetSafeHwnd());
	pControl->SetParent(this);
	
	CRect rce;
	GetClientRect(rce);

	if(_flag==Scroll_Vertical){
		rc.right = rce.right-ScrollBar_Width;
	}

	else if(_flag==Scroll_Horizon){
		rc.bottom = rce.bottom-ScrollBar_Width;		
	}

	_pControl=pControl;
	_controlInfo.rc=rc;
	_controlInfo.wndProc=GetWindowLongPtr(pControl->GetSafeHwnd(),GWLP_WNDPROC);
	SetWindowLongPtr(pControl->GetSafeHwnd(),GWLP_WNDPROC,(LONG_PTR)(MyWndProc));
	SetWindowLongPtr(pControl->GetSafeHwnd(),GWLP_USERDATA,(LONG_PTR)(this));

	_flagSize=TRUE;
	_pControl->MoveWindow(&_controlInfo.rc);
	_pControl->ShowWindow(SW_SHOW);
	
	return TRUE;
}

LRESULT CALLBACK MyWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	CSlideContainer  * theClass = (CSlideContainer*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	
	if(!theClass)
		return 0;
	CSlideContainer::ControlInfo * controlInfo=&(theClass->_controlInfo);
	FUN_PTR oldProc=(FUN_PTR)(controlInfo->wndProc);

	switch(message)
	{
	case WM_MOUSEMOVE:
		{
			SetCursor(theClass->_cursor);
			UINT flag=(UINT)wParam;
			if(flag&MK_LBUTTON)
			{
				POINTS point=MAKEPOINTS(lParam);
			 	theClass->_point.x=point.x;
				theClass->_point.y=point.y;
				theClass->_state=CSlideContainer::State_Scrolling;
				theClass->_OnScrolling();
			}
			break;
		}
	case WM_MOUSEWHEEL :
		{	
			short zDelta=GET_WHEEL_DELTA_WPARAM(wParam);
			theClass->_OnWheeling(zDelta);
			break;
		}
	case WM_LBUTTONDOWN:
		{
			//开始拖动时
			SetCapture(theClass->_pControl->GetSafeHwnd());
			SetFocus(theClass->_pControl->GetSafeHwnd());
			SetCursor(theClass->_cursor);
			break;
		}
	case WM_LBUTTONUP:
		{
			ReleaseCapture();
			SetCursor(theClass->_cursor);
			theClass->_state=CSlideContainer::State_EndScroll;
			theClass->_OnScrolling();
			break;
		}
	case WM_CAPTURECHANGED:
		{
			HWND chwnd=(HWND)lParam;
			if(chwnd!=hwnd)
			{	
				ReleaseCapture();
				theClass->_state=CSlideContainer::State_EndScroll;
				theClass->_OnScrolling();
			}	
			break;
		}
	case  WM_SIZE:
		{
			if(theClass->_flagSize)
			{
				theClass->_flagSize=FALSE;
				break;
			}
			theClass->_EraseBlank();
            break;
		}
	default:
		break;
	}
	return oldProc(hwnd,message,wParam,lParam);
}

void CSlideContainer::_OnWheeling(short zDelta)
{
	int offsetX=0,offsetY=0;

	_state=State_Scrolling;
	if(_flag==Scroll_Horizon)
		offsetX=zDelta/10;
	else if(_flag==Scroll_Vertical)
		offsetY=zDelta/10;

	_OnRecalculate(offsetX,offsetY);
}

BOOL CSlideContainer::Create(RECT & rc,CWnd *pParent,UINT templateId,DWORD flag,DWORD wsStyle)
{
	BOOL ret = CWnd::Create(NULL,NULL,wsStyle,rc,pParent,templateId);	
	if(!ret) 
		return FALSE;
	
	CWinApp *theDll=AfxGetApp();
	assert(theDll);
	_cursor=theDll->LoadCursor(IDC_CURSORMOVE);
	assert(_cursor);
	
	_flag=flag;
	EnableScrollBar(SB_VERT,FALSE);
	EnableScrollBar(SB_HORZ,FALSE);

	return TRUE;
}
void CSlideContainer::_OffsetRect(CRect &distSrc,const CRect &src,int offsetX,int offsetY)
{
	CRect rce ;
	GetClientRect(rce);

	distSrc=src;

	if(_flag==Scroll_Horizon)
	{
		if(src.Width()<=rce.Width())
			return;

		if(offsetX>0){ 
			assert(src.left<=0);
			offsetX = (offsetX > - src.left)?-src.left:offsetX;
		}
		else if(offsetX<0){
			assert(src.right>=rce.right);
			offsetX = (src.right+offsetX<rce.right)?(distSrc.right - src.right):offsetX;
		}

		distSrc.left  = src.left - offsetX;
		distSrc.right = src.right - offsetX;
	}
	else if(_flag==Scroll_Vertical)
	{
		if(src.Height()<=rce.Height())
			return;

		if(offsetY>0){ 
			assert(src.top<=0);
			offsetY = (offsetY > - src.top)?-src.top:offsetY;
		}
		else if(offsetY<0){
			assert(src.bottom>=rce.bottom);
			offsetY = (src.bottom+offsetY<rce.bottom)?(rce.bottom - src.bottom):offsetY;
		}

		distSrc.top = src.top + offsetY;
		distSrc.bottom = src.bottom + offsetY;
	}
}

void CSlideContainer::_OnRecalculate(int offsetX,int offsetY)
{
	if(offsetX==0&&offsetY==0) 
		return;

	if(_pControl){
		
		CRect  rc;
		_OffsetRect(rc,_controlInfo.rc,offsetX,offsetY);

		_controlInfo.rc = rc;
		_pControl->MoveWindow(&rc);
		_DrawScrollBar();
	}
}

void CSlideContainer::_EraseBlank()
{
	CRect rc,eraseArea;
	BOOL  bNeed=FALSE;
	GetClientRect(rc);
	eraseArea=rc;
	if(_flag==Scroll_Vertical)
	{
		if(_controlInfo.rc.Height()>rc.Height())
		{
			eraseArea.top=_controlInfo.rc.bottom;
			bNeed=TRUE;
		}
	}
	else if(_flag==Scroll_Horizon)
	{
		if(_controlInfo.rc.Width()>rc.Width())
		{
			eraseArea.left=_controlInfo.rc.left;
			bNeed=TRUE;
		}
	}
	
	if(bNeed)
	{
		CDC *pDC=GetDC();
		CBrush br;
		DWORD color= GetSysColor(COLOR_BTNFACE);
		br.CreateSolidBrush(color);
		pDC->FillRect(&eraseArea,&br);
		ReleaseDC(pDC);
	}
}

void CSlideContainer::OnPaint()
{
	CPaintDC dc(this);	
	
	CWnd::OnPaint();

	_DrawScrollBar();
}

void CSlideContainer::_OnScrolling()
{
	static BOOL bScroll=FALSE;
	static CPoint  oldPoint;
	switch(_state)
	{
	case State_Scrolling:
		{
			if(!bScroll)
			{
				oldPoint=_point;
				bScroll=TRUE;
				break;
			}
			else
			{
				int offsetX=_point.x-oldPoint.x;
				int offsetY=_point.y-oldPoint.y;
				_OnRecalculate(offsetX,offsetY);
			}
		}
	case State_EndScroll:
		{
			bScroll=FALSE;
		}
	}
}
void CSlideContainer::_DrawScrollBar()
{
	CDC * pDC=GetDC();
	CBitmap bitmap;
	CDC  memDC;
	memDC.CreateCompatibleDC(pDC);

	CRect drawArea,rc,imageArea,posArea;
	GetClientRect(rc);
	drawArea = rc;
	imageArea = rc;

	float ratioL,ratioP,length;

	if(_flag==Scroll_Vertical)
	{
		bitmap.CreateCompatibleBitmap(pDC,ScrollBar_Width,rc.Height());
		imageArea.right = ScrollBar_Width;
		drawArea.left=drawArea.right-ScrollBar_Width;	
		
		ratioL = (float)rc.Height()/(float)_controlInfo.rc.Height();
		length = ratioL*rc.Height();
		ratioP =(float)(-_controlInfo.rc.top)/(float)(_controlInfo.rc.Height()-rc.Height());

		posArea=imageArea;
		posArea.top =ratioP*(rc.Height()-length);
		posArea.bottom = posArea.top + length;

	}

	else if(_flag==Scroll_Horizon)
	{
		bitmap.CreateCompatibleBitmap(pDC,rc.Width(),ScrollBar_Width);
		imageArea.bottom=ScrollBar_Width;
		
		ratioL = (float)rc.Width()/(float)_controlInfo.rc.Width();
		length = ratioL*rc.Width();
		ratioP = (float)(- _controlInfo.rc.left)/(float)(_controlInfo.rc.Width()-rc.Width());

		posArea=imageArea;
		posArea.left = ratioP*(rc.Width()-length);
		posArea.right = posArea.left + length;

		drawArea.top=drawArea.bottom-ScrollBar_Width;
	}

	memDC.SelectObject(bitmap);

	memDC.Draw3dRect(&drawArea,RGB(100,100,100),RGB(100,100,100));

	CBrush brush;
	brush.CreateSolidBrush(RGB(200,200,200));
	posArea.left += 1;
	memDC.FillRect(&posArea,&brush);

	pDC->BitBlt(drawArea.left,drawArea.top,drawArea.Width(),drawArea.Height(),&memDC,0,0,SRCCOPY);
	ReleaseDC(pDC);	
	
	ValidateRect(drawArea);
}

void CSlideContainer::OnMouseMove(UINT nFlags, CPoint point)
{
	if(nFlags&MK_LBUTTON)
	{
		SetCapture();
		_point=point;
		_state=State_Scrolling;
		SetCursor(_cursor);
		_OnScrolling();
	}
	CWnd::OnMouseMove(nFlags, point);
}

void CSlideContainer::OnLButtonUp(UINT nFlags, CPoint point)
{
	_state=State_EndScroll;
	ReleaseCapture();
	_OnScrolling();
	CWnd::OnLButtonUp(nFlags, point);
}

BOOL CSlideContainer::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect rc;
	GetClientRect(rc);
	
	ScreenToClient(&pt);

	if(rc.PtInRect(pt))
	{
		_OnWheeling(zDelta);
		return TRUE;
	}
	return  CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CSlideContainer::OnCaptureChanged(CWnd *pWnd)
{
	if(pWnd&&pWnd->m_hWnd!=m_hWnd)
	{
		_state=State_EndScroll;
		ReleaseCapture();
		_OnScrolling();
	}
	CWnd::OnCaptureChanged(pWnd);
}

void CSlideContainer::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	
	if(!_pControl)
		return ;

	if(_flag==Scroll_Horizon)
	{
		_controlInfo.rc.bottom = _controlInfo.rc.top + cy;
		if(_controlInfo.rc.Height()!=cy) 
			_flagSize=TRUE;
	}
	else if(_flag==Scroll_Vertical)
	{
		_controlInfo.rc.right = _controlInfo.rc.left + cx - ScrollBar_Width;
		if(_controlInfo.rc.Width()!=cx)
			_flagSize=TRUE;
	}
	
	_pControl->MoveWindow(_controlInfo.rc);
}



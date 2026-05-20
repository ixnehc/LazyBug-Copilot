/************************************************************************/
/*
e:\IxEngine\Proj_GuiLib\AnimControlBar.h
author: star
purpose: write a animal control bar.
date: 2007-12-26
*/
/************************************************************************/

#include "stdh.h"
#include ".\animcontrolbar.h"
#include ".\WndBase.h"
#include "resource.h"
#define INDICATOR_WIDTH 15
#define INDICATOR_HEIGH 20
#define BUTTON_LENGHT   55
CAnimControlBar::CAnimControlBar(void)
{
	_clientRect.left=0;
	_clientRect.right=300;
	_clientRect.top=0;
	_clientRect.bottom=26;
	_state=StateStoped;
	_imageSize.cx=16;
	_imageSize.cy=16;
	_curTimeVal=20;
	_startRange=0;
	_endRange=100;
	_innerRefTime=0;
	_activeMark=55;
	_bMouseDown=FALSE;
	_interval=500;
	_pDC=NULL;
	_bForceStop=FALSE;
}

CAnimControlBar::~CAnimControlBar(void)
{
}
BOOL CAnimControlBar::Create(UINT nIDTemplate, CWnd* pParentWnd,int x,int y,int ilength,DWORD wStyle)
{
	int lenght=ilength;
	int height=26;
	_clientRect.left=x;
	_clientRect.top=y;
	_clientRect.right=x+lenght;
	_clientRect.bottom=y+height;

	CWnd::Create(NULL,NULL,wStyle,_clientRect,pParentWnd,nIDTemplate,NULL);
	_owner=pParentWnd;
	//create two control button.
	RECT BtRec;
	BtRec.left=0;BtRec.right=24;
	BtRec.top=height-24;BtRec.bottom=height;
	_play_pauseBt.Create(NULL, WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,BtRec,this,ID_BUTTONPLATPAUSE);
	BtRec.left=26;BtRec.right=50;
	_stopBt.Create(NULL,WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,BtRec,this,ID_BUTTONSTOP);

	//intit process bar range ,shows in client window.
	CRect rect;
	GetClientRect(&rect);
	_proRect.right=rect.right-INDICATOR_WIDTH/2;
	_proRect.left= rect.left+BUTTON_LENGHT+INDICATOR_WIDTH/2;
	_proRect.top=rect.bottom-9;
	_proRect.bottom=rect.bottom-6;

	//
	_activeMark=_proRect.left;

	//initial indicator rectangle ,the range will be used for response user's mouse envent.
	_indicRect.left=_proRect.left-INDICATOR_WIDTH/2;
	_indicRect.right=_indicRect.left+INDICATOR_WIDTH;
	_indicRect.top=rect.top+4;
	_indicRect.bottom=rect.bottom-INDICATOR_HEIGH;

	_pDC=GetDC();
	_bmpCompDC.CreateCompatibleBitmap(_pDC,_proRect.Width()+INDICATOR_WIDTH,rect.Height());
	ReleaseDC(_pDC);

	CreateImageList(_indImage,IDB_INDICATOR,12,12);
	_UpdateUI();
	SetTimer(1,10,NULL);
	return TRUE;
	  return TRUE;
}

void CAnimControlBar::OnClickPlayPauseBt()
{
	 if(_state==StatePause||_state==StateStoped)
	 {
		 _OnPlaying(_curTimeVal);
		 _state=StatePlaying;
	 }
	 else
	 {
		 _OnPause(_curTimeVal);
		 _state=StatePause;
	 }
	 _UpdateUI();
}
void  CAnimControlBar::Stop()
{
	_bForceStop=TRUE;
}
void  CAnimControlBar::Play()
{
	_bForceStop=FALSE;
}
void CAnimControlBar::OnClickStopBt()
{
	_OnStop(_curTimeVal);
	_state=StateStoped;
	_curTimeVal=_startRange;
	_activeMark=_proRect.left;
	_innerRefTime=0;
	_UpdateUI();
	OnEnd();
}
void CAnimControlBar::OnEnd()
{
	_innerRefTime=0;
	SetCurrentTick(_startRange);
	_OnRunning(0,_startRange);
	_OnEnd();
}
void CAnimControlBar::_DrawProcessBar()
{
	_pDC=GetDC();
	ASSERT(_pDC);
	CRect rect;
	GetClientRect(&rect);
	CDC memDC;
	CBitmap *oldBmp=NULL;
	CBrush  brush, *oldbr,LineBr,finBrush,indicBr;
	brush.CreateSolidBrush(RGB(212,208,200));
	LineBr.CreateSolidBrush(RGB(95,175,255));
	finBrush.CreateSolidBrush(RGB(50,50,255));
	indicBr.CreateSolidBrush(RGB(0,0,0));
	memDC.CreateCompatibleDC(_pDC);
	oldBmp=memDC.SelectObject(&_bmpCompDC);
	oldbr=memDC.SelectObject(&brush);
	memDC.SelectObject(&LineBr);
    memDC.SelectObject(&finBrush);
	memDC.SelectObject(&indicBr);
	
	//clear the screen color to background color.
	CRect rectAc;
	rectAc.left=rectAc.top=0;
	rectAc.bottom=rect.Height();
	rectAc.right=_proRect.Width()+INDICATOR_WIDTH;
	memDC.FillRect(&rectAc,&brush);
	
	//draw the frame edage
	//draw total process bar rectangle.
	RECT lineRect;
	lineRect=_proRect;
	lineRect.left=INDICATOR_WIDTH/2;
	lineRect.right=_proRect.right-_proRect.left+INDICATOR_WIDTH/2;
	memDC.FillRect(&lineRect,&LineBr);
	
	//draw current position indicator.
	RECT indRect;
	int offset=_proRect.left-INDICATOR_WIDTH/2;
	indRect=_indicRect;
	indRect.left=_activeMark-offset-INDICATOR_WIDTH/2;
	indRect.right=indRect.left+INDICATOR_WIDTH;
	POINT  indPoint;
	indPoint.x=indRect.left;
	indPoint.y=indRect.top;
	//update _indicRect
	_indicRect.left=_activeMark-INDICATOR_WIDTH/2;
	_indicRect.right=_indicRect.left+INDICATOR_WIDTH;
	_indImage.Draw(&memDC,0,indPoint,0);

	//draw complete process bar ,indicate finished task.
	int toRight=_activeMark-offset;
	RECT  finRect;
	finRect=lineRect;
	finRect.right=toRight;
	memDC.FillRect(&finRect,&finBrush);

	// present to screen
	_pDC->BitBlt(_proRect.left-INDICATOR_WIDTH/2,rect.top,_proRect.Width()+INDICATOR_WIDTH,rect.Height(),&memDC,0,0,SRCCOPY);
	//_pDC->Draw3dRect(&rect,RGB(100,100,100),RGB(100,100,100));
	ReleaseDC(_pDC);
	memDC.SelectObject(oldBmp);
	memDC.SelectObject(oldbr);	
}
void CAnimControlBar::SetAnimRange(DWORD startTime,DWORD endTime)
{
	_startRange=startTime;
	_endRange=endTime;

}
void CAnimControlBar::_UpdateUI()
{
	switch(_state)
	{
	case StateStoped:
		{
			_play_pauseBt.SetBitmap(_imageSize,IDB_PLAY);
			_stopBt.SetBitmap(_imageSize,IDB_STOP);
			break;
		}
	case StatePause:
		{
			_play_pauseBt.SetBitmap(_imageSize,IDB_PLAY);
			_stopBt.SetBitmap(_imageSize,IDB_STOP);
			break;
		}
	case StatePlaying:
		{
			_play_pauseBt.SetBitmap(_imageSize,IDB_PAUSE);
			_stopBt.SetBitmap(_imageSize,IDB_STOP);
			break;
		}
	default:
		break;
	}
}

//for the runtime class invade code.
IMPLEMENT_DYNAMIC(CAnimControlBar,CWnd)

BEGIN_MESSAGE_MAP(CAnimControlBar,CWnd)
	ON_COMMAND(ID_BUTTONPLATPAUSE,OnClickPlayPauseBt)	
	ON_COMMAND(ID_BUTTONSTOP,OnClickStopBt)	
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()


// override function for child class.
void CAnimControlBar::_OnPlaying(DWORD _timeVal)
{
	//AfxMessageBox("_OnPlaying()");
}
void CAnimControlBar::_OnPause(DWORD _timeVal)
{
	//AfxMessageBox("_OnPause()");
}
void CAnimControlBar::_OnStop(DWORD _timeVal)
{
	//AfxMessageBox("_OnStop()");
}
void CAnimControlBar::_OnEnd()
{
	//AfxMessageBox("_OnEnd()");
}
void CAnimControlBar::OnTimer(UINT_PTR nIDEvent)
{
	DWORD curSystemTick=GetTickCount(); //system function ,return minsec from computer startup
	if(nIDEvent==1)
	{
		DWORD  curTick=0;
		if(_state==StatePlaying&&!(_bMouseDown)&&(!_bForceStop))
		{
			_innerRefTime+=(float)(curSystemTick-_preTickCount)/1000;
			if(_innerRefTime>_totalRefTime)  OnEnd();
		    curTick=(DWORD)(_startRange+_innerRefTime*_interval);
			SetCurrentTick(curTick);
			_OnRunning(_innerRefTime,curTick);
		}
		else if(_bMouseDown)
		{
		
		}
	}
	_preTickCount=curSystemTick;
	_DrawProcessBar();
	CWnd::OnTimer(nIDEvent);
}

void CAnimControlBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(&rect);
	rect.left=_proRect.left;
	rect.right=_proRect.right;
	if(rect.PtInRect(point))
	{
		_activeMark=point.x;
		_DrawProcessBar(); 
		_bMouseDown=TRUE;
		SetCapture();
	}
	CWnd::OnLButtonDown(nFlags, point);
}

void CAnimControlBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	_bMouseDown=FALSE;
	ReleaseCapture();
	CWnd::OnLButtonUp(nFlags, point);
}

void CAnimControlBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove(nFlags, point);
	if(!_bMouseDown) return;
	if(point.x>_proRect.right) 
		_activeMark=_proRect.right;
	else if(point.x<_proRect.left)
		_activeMark=_proRect.left;
	else
		_activeMark=point.x;
	if(TRUE)
	{
		float  ratio=(float)(_activeMark-_proRect.left)/_proRect.Width();
		_innerRefTime=ratio*_totalRefTime;
		DWORD  curTick=(DWORD)(_startRange+_innerRefTime*_interval);
		SetCurrentTick(curTick);
		_OnRunning(_innerRefTime,curTick);
	}

	_DrawProcessBar();
}
void CAnimControlBar::OnCaptureChanged(CWnd *pWnd)
{
		_bMouseDown=FALSE;

	CWnd::OnCaptureChanged(pWnd);
}
DWORD CAnimControlBar::GetCurrentTick()
{
	 DWORD curTime,incrTime;
	 float ratio=(float)(_activeMark-_proRect.left)/_proRect.Width();
	 incrTime=(DWORD)(ratio*(_endRange-_startRange));
	 curTime=_startRange+incrTime;
	 return curTime;
}
void CAnimControlBar::SetCurrentTick(DWORD curTime)
{
	int incrProc;
	if(curTime<_startRange||curTime>_endRange)	return;
	float ratio=(float)(curTime-_startRange)/(_endRange-_startRange);
	incrProc=(int)(ratio*_proRect.Width());
    _activeMark=incrProc+_proRect.left;
}
void CAnimControlBar::_OnRunning(float reftime,DWORD _curTick)
{
	
}
//interval step per sec.
//sec total sec.
void CAnimControlBar::SetTotalTime(float sec)
{
	_totalRefTime=sec;
	_interval=(_endRange-_startRange)/sec;
}



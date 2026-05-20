
#pragma once
#include "GuiLib.h"
enum PlayState
{
	StatePlaying,
	StatePause,
	StateStoped,
};
class GuiLib_Api CAnimControlBar: public CWnd
{
	DECLARE_DYNAMIC(CAnimControlBar)
	// Modeless construct
public:
	CAnimControlBar(void);
	~CAnimControlBar(void);
	virtual BOOL Create(UINT nIDTemplate, CWnd* pParentWnd,int x,int y,int ilength=200,DWORD wStyle=WS_CHILD|WS_VISIBLE);
	// Modal construct
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickPlayPauseBt();
	afx_msg void OnClickStopBt();
	virtual void _OnPlaying(DWORD _timeVal);
	virtual void _OnPause(DWORD _timeVal);
	virtual void _OnStop(DWORD _timeVal);
	virtual void _OnRunning(float reftime,DWORD _curTick);
	virtual void _OnEnd();
protected:
	RECT  _clientRect;
	int  _activeMark;
	CRect _proRect;
	CXTButton   _play_pauseBt;
	CXTButton   _stopBt;
	PlayState   _state;
	CSize	_imageSize;
	//tick 

	DWORD   _curTimeVal;
	DWORD   _startRange;
	DWORD   _endRange;
	
	//control flag
	CDC * _pDC;
	BOOL  _bMouseDown;
	CWnd * _owner;
	CRect _indicRect;
	CBitmap _bmpCompDC;
	CImageList  _indImage;
	BOOL  _bForceStop;
	
	//inner time .
	float  _innerRefTime;
	float  _totalRefTime;
	float   _interval;
	DWORD   _preTickCount;
	//when the play state changed ,update the indicated icon.
	void _UpdateUI();
	void _DrawProcessBar();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	void SetAnimRange(DWORD startTime,DWORD endTime);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg void OnEnd();
public:
	DWORD  GetCurrentTick();
	void SetCurrentTick(DWORD curTime);
	void SetTotalTime(float sec);
public:
	void  Stop();
	void  Play();

};


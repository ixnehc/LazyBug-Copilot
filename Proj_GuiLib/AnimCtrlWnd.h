
#pragma once
#include "ResEditCtrl.h"
#include "AnimControlBar.h"
#include "../Common/anim/AnimPiece.h"
#include "../Common/resdata/AnimData.h"

class CAnimCtrlWnd :public CResEditCtrl ,public CAnimControlBar
{
public:
	CAnimCtrlWnd(void);
	~CAnimCtrlWnd(void);
//override function deriver from CAnimControlBar
public:
	virtual void _OnPlaying(DWORD _timeVal);
	virtual void _OnPause(DWORD _timeVal);
	virtual void _OnStop(DWORD _timeVal);
	virtual void _OnRunning(float reftime,DWORD _curTick);
	virtual void _OnEnd();
	virtual void EnableCtrl(BOOL bActive=TRUE);
// override  function deriver from CResEditCtrl
public:
	void SetPanel(CResEditPanel *panel)	{		_panel=panel;	}
	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
protected:
	void _ResetAnimControl();
private:
	CResEditPanel  *_panel;  //the panel that it is the owner of current control.
	ResEditPanelState *_state;
	DWORD _startTick;
	DWORD _endTick;
	AnimPiece *_curAnimPiece;
};

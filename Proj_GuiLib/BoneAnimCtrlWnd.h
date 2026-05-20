
#pragma once
#include "ResEditCtrl.h"
#include "AnimControlBar.h"
#include "../Common/anim/AnimPiece.h"
#include "../Common/resdata/AnimData.h"

class CBoneAnimCtrlWnd :public CResEditCtrl ,public CAnimControlBar
{
public:
	CBoneAnimCtrlWnd(void);
	~CBoneAnimCtrlWnd(void);
//override function deriver from CAnimControlBar
public:
	virtual void _OnRunning(float reftime,DWORD _curTick);
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
	BonesData2::BoneAnimPiece *_curAnimPiece;
};

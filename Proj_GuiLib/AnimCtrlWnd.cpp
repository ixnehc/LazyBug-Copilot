
/************************************************************************/
/*
e:\IxEngine\Proj_GuiLib\AnimCtrlWnd.h
author: star
purpose: write a animal control for add to AnimalPanel.
date: 2007-12-28
*/
/************************************************************************/
#include "stdh.h"
#include ".\AnimCtrlWnd.h"
#include "BoneAnimPanel.h"

CAnimCtrlWnd::CAnimCtrlWnd(void)
{
	_state=NULL;
}
CAnimCtrlWnd::~CAnimCtrlWnd(void)
{
}
void CAnimCtrlWnd::EnableCtrl(BOOL bActive)
{
	if(bActive)
		EnableWindow(TRUE);
	else
	{
		EnableWindow(FALSE);
		Stop();
		OnEnd();
	}
}
void CAnimCtrlWnd::_OnPlaying(DWORD _timeVal)
{

}
void CAnimCtrlWnd::_OnPause(DWORD _timeVal)
{
}
void CAnimCtrlWnd::_OnStop(DWORD _timeVal)
{

}
void CAnimCtrlWnd::_OnRunning(float reftime,DWORD _curTick)
{
	if(!_state) return;	
	Reps_Anim * _stateReps=(Reps_Anim *)_state;
	_stateReps->tCur=_curTick;
}
void CAnimCtrlWnd::_OnEnd()
{

}
void CAnimCtrlWnd::Bind(ResEditPanelState *state,BOOL bUpdateCtrl)
{
	  CResEditCtrl::Bind(state,bUpdateCtrl);
	  if(!bUpdateCtrl) return; 
	  if(!state) return;
	  _state=state;  //anchor to the state that the ResEditPanel retains.
	  Reps_Anim * _stateReps=(Reps_Anim *)_state;
	  _ResetAnimControl();
}
void CAnimCtrlWnd::_ResetAnimControl()
{
	Reps_Anim * _stateReps=(Reps_Anim *)_state;
	AnimData * animData=NULL;
	animData=(AnimData *)(_stateReps->resdata);
	if(!animData) return;
	KeySet  * keySet=animData->GetKeySet();
	if(animData->animpieces.size()==0) return;
	float sec=30;
	if(_stateReps->iSelAP==0xffffffff)  
	{
		SetAnimRange(0,0);
		_stateReps->tCur=0;
	}
    else
	{
		_curAnimPiece=&(animData->animpieces)[_stateReps->iSelAP];
		if(_curAnimPiece->tEnd==ANIMTICK_INFINITE)
			_curAnimPiece->tEnd=keySet->GetKey(keySet->GetKeyCount()-1)->t;
		SetAnimRange(_curAnimPiece->tStart,_curAnimPiece->tEnd);
		_stateReps->tCur=_curAnimPiece->tStart;
		sec=(float)(_curAnimPiece->tEnd-_curAnimPiece->tStart)/ANIMTICK_PER_SECOND;

	}
	SetTotalTime(sec);
	OnEnd();  //on reset AnimControl, will set OnEnd message to CAnimControlBar.
}

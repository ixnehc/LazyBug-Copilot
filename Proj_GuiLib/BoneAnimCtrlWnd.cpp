

#include "stdh.h"
#include "BoneAnimCtrlWnd.h"
#include "BoneAnimPanel.h"

CBoneAnimCtrlWnd::CBoneAnimCtrlWnd(void)
{
	_state=NULL;
}

CBoneAnimCtrlWnd::~CBoneAnimCtrlWnd(void)
{
}

void CBoneAnimCtrlWnd::EnableCtrl(BOOL bActive)
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

void CBoneAnimCtrlWnd::_OnRunning(float reftime,DWORD _curTick)
{
	if(!_state) return;	
	Reps_Anim * _stateReps=(Reps_Anim *)_state;
	_stateReps->tCur=_curTick;
}

void CBoneAnimCtrlWnd::Bind(ResEditPanelState *state,BOOL bUpdateCtrl)
{
	  CResEditCtrl::Bind(state,bUpdateCtrl);
	  if(!bUpdateCtrl) return; 
	  if(!state) return;
	  _state=state;  //anchor to the state that the ResEditPanel retains.
	  Reps_Anim * _stateReps=(Reps_Anim *)_state;
	  _ResetAnimControl();
}

void CBoneAnimCtrlWnd::_ResetAnimControl()
{
	Reps_Anim * _stateReps = (Reps_Anim *)_state;
	BonesData2 * boneData2 = NULL;
	boneData2 = (BonesData2 *)(_stateReps->resdata);
	if(!boneData2) 
		return;

	if(boneData2->animpieces.empty()) 
		return;
	
	float sec=30;
	if(_stateReps->iSelAP==0xffffffff)  
	{
		SetAnimRange(0,0);
		_stateReps->tCur=0;
	}
    else
	{
		_curAnimPiece = &(boneData2->animpieces)[_stateReps->iSelAP];
		if(_curAnimPiece->tEnd==ANIMTICK_INFINITE){
			 AnimTick t0,t1;
			 boneData2->GetTickRange(t0,t1);
			_curAnimPiece->tEnd =  t1;
		}

		SetAnimRange(_curAnimPiece->tStart,_curAnimPiece->tEnd);
		_stateReps->tCur=_curAnimPiece->tStart;
		sec=(float)(_curAnimPiece->tEnd-_curAnimPiece->tStart)/ANIMTICK_PER_SECOND;
	}

	SetTotalTime(sec);
	OnEnd();  //on reset AnimControl, will set OnEnd message to CAnimControlBar.
}




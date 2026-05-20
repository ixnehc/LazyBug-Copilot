#include "stdh.h"
#include ".\ResPanelAnimCtrl.h"
#include "BoneAnimPanel.h"

CResPanelAnimCtrl::CResPanelAnimCtrl(void)
{
	_state=NULL;
}
CResPanelAnimCtrl::~CResPanelAnimCtrl(void)
{
}
void CResPanelAnimCtrl::_OnPlaying(DWORD _timeVal)
{

}
void CResPanelAnimCtrl::_OnPause(DWORD _timeVal)
{
}
void CResPanelAnimCtrl::_OnStop(DWORD _timeVal)
{

}
void CResPanelAnimCtrl::_OnRunning(float reftime,DWORD _curTick)
{
	if(!_state) return;	
	Reps_Anim * _stateReps=(Reps_Anim *)_state;
	_stateReps->tCur=_curTick;
}
void CResPanelAnimCtrl::_OnEnd()
{

}
void CResPanelAnimCtrl::Bind(ResEditPanelState *state,BOOL bUpdateCtrl)
{
	  CResEditCtrl::Bind(state,bUpdateCtrl);
	  if(!bUpdateCtrl) return; 
	  if(!state) return;
	  _state=state;  //anchor to the state that the ResEditPanel retains.
	  _ResetAnimControl();
}
void CResPanelAnimCtrl::_ResetAnimControl()
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
		sec=(float)(_curAnimPiece->tEnd-_curAnimPiece->tStart)/TICK_PER_SECOND;

	}
	SetTotalTime(sec);
	OnEnd();  //on reset AnimControl, will set OnEnd message to CAnimControlBar.
}


#include "stdh.h"

#include "BoneAnimPieceRange.h"

#include "BoneAnimPanel.h"

#include "resdata/AnimData.h"

#include "Log/LogFile.h"

#include "stringparser/stringparser.h"

#include <assert.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BonesData2::BoneAnimPiece *GetBoneData2AP(Reps_Anim *state)
{
	if (!state)
		return NULL;
	if (!state->resdata)
		return NULL;
	if (state->iSelAP==-1)
		return NULL;

	return &((BonesData2*)(state->resdata))->animpieces[state->iSelAP];
}

//////////////////////////////////////////////////////////////////////////
//
void CBoneAnimPieceRange::CAPRangeEdit::OnBeginValueChange()
{
	BonesData2::BoneAnimPiece *ap=NULL;
	if(!_owner->GetState()) return;
	ap = GetBoneData2AP(_owner->GetState());

	_owner->GetState()->bAnimRangeDraging=TRUE;
	_owner->RefreshMod();

	_bDirty=FALSE;
}
void CBoneAnimPieceRange::CAPRangeEdit::OnEndValueChange()
{
	BonesData2::BoneAnimPiece *ap = GetBoneData2AP(_owner->GetState());
	if (!ap)
		return;

	if (!_bDirty)
		return;

	AnimTick tickStart = 0,tickEnd = 0;
	BonesData2 * boneData2  = (BonesData2 *)(_owner->GetState()->resdata);
	AnimTick t0,t1;
	if(boneData2){
		boneData2->GetTickRange(t0,t1);
		tickStart = i_math::clamp_i(ap->tStart,t0,t1);
		tickEnd = i_math::clamp_i(ap->tEnd,t0,t1);
	}

	//update each event time in this anim piece
	for (int i=0;i<ap->events.size();i++)
	{
		AnimEvent &e=ap->events[i];
		int v=e.tEvent+_tickStartOrg-tickStart;
		if (v<0)
			v=0;
		if (v>tickEnd)
			v=tickEnd;
		e.tEvent=v;
	}


	_owner->GetState()->tCur=ANIMTICK_INFINITE;
	_owner->GetState()->tEventAdjust=0;
	
	_owner->GetState()->bAnimRangeDraging=FALSE;

	_owner->RefreshMod();
}

void CBoneAnimPieceRange::CAPRangeEdit::OnValueChange(SlideSpinValue v)
{
	BOOL bStartOrEnd;
	if (this==&_owner->_editStart)
		bStartOrEnd=TRUE;
	else
		bStartOrEnd=FALSE;

	BonesData2::BoneAnimPiece *ap;
	ap = GetBoneData2AP(_owner->GetState());

	if (!ap)
		return;

	if (bStartOrEnd)
	{
		ap->tStart=(DWORD)v;
		if (ap->tEnd<ap->tStart)
			ap->tEnd=ap->tStart;
	}
	else
	{
		ap->tEnd=(DWORD)v;
		if (ap->tEnd<ap->tStart)
			ap->tStart=ap->tEnd;
	}

	if (TRUE)
	{
		BonesData2 * boneData2 = (BonesData2 *)(_owner->GetState()->resdata);
		AnimTick t0,t1;
		if(boneData2){
			boneData2->GetTickRange(t0,t1);	
			ap->tStart = i_math::clamp_i(ap->tStart,t0,t1);
			ap->tEnd = i_math::clamp_i(ap->tEnd,t0,t1);
		}
	}

	_owner->_editStart.SetValue(ap->tStart);
	_owner->_editEnd.SetValue(ap->tEnd);

	_owner->GetState()->tCur=(AnimTick)v;
	_owner->GetState()->tEventAdjust=(int)_tickStartOrg-(int)ap->tStart;

	_bDirty=TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CBoneAnimPieceRange
BOOL CBoneAnimPieceRange::Create(CRect &rcStartEdit,CRect &rcEndEdit,
							 UINT idStartEdit,UINT idEndEdit,CWnd*pParent)
{
	_editStart.Create(pParent,rcStartEdit,idStartEdit);
	_editEnd.Create(pParent,rcEndEdit,idEndEdit);

	_editStart.SetSpinSpeed(5.0f);
	_editEnd.SetSpinSpeed(5.0f);

	_editStart._owner=this;
	_editEnd._owner=this;

	return TRUE;
}
void CBoneAnimPieceRange::EnableCtrl(BOOL bActive)
{
	if(bActive)
	{
		_editEnd.EnableWindow(TRUE);
		_editStart.EnableWindow(TRUE);
	}
	else 
	{
		_editEnd.EnableWindow(FALSE);
		_editStart.EnableWindow(FALSE);
	}
}

void CBoneAnimPieceRange::Bind(ResEditPanelState *state,BOOL bUpdateCtrl)
{
	CResEditCtrl::Bind(state,bUpdateCtrl);

	if (bUpdateCtrl)
	{
		BonesData2 * boneData2 = (BonesData2 *)(_state->resdata);
		BonesData2::BoneAnimPiece *ap = GetBoneData2AP(GetState());
		BOOL bDisable = FALSE;
		if (!ap)
			bDisable = TRUE;
		else
		{
			if(!boneData2)
				bDisable = TRUE;
		}

		if(bDisable)
		{
			_editStart.SetRange(0,0);
			_editStart.SetValue(0);
			_editEnd.SetRange(0,0);
			_editEnd.SetValue(0);
			_editStart.EnableWindow(FALSE);
			_editEnd.EnableWindow(FALSE);
			return;
		}
		
		_editStart.EnableWindow(TRUE);
		_editEnd.EnableWindow(TRUE);
		
		AnimTick tStart,tEnd;
		boneData2->GetTickRange(tStart,tEnd);
		_editStart.SetRange(tStart,tEnd);
		_editEnd.SetRange(tStart,tEnd);
		_editStart.SetValue(ap->tStart);
		_editEnd.SetValue(ap->tEnd);
	}
}



#pragma once
#include "GuiLib.h"

#include "resource.h"
#include <vector>
#include "anim/AnimPiece.h"
#include "ResEditCtrl.h"

#include "SpinEdit.h"


struct AnimPiece;
class CResEditPanel;
struct ResEditPanelState;
struct Reps_Anim;

class CBoneAnimPieceRange:public CResEditCtrl
{

public:
	class CAPRangeEdit:public CSpinEdit
	{
	public:
		virtual void OnBeginValueChange();
		virtual void OnEndValueChange();

		virtual void OnValueChange(SlideSpinValue v);

	protected:
		CBoneAnimPieceRange *_owner;

		//used during changing
		BOOL _bDirty;
		AnimTick _tickStartOrg;

		friend class CBoneAnimPieceRange;
	};

	CBoneAnimPieceRange()
	{
	}

	BOOL Create(CRect &rcStartEdit,CRect &rcEndEdit,UINT idStartEdit,UINT idEndEdit,CWnd*pParent);
	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
	Reps_Anim *GetState()	{		return (Reps_Anim *)_state;	}
	virtual void EnableCtrl(BOOL bActive=TRUE);
protected:
	CAPRangeEdit _editStart,_editEnd;
	friend class CAPRangeEdit;
};

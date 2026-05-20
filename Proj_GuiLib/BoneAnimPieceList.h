
#pragma once
#include "GuiLib.h"

#include "resource.h"
#include <vector>
#include "anim/AnimPiece.h"

#include "ResEditCtrl.h"

#include "EditListBoxEx.h"

struct AnimPiece;
class CResEditPanel;
struct ResEditPanelState;
struct Reps_Anim;
struct BonesData2;

class GuiLib_Api CBoneAnimPieceList: public CEditListBoxEx,public CResEditCtrl
{
public:
	CBoneAnimPieceList(void){}
public:
	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
	Reps_Anim *GetState()	{		return (Reps_Anim *)_state;	}

protected:
	virtual void EnableCtrl(BOOL bActive=TRUE);
	virtual void _OnSelChange(DWORD iSel);
};


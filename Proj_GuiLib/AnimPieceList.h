
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
struct AnimData;





class GuiLib_Api CAnimPieceList: public CEditListBoxEx,public CResEditCtrl
{
public:
	CAnimPieceList()
	{
	}

public:
	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
	Reps_Anim *GetState()	{		return (Reps_Anim *)_state;	}
	AnimData *GetAnimData()	{		return (AnimData *)_GetResData();	}

protected:
	virtual void EnableCtrl(BOOL bActive=TRUE);
	virtual void _OnSelChange(DWORD iSel);
};


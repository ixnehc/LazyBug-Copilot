
#pragma once
#include "GuiLib.h"

#include "resource.h"
#include <vector>
#include "anim/AnimPiece.h"

struct AnimPiece;
class CResEditPanel;
struct ResEditPanelState;
struct Reps_Anim;

class GuiLib_Api CAnimEventList: public CXTEditListBox
{
public:
	CAnimEventList(CResEditPanel *panel)
	{
		_panel=panel;

		_state=NULL;

	}

public:
	void Bind(Reps_Anim *state);


	DECLARE_MESSAGE_MAP()
	afx_msg void OnEndLabelEdit();
	afx_msg void OnCancelLabelEdit();
	afx_msg void OnDeleteItem();
	afx_msg void OnMoveItemUp();
	afx_msg void OnMoveItemDown();
protected:
	void _EndLabelEdit(BOOL bCancel);
	BOOL _CheckNewAPName(const char *name);
	BOOL _SwapAnimPiece(int idx1,int idx2);


	CResEditPanel *_panel;

	Reps_Anim *_state;
	
};


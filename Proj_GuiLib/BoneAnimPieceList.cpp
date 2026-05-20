
#include "stdh.h"

#include "BoneAnimPieceList.h"

#include "CommonCtrlBase.h"

#include "Log/LogFile.h"

#include "stringparser/stringparser.h"

#include "BoneAnimPanel.h"

#include "resdata/AnimData.h"

#include "setter/setter.h"

#include "strlib/strlib.h"

#include <assert.h>




#ifdef _DEBUG
#define new DEBUG_NEW
#endif




//////////////////////////////////////////////////////////////////////////
//CBoneAnimPieceList
void CBoneAnimPieceList::Bind(ResEditPanelState *state0,BOOL bUpdateCtrl)
{
	CResEditCtrl::Bind(state0,bUpdateCtrl);
	if (!bUpdateCtrl)
		return;

	Reps_Anim *state=(Reps_Anim *)state0;
	BonesData2 * boneData2 = (BonesData2 *)(state->resdata);
	
	BoolSetter setter(_bAcceptSelChange,FALSE);
	std::vector<BonesData2::BoneAnimPiece>*vecAP=NULL;
	if (boneData2)
		vecAP=&(boneData2->animpieces);

	if (vecAP)
	{
		std::vector<std::string>temp;

		for (int i=0;i<vecAP->size();i++)
			temp.push_back(std::string(StrLib_GetStr((*vecAP)[i].name)));

		ListBox_UpdateItems(this,temp,TRUE);

		int sel=state->iSelAP;

		SetCurSel(sel);
		EnableWindow(TRUE);
	}
	else
	{
		ResetContent();
		EnableWindow(FALSE);
	}

	EnableEdit(FALSE);
}

void CBoneAnimPieceList::EnableCtrl(BOOL bActive)
{
	if(bActive)
		EnableWindow(TRUE);
	else
		EnableWindow(FALSE);
}

void CBoneAnimPieceList::_OnSelChange(DWORD iSel)
{
	if (_state)
	{
		if (GetState()->ChangeSelAP(iSel))
			RefreshMod();
	}
}


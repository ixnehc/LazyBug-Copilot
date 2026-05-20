/********************************************************************
	created:	2006/9/6   10:48
	filename: 	e:\IxEngine\Proj_GuiLib\AnimPieceList.cpp
	author:		cxi
	
	purpose:	a list that could edit anim pieces for AnimData
*********************************************************************/

#include "stdh.h"
#include "AnimPieceList.h"

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
//CAnimPieceList
void CAnimPieceList::Bind(ResEditPanelState *state0,BOOL bUpdateCtrl)
{
	CResEditCtrl::Bind(state0,bUpdateCtrl);
	if (!bUpdateCtrl)
		return;

	Reps_Anim *state=(Reps_Anim *)state0;

	BoolSetter setter(_bAcceptSelChange,FALSE);
	std::vector<AnimPiece>*vecAP=NULL;
	if (GetAnimData())
		vecAP=&(GetAnimData()->animpieces);
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
void CAnimPieceList::EnableCtrl(BOOL bActive)
{
	if(bActive)
		EnableWindow(TRUE);
	else
		EnableWindow(FALSE);
}

void CAnimPieceList::_OnSelChange(DWORD iSel)
{
	if (_state)
	{
		if (GetState()->ChangeSelAP(iSel))
			RefreshMod();
	}
}


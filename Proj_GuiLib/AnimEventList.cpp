/********************************************************************
	created:	2006/9/7   17:35
	filename: 	e:\IxEngine\Proj_GuiLib\AnimEventList.cpp
	author:		cxi
	
	purpose:	a list to edit anim event
*********************************************************************/

#include "stdh.h"
#include "AnimPieceList.h"

#include "CommonCtrlBase.h"

#include "Log/LogFile.h"

#include "stringparser/stringparser.h"

#include "BoneAnimPanel.h"

#include "resdata/AnimData.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CAnimEventList, CXTEditListBox)
	ON_LBN_XT_LABELEDITEND(XT_IDC_LBOX_EDIT, OnEndLabelEdit)
	ON_LBN_XT_LABELEDITCANCEL(XT_IDC_LBOX_EDIT, OnCancelLabelEdit)
	ON_LBN_XT_DELETEITEM(XT_IDC_GROUP_EDIT, OnDeleteItem)
	ON_LBN_XT_MOVEITEMUP(XT_IDC_GROUP_EDIT, OnMoveItemUp)
	ON_LBN_XT_MOVEITEMDOWN(XT_IDC_GROUP_EDIT, OnMoveItemDown)

END_MESSAGE_MAP()

std::vector<AnimEvent>* GetVecAE(Reps_Anim *state)
{
	if (!state->data)
		return NULL;

	if (_state->iSelAP==-1)
		return NULL;

	return &(state->data->animpieces)[state->iSelAP].events;
}


void CAnimEventList::Bind(Reps_Anim *state)
{
	std::vector<AnimEvent>*vecAE=GetVecAE(state);
	if (vecAE)
	{
		std::vector<std::string>temp;

		for (int i=0;i<vecAE->size();i++)
			temp.push_back(std::string((*vecAE)[i].name));

		ListBox_UpdateItems(this,temp,TRUE);

		SetCurSel(state->iSelEvent);
		EnableWindow(TRUE);
	}
	else
	{
		ResetContent();
		EnableWindow(FALSE);
	}

	_state=state;
}


void CAnimEventList::OnEndLabelEdit()
{
	_EndLabelEdit(FALSE);
}

void CAnimEventList::OnCancelLabelEdit()
{
	_EndLabelEdit(TRUE);
}


void CAnimEventList::_EndLabelEdit(BOOL bCancel)
{
	BOOL bInvalid=FALSE;
	if (TRUE)
	{
		std::string sItem=(LPCTSTR)m_strItemText;
		if (sItem.length()>=MAX_ANIMEVENT_NAME-1)
		{
			LogFile::Prompt("Too long AnimEvent name found!");
			bInvalid=TRUE;
		}
		else
		{
			int idx;
			idx=ListBox_Find(this,sItem.c_str(),FALSE);
			if ((idx!=-1)&&(idx!=GetCurrentIndex()))
			{
				LogFile::Prompt("Duplicated AnimEvent name found!");
				bInvalid=TRUE;
			}
			else
			{
				if ((m_bNewItem&&(sItem!=""))||(!m_bNewItem))
				{
					if (IsBlankString(sItem.c_str()))
					{
						LogFile::Prompt("Blank AnimEvent name found!");
						bInvalid=TRUE;
					}
				}
			}
		}
	}

	if (bInvalid)
	{
		BOOL b=m_bNewItem;
		EditItem(GetCurrentIndex());
		m_bNewItem=b;
		return;
	}
	SetCurSel(GetCurrentIndex());
	CXTEditListBox::OnEndLabelEdit();

	if (bCancel)
		return;

	if (m_strItemText=="")
		return;

	assert(_state);
	assert(_panel);

	int idx;
	idx=GetCurrentIndex();
	std::string name;
	name=(LPCTSTR)m_strItemText;
	std::vector<AnimEvent>*vecAE=GetVecAE(_state);

	if (strcmp((*vecAE)[idx].name,name.c_str())==0)
		return;//No change

	if (!m_bNewItem)
		strcpy((*vecAE)[idx].name,name.c_str());
	else
	{
		AnimEvent ae;
		strcpy(ae.name,name.c_str());
		ae.tEvent=0;
		vecAE->insert(vecAE->begin()+idx,ae);
	}

	_state->iSelEvent=GetCurSel();

	_state->IncFileVer();//Need save
	_panel->ApplyStateMod(FALSE);

}


void CAnimEventList::OnDeleteItem()
{
	int idx;
	idx=GetCurSel();
	if (idx==-1)
		return;


	CXTEditListBox::OnDeleteItem();

	std::vector<AnimEvent>*vecAE=GetVecAE(_state);
	vecAE->erase(vecAE->begin()+idx);
	_state->iSelEvent=GetCurSel();

	_state->IncFileVer();//Need save
	_panel->ApplyStateMod(FALSE);

}

//return whether any real change occurs
BOOL CAnimEventList::_SwapAnimEvent(int idx1,int idx2)
{
	std::vector<AnimPiece>*vecAP=&(_state->data->animpieces);

	if (idx1<0)		idx1=0;
	if (idx2<0)		idx2=0;
	if (idx1>vecAP->size()-1)		idx1=vecAP->size()-1;
	if (idx2>vecAP->size()-1)		idx2=vecAP->size()-1;

	if (idx1==idx2)
		return FALSE;
	AnimPiece apTemp;
	apTemp=(*vecAP)[idx1];
	(*vecAP)[idx1]=(*vecAP)[idx2];
	(*vecAP)[idx2]=apTemp;
	return TRUE;
}


void CAnimEventList::OnMoveItemUp()
{
	int idx;
	idx=GetCurSel();
	if (idx==-1)
		return;

	CXTEditListBox::OnMoveItemUp();
	if (!_SwapAnimPiece(idx-1,idx))
		return;

	_state->ChangeSelAP(GetCurSel());

	_state->IncFileVer();//Need save
	_panel->ApplyStateMod(FALSE);
	
}
void CAnimEventList::OnMoveItemDown()
{
	int idx;
	idx=GetCurSel();
	if (idx==-1)
		return;

	CXTEditListBox::OnMoveItemDown();
	if (!_SwapAnimPiece(idx,idx+1))
		return;

	_state->ChangeSelAP(GetCurSel());

	_state->IncFileVer();//Need save
	_panel->ApplyStateMod(FALSE);
}


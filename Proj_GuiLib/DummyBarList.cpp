#include "stdh.h"
#include ".\dummybarlist.h"
#include "resdata/DummiesData.h"
#include "DummiesEditPanel.h"
#include "CommonCtrlBase.h"
#include "setter/setter.h"
#define  STATE ((Reps_Dummies *)_state)
#define  DUMMIES ((DummiesData *)(_state->resdata))
#define  CHECK_IDX(idx)  \
	if(idx>=DUMMIES->dummies.size()||idx<0) \
	return;									\


CDummyBarList::CDummyBarList(void)
{
	_SetNameLimit(MAX_DUMMYNAMELEN-1);
}

CDummyBarList::~CDummyBarList(void)
{
}
BEGIN_MESSAGE_MAP(CDummyBarList,CEditListBoxEx)
END_MESSAGE_MAP()
void CDummyBarList::Bind(ResEditPanelState *state,BOOL bUpdateCtrl)
{	
		
	    CResEditCtrl::Bind(state,bUpdateCtrl);
		Reps_Dummies * stateDum=(Reps_Dummies *)state;	
		if(FALSE==bUpdateCtrl||!state) return;
		BoolSetter setter(_bAcceptSelChange,FALSE);
		DummiesData * dummies=(DummiesData*)stateDum->resdata;
		if(dummies)
		{
			std::vector<std::string>  items;
			for(int i=0;i<dummies->dummies.size();i++)
			{
				Dummy * dummy=&(dummies->dummies[i]);
				items.push_back(std::string(dummy->name));
			}
			ListBox_UpdateItems(this,items,FALSE);
			if(stateDum&&stateDum->dummyIdx>=0) 
				SetCurSel(stateDum->dummyIdx);
			EnableWindow(TRUE);
		}
		else
		{
			ResetContent();
			EnableWindow(FALSE);
		}
}
void CDummyBarList::EnableCtrl(BOOL bActive)
{
	if(bActive)
		EnableWindow(TRUE);
	else
		EnableWindow(FALSE);
}
void CDummyBarList::_OnNewItem(DWORD idx,const char *name)
{	
	Dummy  dummy;
	strncpy(dummy.name,name,MAX_DUMMYNAMELEN);
	dummy.idxBone=0;
	dummy.setType(DummyInfo::BoundType_Sphere);
	DUMMIES->dummies.insert(DUMMIES->dummies.begin()+idx,dummy);
	RefreshMod();
}
void CDummyBarList::_OnChangeItem(DWORD idx,const char *name)
{
	CHECK_IDX(idx);
	strcpy(((DummiesData*)(_state->resdata))->dummies[idx].name,name);
	RefreshMod();
}
void CDummyBarList::_OnDeleteItem(DWORD idx)
{
	CHECK_IDX(idx);
	Reps_Dummies * state = (Reps_Dummies * )_state;
	DUMMIES->dummies.erase(DUMMIES->dummies.begin()+idx);
	if(state->dummyIdx == idx)
		state->dummyIdx = idx-1;
	RefreshMod();
}
void CDummyBarList::_OnSwapItem(DWORD idx1,DWORD idx2)
{
	CHECK_IDX(idx1);
	CHECK_IDX(idx2);
	Dummy dummy;
	dummy= ((DummiesData *)(_state->resdata))->dummies[idx1];
    ((DummiesData *)(_state->resdata))->dummies[idx1]=((DummiesData *)(_state->resdata))->dummies[idx2];
	((DummiesData *)(_state->resdata))->dummies[idx2]=dummy;
	Reps_Dummies * stateDum=(Reps_Dummies *)_state;	
	stateDum->dummyIdx = GetCurSel();
	RefreshMod();

}
void CDummyBarList::_OnSelChange(DWORD iSel)
{
	Reps_Dummies * stateDum=(Reps_Dummies *)_state;
	if(!stateDum) return;
	if(stateDum->dummyIdx!=iSel)
	{
		stateDum->dummyIdx=iSel;
		RefreshMod();
	}
}





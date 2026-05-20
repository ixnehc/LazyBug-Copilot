/************************************************************************/
/*  author:star
	date: 2008-01-14
	pupose: ui for edit dummy properties.
*/
/************************************************************************/
#include "stdh.h"
#include "RichGridComboItem.h"
#include "DummyPropertiesWnd.h"
#include "DummiesEditPanel.h"
#include "resdata/DummiesData.h"
#include "RichGridFloatItem.h"
#define  Dummy_State ((Reps_Dummies *)_state)
#define  Dummy_ResData ((DummiesData *)(_state->resdata))


CDummyPropertiesWnd::CDummyPropertiesWnd()
{
	itemType = NULL;
}
CDummyPropertiesWnd::~CDummyPropertiesWnd()
{

}
BEGIN_MESSAGE_MAP(CDummyPropertiesWnd,CRichGrid)
END_MESSAGE_MAP()
void CDummyPropertiesWnd::EnableCtrl(BOOL bActive)
{
	if(bActive)
		EnableWindow(TRUE);
	else
	{
		ResetContent();
		EnableWindow(FALSE);
	}
}
void CDummyPropertiesWnd::OnBeginItemChange(CXTPPropertyGridItem *item)
{
	if(item==itemType)
	{
		DummiesData * dummies=Dummy_ResData;
		Reps_Dummies *_stateDum=Dummy_State;
		if(_stateDum->dummyIdx>=0)
			btType = dummies->dummies[_stateDum->dummyIdx].getBoundType();
	}

	RecordState(state);

	if(item!=itemType)
		LockPaint();
}
void CDummyPropertiesWnd::OnItemChange(CXTPPropertyGridItem *item)
{
	
}
void CDummyPropertiesWnd::OnEndItemChange(CXTPPropertyGridItem *item)
{
	if(item==itemType)
	{
		DummiesData * dummies=Dummy_ResData;
		Reps_Dummies *_stateDum=Dummy_State;
		if(_stateDum->dummyIdx>=0)
		{
			DWORD t = dummies->dummies[_stateDum->dummyIdx].getBoundType();
			if(t!=btType)
			{
				dummies->dummies[_stateDum->dummyIdx].getBoundType() = btType;
				dummies->dummies[_stateDum->dummyIdx].setType(t);
			}
		}
	}

	RefreshMod(FALSE);

	if(item!=itemType)
		UnLockPaint();
	
	RestoreState(state);
}
void CDummyPropertiesWnd::OnItemCommand(CXTPPropertyGridItem *item,DWORD idCmd)
{
}
void CDummyPropertiesWnd::Bind(ResEditPanelState *state,BOOL bUpdateCtrl)
{
	CResEditCtrl::Bind(state,bUpdateCtrl);
	DummiesData * dummies=Dummy_ResData;
	Reps_Dummies *_stateDum=Dummy_State;
	if(NULL==dummies||NULL==_stateDum)  return;
	if(bUpdateCtrl)
	{
		LockPaint();
		ResetContent();
		if(_stateDum->dummyIdx<0)
		{
			UnLockPaint();
			return;
		}
		Dummy *dummy=&(dummies->dummies[_stateDum->dummyIdx]);
		BeginInsert();
		InsertCategory("Dummy data","Dummy properties.");
		
		PushInsert();
		
		std::vector<std::string> dummyTypes;
		dummyTypes.push_back("sphere");
		dummyTypes.push_back("obb");
		dummyTypes.push_back("point");
		itemType = InsertComboItem<DWORD>("type","Dummy type.",(&(dummies->dummies[_stateDum->dummyIdx].getBoundType())),dummyTypes);	
		
		DWORD btype = dummies->dummies[_stateDum->dummyIdx].getBoundType();

		switch(btype)
		{
		case DummyInfo::BoundType_Sphere :
			{
				i_math::spheref * pSph = dummies->dummies[_stateDum->dummyIdx].getSphere();
				assert(pSph);

				CRichGrid_FloatItem  *item= (CRichGrid_FloatItem *)InsertFloatItem("radius","the radius of the sphere which surround current dummy.",&(pSph->radius),0,1000.0f);
				item->SetSlideSpeed(0.01);
				InsertVec3Item("center","the center of the bounding sphere.",&(pSph->center));
				break;
			}
		case DummyInfo::BoundType_AABB:
			{
				i_math::aabbox3df * abb = dummies->dummies[_stateDum->dummyIdx].getAAbb();
				InsertAbbItem("aabb","the bounding box.",abb);
				break;
			}
		case DummyInfo::BoundType_Point:
			{
				i_math::vector3df * point = dummies->dummies[_stateDum->dummyIdx].getPoint();
				InsertVec3Item("center","the center of the dummy point.",point);
				break;
			}
		default:
			break;
		}
						
		// combobox select bone for dummy.
		if(TRUE)
		{
			std::vector<std::string> items;
			for(int i=0;i<dummies->skeletonInfo.size();i++)
				items.push_back(std::string(dummies->skeletonInfo[i].name));
			InsertComboItem<DWORD>("Bone index","index of the bone which current dummy connect to.",(&(dummies->dummies[_stateDum->dummyIdx].idxBone)),items);	
		}
		
		InsertXformItem("matrix","the dummy offset matrix.",&(dummy->matOff));
		PopInsert();
		ExpandAll();
		UnLockPaint();
	}
}



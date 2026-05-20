// ***************************************************************
//  GObjItem   version:  1.0   ? date: 07/01/2007
//  -------------------------------------------------------------
//  author:		ixnehc
//  -------------------------------------------------------------
//  Copyright (C) 2007 - All Rights Reserved
// ***************************************************************
//  Purpose: item for editing GObj
// ***************************************************************

#include "stdh.h"
#include "RichGrid.h"
#include "GObjItem.h"

#include "gds/GObj.h"

#include <assert.h>

//////////////////////////////////////////////////////////////////////////
//CGObjVectorItem
IMPLEMENT_DYNAMIC(CGObjVectorItem,CXTPPropertyGridItem)

void CGObjVectorItem::OnButtonClick(DWORD idButton)
{
	if ((!(idButton&(ID_RGIB_Clear|ID_RGIB_New)))||
		(!_elem)||(!_owner))
	{
		CRichGrid_ButtonItem::OnButtonClick(idButton);
		return;
	}

	CRichGrid *grid=GetRichGrid(this);
	std::string path;
	grid->OnBeginItemChange(this);

	if (idButton==ID_RGIB_Clear)
	{
		_elem->ClearSub(_owner);
		grid->OnItemChange(this);
	}
	if (idButton==ID_RGIB_New)
	{
		path=grid->PathFromItem(this);
		_elem->NewSub(_owner);
		grid->OnItemChange(this);
	}

	grid->OnEndItemChange(this);

	//NOTE:after calling OnEndItemChange(..),the grid has been reset and this item 
	//is no longer valid,so never use "this" pointer in the following code

	if (idButton==ID_RGIB_New)
	{//select&expand the new added item
		CXTPPropertyGridItem *item=grid->ItemFromPath(path.c_str());
		if (item)
		{
			item->Expand();
			//select&expand the last child of item
			if (item->GetChilds()->GetCount()>0)
			{
				item=item->GetChilds()->GetAt(item->GetChilds()->GetCount()-1);
				item->Select();
				item->Expand();
			}
		}
	}

	return;
}

void DoElemCommand(CRichGrid *grid,CXTPPropertyGridItem *item,void *owner,GElemBase *elem,int iSub,DWORD idButton)
{
	std::string path,path2;

	if (idButton&(ID_RGIB_MoveUp|ID_RGIB_MoveDown))
	{
		CXTPPropertyGridItem *itemT;
		if (idButton==ID_RGIB_MoveUp)
			itemT=grid->GetPrevItem(item);
		else
			itemT=grid->GetNextItem(item);
		if (!itemT)
			return;
		path=grid->PathFromItem(item);
		path2=grid->PathFromItem(itemT);
	}


	grid->OnBeginItemChange(item);
	switch (idButton)
	{
	case ID_RGIB_Remove:
		{
			elem->RemoveSub(owner,iSub);
			break;
		}
	case ID_RGIB_MoveUp:
		{
			elem->MoveSub(owner,iSub,TRUE);
			break;
		}
	case ID_RGIB_MoveDown:
		{
			elem->MoveSub(owner,iSub,FALSE);
			break;
		}
	case ID_RGIB_Clone:
		{
			path=grid->PathFromItem(item->GetParentItem());
			elem->CloneSub(owner,iSub);
			break;
		}
	default:
		assert(FALSE);
	}

	grid->OnItemChange(item);
	grid->OnEndItemChange(item);

	//NOTE:after calling OnEndItemChange(..),the grid has been reset and this item 
	//is no longer valid,so never use "this" pointer in the following code
	if (idButton&(ID_RGIB_MoveUp|ID_RGIB_MoveDown))
	{
		CXTPPropertyGridItem *item;
		item=grid->ItemFromPath(path.c_str());
		item->Collapse();
		item=grid->ItemFromPath(path2.c_str());
		item->Collapse();
		item->Select();
	}

	if (idButton==ID_RGIB_Clone)
	{//select&expand the new added item
		CXTPPropertyGridItem *item=grid->ItemFromPath(path.c_str());
		if (item)
		{
			item->Expand();
			//select&expand the last child of item
			if (item->GetChilds()->GetCount()>0)
			{
				item=item->GetChilds()->GetAt(item->GetChilds()->GetCount()-1);
				item->Select();
				item->Expand();
			}
		}
	}

}


//////////////////////////////////////////////////////////////////////////
//CGObjSubItem
void CGObjSubItem::OnButtonClick(DWORD idButton)
{
	if ((!(idButton&(ID_RGIB_MoveDown|ID_RGIB_MoveUp|ID_RGIB_Remove|ID_RGIB_Clone)))||
		(!_elem)||(!_owner))
	{
		CRichGrid_ButtonItem::OnButtonClick(idButton);
		return;
	}

	DoElemCommand(GetRichGrid(this),this,_owner,_elem,_iSub,idButton);
}

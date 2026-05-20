/********************************************************************
	created:	30:7:2006   19:55
	filename: 	d:\IxEngine\Proj_GuiLib\TreeCtrlBase.cpp
	file path:	d:\IxEngine\Proj_GuiLib
	file base:	TreeCtrlBase
	author:		cxi
	
	purpose:	useful functions for tree ctrl processing
*********************************************************************/
#include "stdh.h"

#include "TreeCtrlBase.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"

#pragma warning(disable:4018)

HTREEITEM ItemFromPath(CTreeCtrl *pTreeCtrl,std::vector<std::string>names,BOOL bNoCase)
{
	HTREEITEM hItem;
	hItem=TVI_ROOT;

	int i;
	for (i=0;i<names.size();i++)
	{
		BOOL bFound;
		bFound=FALSE;
		TREEVIEW_BEGIN_RECURSIVE(pTreeCtrl,hChildItem,hItem)
			CString s;
			s=pTreeCtrl->GetItemText(hChildItem);
			BOOL bEqual;
			if (!bNoCase)
				bEqual = (names[i] == toMBCS((LPCTSTR)s));
			else
				bEqual=StringEqualNoCase(names[i].c_str(), toMBCS((LPCTSTR)s));
			if (bEqual)
			{
				hItem=hChildItem;
				bFound=TRUE;
				break;
			}
		TREEVIEW_END_RECURSIVE()

		if (!bFound)
			return NULL;
	}

	return hItem;
}


HTREEITEM ItemFromPath(CTreeCtrl *pTreeCtrl,std::string &path,const char *sep,BOOL bNoCase)
{
	std::vector<std::string>vecTemp;
	SplitStringBy(sep,path,&vecTemp);

	return ItemFromPath(pTreeCtrl,vecTemp,bNoCase);
}


//if the item with the name already exists,this function will not insert a new item,instead,
//it return the handle to that existing item
HTREEITEM SafeInsertItem(CTreeCtrl *pTreeCtrl,HTREEITEM hParent,const char *name,int ii,DWORD dwData)
{
	HTREEITEM hItem=NULL;
	TREEVIEW_BEGIN_RECURSIVE(pTreeCtrl,hChildItem,hParent)
		CString s;
		s=pTreeCtrl->GetItemText(hChildItem);
		if (s==name)
		{
			hItem=hChildItem;
			break;
		}
	TREEVIEW_END_RECURSIVE()

	if (!hItem)
	{	
		hItem = pTreeCtrl->InsertItem(fromMBCS(name), ii, ii, hParent);
		pTreeCtrl->SetItemData(hItem,dwData);
	}

	return hItem;
}

HTREEITEM InsertItemByPath(CTreeCtrl *pTreeCtrl,std::string &path,std::string &sep,int iiFolder,int iiItem,DWORD dwDataFolder,DWORD dwDataItem)
{
	std::vector<std::string>vecTemp;
	SplitStringBy(sep.c_str(),path,&vecTemp);

	HTREEITEM hItem;
	hItem=TVI_ROOT;

	int i;
	for (i=0;i<vecTemp.size();i++)
	{
		BOOL bFound;
		bFound=FALSE;
		TREEVIEW_BEGIN_RECURSIVE(pTreeCtrl,hChildItem,hItem)
		CString s;
		s=pTreeCtrl->GetItemText(hChildItem);
		if (vecTemp[i] == toMBCS((LPCTSTR)s))
		{
			hItem=hChildItem;
			bFound=TRUE;
			break;
		}
		TREEVIEW_END_RECURSIVE()

		if (!bFound)
		{
			if (i!=vecTemp.size()-1)
			{
				hItem = pTreeCtrl->InsertItem(fromMBCS(vecTemp[i].c_str()), iiFolder, iiFolder, hItem);
				pTreeCtrl->SetItemData(hItem,dwDataFolder);
			}
			else
			{
				hItem=pTreeCtrl->InsertItem(fromMBCS(vecTemp[i].c_str()),iiItem,iiItem,hItem);
				pTreeCtrl->SetItemData(hItem,dwDataItem);
			}
			if (hItem==NULL)
				break;
		}
	}

	return hItem;
}

//Whether the point is in the blank area(right side,non-item area)
//pt is in client coordinates
BOOL BlankAreaHitTest(CTreeCtrl *pTreeCtrl,CPoint pt)
{
	CRect rcClient;
	pTreeCtrl->GetClientRect(&rcClient);
	if (!rcClient.PtInRect(pt))
		return FALSE;

	HTREEITEM hItem;
	hItem=pTreeCtrl->HitTest(pt);
	if (hItem==NULL)
		return TRUE;

	if (TRUE)
	{
		CRect rc;
		pTreeCtrl->GetItemRect(hItem,&rc,TRUE);
		rc.left-=36;
		if (!rc.PtInRect(pt))
			return TRUE;
	}

	return FALSE;
}



int GetFirstVisibleItem(CTreeCtrl *pTreeCtrl,std::vector<HTREEITEM>&vecItemList)
{
	CRect rcClient;
	pTreeCtrl->GetClientRect(&rcClient);

	int i;
	for (i=0;i<vecItemList.size();i++)
	{
		CRect rc;
		if (pTreeCtrl->GetItemRect(vecItemList[i],&rc,FALSE))
		{
			if ((rc&rcClient).IsRectEmpty())
				continue;
			return i;
		}
	}

	return -1;
}

int GetLastVisibleItem(CTreeCtrl *pTreeCtrl,std::vector<HTREEITEM>&vecItemList)
{
	CRect rcClient;
	pTreeCtrl->GetClientRect(&rcClient);

	int i;
	for (i=(int)vecItemList.size()-1;i>=0;i--)
	{
		CRect rc;
		if (pTreeCtrl->GetItemRect(vecItemList[i],&rc,FALSE))
		{
			if ((rc&rcClient).IsRectEmpty())
				continue;
			return i;
		}
	}
	return -1;
}

void CollectItemList(CTreeCtrl *pTreeCtrl,HTREEITEM hParentItem,std::vector<HTREEITEM>&vecItemList)
{
	if (hParentItem!=TVI_ROOT)
		vecItemList.push_back(hParentItem);

	if (TRUE)
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = pTreeCtrl->GetChildItem(hParentItem);

		while (hChildItem != NULL)
		{
			hNextItem = pTreeCtrl->GetNextItem(hChildItem, TVGN_NEXT);

			CollectItemList(pTreeCtrl,hChildItem,vecItemList);

			hChildItem = hNextItem;
		}
	}
}


void CollectExpandedItemList(CTreeCtrl *pTreeCtrl,HTREEITEM hParentItem,std::vector<HTREEITEM>&vecItemList)
{
	BOOL bExpanded;
	if (hParentItem!=TVI_ROOT)
	{
		vecItemList.push_back(hParentItem);

		UINT state;
		state=pTreeCtrl->GetItemState(hParentItem,TVIS_EXPANDED);
		bExpanded=(state&TVIS_EXPANDED);
	}
	else
		bExpanded=TRUE;

	if (bExpanded)
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = pTreeCtrl->GetChildItem(hParentItem);

		while (hChildItem != NULL)
		{
			hNextItem = pTreeCtrl->GetNextItem(hChildItem, TVGN_NEXT);

			CollectExpandedItemList(pTreeCtrl,hChildItem,vecItemList);

			hChildItem = hNextItem;
		}
	}
}

void CollectVisibleItemList(CTreeCtrl *pTreeCtrl,std::vector<HTREEITEM>&vecItemList)
{
	vecItemList.clear();
	CollectExpandedItemList(pTreeCtrl,TVI_ROOT,vecItemList);
	if (vecItemList.size()<=0)
		return;

	if (TRUE)//Exclude those not visible
	{
		int iFirst,iLast;
		iFirst=GetFirstVisibleItem(pTreeCtrl,vecItemList);
		iLast=GetLastVisibleItem(pTreeCtrl,vecItemList);
		ASSERT(iFirst!=-1);
		ASSERT(iLast!=-1);
		vecItemList.resize(iLast+1);
		vecItemList.erase(vecItemList.begin(),vecItemList.begin()+iFirst);
	}
}


BOOL CheckSiblingItemNameDupe(CTreeCtrl *pTreeCtrl,HTREEITEM hItem,const char *name)
{
	if (hItem==TVI_ROOT)
		return FALSE;

	HTREEITEM hParentItem;
	hParentItem=pTreeCtrl->GetParentItem(hItem);
	if (!hParentItem)
		hParentItem=TVI_ROOT;

	std::string s1,s2;
	s1=name;
	StringUpper(s1);

	TREEVIEW_BEGIN_RECURSIVE(pTreeCtrl,hChildItem,hParentItem)
		if (hChildItem!=hItem)
		{
			s2 = toMBCS(pTreeCtrl->GetItemText(hChildItem));
			StringUpper(s2);
			if (s2==s1)
				return TRUE;
		}
	TREEVIEW_END_RECURSIVE()

	return FALSE;
}


std::string PathFromItem(CTreeCtrl *pTreeCtrl,HTREEITEM hItem,const char *sep)
{
	std::string s;
	if (hItem!=TVI_ROOT)
	{
		s= toMBCS(pTreeCtrl->GetItemText(hItem));
		while(1)
		{
			HTREEITEM hParent;
			hParent=pTreeCtrl->GetParentItem(hItem);
			if(hParent==NULL)
				break;

			std::string ss;
			ss= toMBCS(pTreeCtrl->GetItemText(hParent));
			s=ss+sep+s;
			hItem=hParent;
		}
	}

	return s;
}


//check whether hItem1 is ancestor of hItem2
BOOL CheckAncestor(CTreeCtrl *pTreeCtrl,HTREEITEM hItem1,HTREEITEM hItem2)
{
	HTREEITEM hItem;
	while(1)
	{
		hItem=pTreeCtrl->GetParentItem(hItem2);
		if (hItem==hItem1)
			return TRUE;
		if ((hItem==TVI_ROOT)||(hItem==NULL))
			break;
		hItem2=hItem;
	}
	return FALSE;
}

void _TreeCtrlClone(CTreeCtrl *src,HTREEITEM hSrcItem,CTreeCtrl *target,HTREEITEM hTargetItem)
{
	if (hSrcItem!=TVI_ROOT)
	{
		target->SetItemText(hTargetItem,src->GetItemText(hSrcItem));
		target->SetItemData(hTargetItem,src->GetItemData(hSrcItem));
		int ii,iiSel;
		src->GetItemImage(hSrcItem,ii,iiSel);
		target->SetItemImage(hTargetItem,ii,iiSel);
		target->SetItemState(hTargetItem,src->GetItemState(hSrcItem,TVIS_BOLD|TVIS_EXPANDED),TVIS_BOLD|TVIS_EXPANDED);
	}
	TREEVIEW_BEGIN_RECURSIVE(src,hSrcChild,hSrcItem)
		HTREEITEM hTargetChild;
		hTargetChild = target->InsertItem(_T(""), 0, 0, hTargetItem);
		_TreeCtrlClone(src,hSrcChild,target,hTargetChild);
	TREEVIEW_END_RECURSIVE()
}

void TreeCtrlClone(CTreeCtrl *src,CTreeCtrl *target)
{
	target->DeleteAllItems();
	_TreeCtrlClone(src,TVI_ROOT,target,TVI_ROOT);
}

void TreeCtrlEnsureItemExpanded(CTreeCtrl *ctrl,HTREEITEM hItem)
{
	HTREEITEM hParent=ctrl->GetParentItem(hItem);
	while((hParent!=TVI_ROOT)&&(hParent!=NULL))
	{
		ctrl->Expand(hParent,TVE_EXPAND);
		hParent=ctrl->GetParentItem(hParent);
	}
}

static void _RecordTreeCtrlItemState(CTreeCtrl *ctrl,HTREEITEM hItem,
														TreeCtrlState &state,const char *sep)
{
	if (hItem!=TVI_ROOT)
	{
		TreeCtrlItemState t;
		t.bSel=(ctrl->GetItemState(hItem,TVIS_SELECTED)&TVIS_SELECTED)!=0;
		t.bExpand=(ctrl->GetItemState(hItem,TVIS_EXPANDED)&TVIS_EXPANDED)!=0;

		if (t.bSel||t.bExpand)
			state[PathFromItem(ctrl,hItem,sep)]=t;
	}

	TREEVIEW_BEGIN_RECURSIVE(ctrl,hChild,hItem)
		_RecordTreeCtrlItemState(ctrl,hChild,state,sep);
		
	TREEVIEW_END_RECURSIVE()

}

void RecordTreeCtrlState(CTreeCtrl *ctrl,TreeCtrlState &state,const char *sep)
{
	_RecordTreeCtrlItemState(ctrl,TVI_ROOT,state,sep);

}

static void _RestoreTreeCtrlItemState(CTreeCtrl *ctrl,HTREEITEM hItem,
														TreeCtrlState &state,const char *sep)
{
	if (hItem!=TVI_ROOT)
	{
		std::string path=PathFromItem(ctrl,hItem,sep);

		TreeCtrlState::iterator it=state.find(path);
		if (it!=state.end())
		{
			TreeCtrlItemState *p=&(*it).second;
			ctrl->SetItemState(hItem,p->bSel?TVIS_SELECTED:0,TVIS_SELECTED);

			if (p->bExpand)
				ctrl->Expand(hItem,TVE_EXPAND);
			else
				ctrl->Expand(hItem,TVE_COLLAPSE);
//			ctrl->SetItemState(hItem,p->bExpand?TVIS_EXPANDED:0,TVIS_EXPANDED);
		}
	}

	TREEVIEW_BEGIN_RECURSIVE(ctrl,hChild,hItem)
		_RestoreTreeCtrlItemState(ctrl,hChild,state,sep);

	TREEVIEW_END_RECURSIVE()

}


void RestoreTreeCtrlState(CTreeCtrl *ctrl,TreeCtrlState &state,const char *sep)
{
	_RestoreTreeCtrlItemState(ctrl,TVI_ROOT,state,sep);
}


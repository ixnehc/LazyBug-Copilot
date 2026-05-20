#pragma once

#include "GuiLib.h"

#include <string>
#include <vector>
#include <unordered_map>

#ifndef TREEVIEW_BEGIN_RECURSIVE
#define TREEVIEW_BEGIN_RECURSIVE(pTreeCtrl,hChildItem,hParentItem) \
	if (TRUE)\
{\
	HTREEITEM hNextItem;\
	HTREEITEM hChildItem = (pTreeCtrl)->GetChildItem(hParentItem);\
	hNextItem=hChildItem;\
	while ((hChildItem = hNextItem)!= NULL)\
	{\
	hNextItem = (pTreeCtrl)->GetNextItem(hChildItem, TVGN_NEXT);
#endif

#ifndef TREEVIEW_END_RECURSIVE
#define TREEVIEW_END_RECURSIVE() \
	}\
}
#endif


extern HTREEITEM ItemFromPath(CTreeCtrl *pTreeCtrl,std::string &path,const char *sep,BOOL bNoCase);
extern HTREEITEM ItemFromPath(CTreeCtrl *pTreeCtrl,std::vector<std::string>names,BOOL bNoCase);

extern HTREEITEM SafeInsertItem(CTreeCtrl *pTreeCtrl,HTREEITEM hParent,const char *name,int ii,DWORD dwData);

extern HTREEITEM InsertItemByPath(CTreeCtrl *pTreeCtrl,std::string &path,std::string &sep,int iiFolder=0,int iiItem=0,DWORD dwDataFolder=0,DWORD dwDataItem=0);

extern BOOL CheckSiblingItemNameDupe(CTreeCtrl *pTreeCtrl,HTREEITEM hItem,const char *name);

extern std::string PathFromItem(CTreeCtrl *pTreeCtrl,HTREEITEM hItem,const char *sep);

extern void CollectItemList(CTreeCtrl *pTreeCtrl,HTREEITEM hParentItem,std::vector<HTREEITEM>&vecItemList);

//check whether hItem1 is ancestor of hItem2
extern BOOL CheckAncestor(CTreeCtrl *pTreeCtrl,HTREEITEM hItem1,HTREEITEM hItem2);

extern void TreeCtrlClone(CTreeCtrl *src,CTreeCtrl *target);

extern void TreeCtrlEnsureItemExpanded(CTreeCtrl *ctrl,HTREEITEM hItem);


struct TreeCtrlItemState
{
	BOOL bExpand;
	BOOL bSel;
};

class TreeCtrlState:public std::unordered_map<std::string,TreeCtrlItemState>
{
public:
	TreeCtrlState&operator=(TreeCtrlState&src)
	{
		(std::unordered_map<std::string,TreeCtrlItemState>&)(*this)=(std::unordered_map<std::string,TreeCtrlItemState>&)src;
		return *this;
	}
};

void RecordTreeCtrlState(CTreeCtrl *ctrl,TreeCtrlState &state,const char *sep);
void RestoreTreeCtrlState(CTreeCtrl *ctrl,TreeCtrlState &state,const char *sep);

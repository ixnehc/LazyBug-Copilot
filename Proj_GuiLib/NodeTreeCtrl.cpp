// ***************************************************************
//  NodeTreeCtrl   version:  1.0   ? date: 04/14/2008
//  -------------------------------------------------------------
//  author:		ixnehc
//  -------------------------------------------------------------
//  Copyright (C) 2008 - All Rights Reserved
// ***************************************************************
//  Purpose: gui control for CNodeTree
// ***************************************************************
#include "stdh.h"
#include "commondefines\general_stl.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>

#include "TreeCtrlBase.h"

#include "NodeTreeCtrl.h"


#include "interface/interface.h"

#include "assert.h"

#include "Log/LogFile.h"

#include "stringparser/stringparser.h"

#include "timer/profiler.h"


#include "resource.h"


#include "WMGuiLib.h"
#include ".\nodetreectrl.h"

#pragma warning(disable:4311)
#pragma warning(disable:4312)

#define ID_COMMON_ADDEXIST_START ((ID_COMMON_NEW_START+ID_COMMON_NEW_END)/2)



//////////////////////////////////////////////////////////////////////////
//CNodeTreeCtrl
BEGIN_MESSAGE_MAP(CNodeTreeCtrl, CXTTreeCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_COMMON_DELETE,OnDelete)
	ON_COMMAND(ID_COMMON_CUT,OnCut)
	ON_COMMAND(ID_COMMON_COPY,OnCopy)
	ON_COMMAND(ID_COMMON_PASTE,OnPaste)
	ON_COMMAND(ID_COMMON_RENAME,OnRename)
	ON_COMMAND(ID_COMMON_MOVEUP,OnMoveUp)
	ON_COMMAND(ID_COMMON_MOVEDOWN,OnMoveDown)
//	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTvnSelchanged)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnTvnBeginlabeledit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnTvnEndlabeledit)
	//modify by star. 2007-11-6
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
END_MESSAGE_MAP()

CNodeTreeCtrl::CNodeTreeCtrl()
{
	_bEditable=TRUE;
	_hOwner=NULL;
	_bRecursiveCheck=TRUE;
	_bNewItem=FALSE;

	_ssc=NULL;
	_bInSscOp=FALSE;

	_iSscUpdate=0;

	_treeref=NULL;
}


BOOL CNodeTreeCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style|=TVS_SHOWSELALWAYS|TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|0x8000|TVS_EDITLABELS;

	return CXTTreeCtrl::PreCreateWindow(cs);
}


int CNodeTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	extern BOOL CreateImageList(CImageList& il, UINT nID,int w,int h);
	UINT nID=_GetImageID();
	if (nID!=0xffffffff)
	{
		CreateImageList(_imgNode,nID,16,16);
		SetImageList(&_imgNode,TVSIL_NORMAL);
	}

	EnableMultiSelect(TRUE);

	XTPImageManager()->SetIcons(IDR_TOOLBAREXT);


	return 0;
}

void CNodeTreeCtrl::OnDestroy()
{
	SAFE_RELEASE(_treeref);
	_ClearMenu();
	_imgNode.CImageList::~CImageList();
	_imgNode.CImageList::CImageList();

}

HTREEITEM CNodeTreeCtrl::ItemFromPath(std::vector<std::string>&names)
{
	if (!_Tree())
		return NULL;
	HTREEITEM hItem=TVI_ROOT;

	for (int i=0;i<names.size();i++)
	{
		BOOL bFound=FALSE;

		TREEVIEW_BEGIN_RECURSIVE(this,hChildItem,hItem)
			if (names[i]==
				_Tree()->GetName(_GetItemData(hChildItem)))
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

HTREEITEM CNodeTreeCtrl::ItemFromPath(std::string &path)
{
	if (!_Tree())
		return NULL;

	std::vector<std::string>pieces;
	SplitStringBy(_Tree()->GetSep(),path,&pieces);
	return ItemFromPath(pieces);
}

HTREEITEM CNodeTreeCtrl::ItemFromNodeHandle(NodeHandle hNode)
{
	std::string path=_Tree()->GetPath(hNode);
	return ItemFromPath(path);
}


BOOL CNodeTreeCtrl::IsChecked(HTREEITEM item)
{
	if(item!= TVI_ROOT){
		DWORD state = GetItemState(item,TVIS_STATEIMAGEMASK)>>12;
		if(state==2)
			return TRUE;
	}

	return FALSE;
}
BOOL CNodeTreeCtrl::SetChecked(HTREEITEM item,BOOL bChecked)
{
	if(item!= TVI_ROOT){
		DWORD state = (bChecked)?2:1;
		SetItemState(item,INDEXTOSTATEIMAGEMASK(state),TVIS_STATEIMAGEMASK);
		return TRUE;
	}
	return FALSE;
}
void CNodeTreeCtrl::EnableEdit(BOOL bEnable)	
{
	if (_bEditable==bEnable)
		return;
	LONG_PTR style=GetWindowLongPtr(GetSafeHwnd(),GWL_STYLE);
	if (bEnable)
		style|=TVS_EDITLABELS;
	else
		style&=~TVS_EDITLABELS;
	SetWindowLongPtr(GetSafeHwnd(),GWL_STYLE,(LONG)style);
	_bEditable=bEnable;
}



BOOL CNodeTreeCtrl::Create(CWnd *pParent,RECT &rc,UINT id,DWORD StyleEx/* = 0*/)
{
	if (!CXTTreeCtrl::Create(StyleEx|WS_CHILD|WS_VISIBLE|WS_BORDER|
											TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|
											TVS_EDITLABELS,rc,pParent,id))
		return FALSE;

	return TRUE;
}

void CNodeTreeCtrl::_AddNode(HTREEITEM hParent,NodeHandle hNode)
{

	if (_OnFilterItem(hNode))
		return;

	HTREEITEM hItem;	
	if (hNode!=NodeHandle_Root)
	{
		hItem=InsertItem(_T(""),0,0,hParent);

		if (hItem)
		{
			SetItemData(hItem,(DWORD_PTR)hNode);
			_UpdateItem(hItem,hNode);
		}
	}
	else
		hItem=TVI_ROOT;

	if (hItem)
	{
		DWORD c=_Tree()->GetChildCount(hNode);
		for (int i=0;i<c;i++)
			_AddNode(hItem,_Tree()->GetChild(hNode,i));
	}
}

BOOL CNodeTreeCtrl::UpdateItem(NodeHandle hNode)
{
	if (!_Tree())
		return FALSE;

	HTREEITEM hItem=ItemFromNodeHandle(hNode);
	if ((hItem==0)||(hItem==TVI_ROOT))
		return FALSE;

	_UpdateItem(hItem,hNode);
	return TRUE;
}

BOOL CNodeTreeCtrl::_SupportCheckBox()
{
	return _Tree()->SupportCheck();
}

void CNodeTreeCtrl::_UpdateItem(HTREEITEM hItem,NodeHandle hNode)
{

	std::string name=_GetShowName(hNode);

	SscState ss=SSC_UNKNOWN;
// 	if (_ssc)
// 	{
// 		std::string pathSsc=_Tree()->GetSscPath(hNode);
// 		_ssc->GetState(pathSsc.c_str(),ss);
// 	}
	DWORD idx=_GetImageIdx(hNode,ss);
	SetItemImage(hItem,idx,idx);

	SetItemText(hItem, fromMBCS(name.c_str()));

	//modify by star. 2007-11-6  init the check-box state{
	if(_SupportCheckBox())
	{
		BOOL  bChecked=_Tree()->TestNodeCheck(hNode);
		if(bChecked)
			SetItemState(hItem,INDEXTOSTATEIMAGEMASK(2),TVIS_STATEIMAGEMASK);
		else
			SetItemState(hItem,INDEXTOSTATEIMAGEMASK(1),TVIS_STATEIMAGEMASK);
	}
	//}

}

void CNodeTreeCtrl::_UpdateNode(HTREEITEM hItem,NodeHandle hNode)
{


	if (hNode!=NodeHandle_Root)
	{
		SetItemData(hItem,(DWORD_PTR)hNode);
		_UpdateItem(hItem,hNode);
	}
	else
		hItem=TVI_ROOT;

	if (hItem)
	{
		DWORD c=_Tree()->GetChildCount(hNode);
		DWORD i=0;
		TREEVIEW_BEGIN_RECURSIVE(this,hChild,hItem)

			if (i>=c)
				DeleteItem(hChild);
			else
				_UpdateNode(hChild,_Tree()->GetChild(hNode,i));

			i++;

		TREEVIEW_END_RECURSIVE();

		for (;i<c;i++)
		{
			HTREEITEM hChild=InsertItem(_T(""),hItem);
			_UpdateNode(hChild,_Tree()->GetChild(hNode,i));
		}

	}

}



BOOL CNodeTreeCtrl::SetNodeTree(NodeTreeRef *treeref)
{
	DeleteAllItems();
	_sel.Clear();
	_clip.Clear();
	SAFE_REPLACE(_treeref,treeref);
	if (!_treeref)
		return TRUE;

	if (!_Tree())
		return FALSE;

	_AddNode(NULL,NodeHandle_Root);

	return TRUE;
}

BOOL CNodeTreeCtrl::UpdateNodeTree(NodeTreeRef *treeref)
{
	_sel.Clear();
	_clip.Clear();

	SAFE_REPLACE(_treeref,treeref);
	if (!_treeref)
	{
		DeleteAllItems();
		return TRUE;
	}

	if (!_Tree())
		return FALSE;

	CXTTreeCtrl::LockPaint();

	TreeCtrlState state;
	RecordTreeCtrlState(this,state,_Tree()->GetSep());

	_UpdateNode(TVI_ROOT,NodeHandle_Root);

	RestoreTreeCtrlState(this,state,_Tree()->GetSep());

	CXTTreeCtrl::UnLockPaint();

	return TRUE;
}


void CNodeTreeCtrl::_CollapseItems(std::vector<HTREEITEM>&collapsed,std::vector<HTREEITEM>&total)
{
	collapsed=total;

	int i=0;
	while(i<collapsed.size())
	{
		int j;
		for (j=0;j<collapsed.size();j++)
		{
			if (i==j)
				continue;
			if (CheckAncestor(this,collapsed[j],collapsed[i]))
				break;
		}
		if (j<collapsed.size())//this item is a descendent of another item,remove it
		{
			collapsed.erase(collapsed.begin()+i);
			continue;
		}
		i++;
	}
}


void CNodeTreeCtrl::_RecordSel(ItemGroupDesc2 *sel)
{
	sel->Clear();
	if (TRUE)//first get all the select
	{
		HTREEITEM hItem;
		hItem=GetFirstSelectedItem();
		if (hItem)
			sel->total.push_back(hItem);
		while(hItem=GetNextSelectedItem(hItem))
			sel->total.push_back(hItem);
	}

	//now simplify the selections 
	if (TRUE)
	{
		std::vector<HTREEITEM>total;
		_CollapseItems(total,sel->total);
		_sel.total=total;
	}

}

void CNodeTreeCtrl::_RecordClip()
{
	_ClearClip();
	_clip.Copy(_sel);

	for (int i=0;i<_clip.total.size();i++)
	{
		SetItemState(_clip.total[i],TVIS_CUT,TVIS_CUT);
		SetItemColor(_clip.total[i],RGB(128,128,128));
	}
}
void CNodeTreeCtrl::_ClearClip()
{
	for (int i=0;i<_clip.total.size();i++)
	{
		SetItemState(_clip.total[i],0,TVIS_CUT);
		SetItemColor(_clip.total[i],0);
	}
	_clip.Clear();
}


void CNodeTreeCtrl::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
//	if (_pDescWnd)
//	{
//		HTREEITEM hSel;
//		hSel=GetSelectedItem();
//		ResData *p;
//		p=(ResData *)GetItemData(hSel);
//		std::string desc;
//		if (p)
//			p->CalcContent(desc);
//
//		_pDescWnd->SetWindowText(desc.c_str());
//	}

	*pResult = 0;
}

void CNodeTreeCtrl::NotifyOwner(UINT msg,DWORD_PTR param1,DWORD_PTR param2)
{
	if (!_hOwner)
		return;
	::SendMessage(_hOwner,msg,param1,param2);
}

 
void CNodeTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// If multiselect control, process possible left
	// click drag selection.
	if (m_bMultiSelect)
	{
		UINT nHitFlags = 0;

		HTREEITEM hItemHit = m_pTreeCtrl->HitTest(point, &nHitFlags);
		HTREEITEM hItemSel = m_pTreeCtrl->GetSelectedItem();

		// if expanding/contracting call base class.
		if ((nHitFlags & TVHT_ONITEMSTATEICON) != 0)
		{
			SelectAll(FALSE);
			SelectItem(hItemHit);
		}
	}

 	CXTTreeCtrl::OnLButtonDown(nFlags,point);
}

void CNodeTreeCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (!_Tree())
		return;

//	CXTTreeCtrl::OnRButtonDown(nFlags,point);

	//We copy some code from xtp960,the version 1122 seems working strange here

	// hittest to get the tree item under the cursor
	// and select it.
	UINT uFlags;
	HTREEITEM hItem = m_pTreeCtrl->HitTest(point, &uFlags);
	if (hItem != NULL && (uFlags & TVHT_ONITEM) != 0)
	{
		// if the item is not selected, clear previous
		// selections and select the item under cursor.
		if (!IsSelected(hItem))
		{
			SelectAll(FALSE);
			SelectItem(hItem);
		}
	}
	else
	{
		// clear previous selections.
		SelectAll(FALSE);
	}

	// get the owner of the tree control.
	HWND hWnd = m_pTreeCtrl->GetOwner()->m_hWnd;

	if (::IsWindow(hWnd))
	{
		// construct a NMHDR struct...
		NMHDR mHDR;
		mHDR.hwndFrom = m_pTreeCtrl->m_hWnd;
		mHDR.code     = NM_RCLICK;
		mHDR.idFrom   = m_pTreeCtrl->GetDlgCtrlID();

		// and send a WM_NOTIFY message to our owner.
		SendNotify( &mHDR );
	}

}


BOOL CNodeTreeCtrl::BuildContextMenu(CMenu *menu)
{
	_ClearMenu();

	if (!_Tree())
		return FALSE;


	_RecordSel(&_sel);
	int idx=0;

	NodeType typeParent;
	if (_sel.total.size()==1)
	{
		NodeHandle hNode=_GetItemData(_sel.total[0]);
		typeParent=_Tree()->GetType(hNode);
	}
	else
		typeParent=NodeType_Root;

	if (_IsEditable())
	{

		//Now the new xxx commands
		if (_sel.total.size()<=1)
		{
			DWORD c;
			NodeType *types=_Tree()->GetChildType(typeParent,c);
			if (c>0)
			{
				std::string s;
				std::vector<NodeTypeInfo *>info;
				info.resize(c);
				for (int i=0;i<c;i++)
					info[i]=_Tree()->GetTypeInfo(types[i]);

				//先加被归类的node type,(注意,暂时没有支持被归类的item的NewMethod标志)
				CMenu *menuNew=new CMenu;
				menuNew->CreatePopupMenu();

				DWORD bAny=FALSE;

				for (int i=0;i<c;i++)
				{
					NodeTypeInfo *p=info[i];
					if (!p)
						continue;

					if (p->category=="")
						continue;

					bAny=TRUE;

					std::string category=p->category;
					CMenu *mn=new CMenu;
					mn->CreatePopupMenu();
					DWORD idx2=0;
					for (int j=i;j<c;j++)
					{
						NodeTypeInfo *q=info[j];
						if (!q)
							continue;
						if (q->category!=category)
							continue;

						assert(q->type+ID_COMMON_NEW_START<ID_COMMON_NEW_END);
						if (_CanNew(q->type))
						{
							if (q->showname.empty())
								mn->InsertMenu(idx2++,MF_ENABLED|MF_STRING,
									ID_COMMON_NEW_START + q->type, fromMBCS(q->name.c_str()));
							else
								mn->InsertMenu(idx2++,MF_ENABLED|MF_STRING,
									ID_COMMON_NEW_START + q->type, fromMBCS(q->showname.c_str()));
							info[j]=NULL;
						}
					}

					FormatString(s,"%s",category.c_str());
					menuNew->AppendMenu(MF_POPUP|MF_ENABLED|MF_STRING,
									(UINT_PTR)mn->GetSafeHmenu(), fromMBCS(s.c_str()));
					
					_menus.push_back(mn);
				}

				if (bAny)
				{
					menu->AppendMenu(MF_POPUP|MF_ENABLED|MF_STRING,
						(UINT_PTR)menuNew->GetSafeHmenu(),_T("New"));

					_menus.push_back(menuNew);
				}
				else
					delete menuNew;


				//然后是未归类的type
				for (int i=0;i<c;i++)
				{
					NodeTypeInfo *q=info[i];
					if (!q)
						continue;
					if (q->category!="")
						continue;
					assert(q->type+ID_COMMON_NEW_START<ID_COMMON_NEW_END);

					if (_CanNew(q->type))
					{
						FormatString(s,"New %s",q->showname.empty()?q->name.c_str():q->showname.c_str());
						menu->AppendMenu(MF_ENABLED|MF_STRING,
							ID_COMMON_NEW_START + q->type, fromMBCS(s.c_str()));
					}
				}

			}
		}

		if (_sel.total.size()>0)
		{
			menu->AppendMenu(MF_ENABLED|MF_SEPARATOR,0,_T(""));

	//		menu->InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_COMMON_COPY,"Copy");
	//		menu->InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_COMMON_CUT,"Cut");
	//		if (_sel.total.size()==1)
	//			menu->InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_COMMON_PASTE,"Paste");
			menu->AppendMenu(MF_ENABLED|MF_STRING,ID_COMMON_DELETE,_T("Delete"));
			if (_sel.total.size()==1)
			{
				if (_GetActualItemSscState(_sel.total[0])==SSC_NOTCONTROLLED)
				{
					if (_CanRename(typeParent))
						menu->AppendMenu(MF_ENABLED|MF_STRING,ID_COMMON_RENAME,_T("Rename"));
				}
			}
		}

		if (_IsExchangable())
		{
			if (_sel.total.size()==1)
			{
				menu->AppendMenu(MF_ENABLED | MF_SEPARATOR, 0, _T(""));
				menu->AppendMenu(MF_ENABLED|MF_STRING,ID_COMMON_MOVEUP, _T("Move Up"));
				menu->AppendMenu(MF_ENABLED|MF_STRING,ID_COMMON_MOVEDOWN, _T("Move Down"));
			}
		}
	}

	//ssc menu items
	if (_ssc&&_CanSscOp())
	{
		if (menu->GetMenuItemCount()>0)
			menu->AppendMenu(MF_SEPARATOR,0, _T(""));

		if (_TestSscOp(&_sel.total[0],_sel.total.size(),CheckIn_))
			menu->AppendMenu(MF_ENABLED|MF_STRING,ID_SSC_CHECKIN, _T("Check In"));
		if (_TestSscOp(&_sel.total[0],_sel.total.size(),CheckIn_KeepOut))
			menu->AppendMenu(MF_ENABLED|MF_STRING,ID_SSC_CHECKIN_KEEPOUT, _T("Check In(Keep CheckedOut)"));
		if (_TestSscOp(&_sel.total[0],_sel.total.size(),CheckOut))
			menu->AppendMenu(MF_ENABLED|MF_STRING,ID_SSC_CHECKOUT, _T("Check Out"));
		if (_TestSscOp(&_sel.total[0],_sel.total.size(),Get))
			menu->AppendMenu(MF_ENABLED|MF_STRING,ID_SSC_GET, _T("Get Latest Version"));
		if (_TestSscOp(&_sel.total[0],_sel.total.size(),Add))
			menu->AppendMenu(MF_ENABLED|MF_STRING,ID_SSC_ADDTOVSS, _T("Add to SourceSafe Control"));
		if (_TestSscOp(&_sel.total[0],_sel.total.size(),Remove))
			menu->AppendMenu(MF_ENABLED|MF_STRING,ID_SSC_RMFROMVSS, _T("Remove from SourceSafe Control"));
		if (_TestSscOp(&_sel.total[0],_sel.total.size(),Refresh))
			menu->AppendMenu(MF_ENABLED|MF_STRING,ID_SSC_REFRESH, _T("Refresh SourceSafe State"));
		if (_TestSscOp(&_sel.total[0],_sel.total.size(),GetFolder))
			menu->AppendMenu(MF_ENABLED|MF_STRING,ID_SSC_GETFOLDER, _T("Get Folder"));

		if (menu->GetMenuItemCount()>0)
			menu->AppendMenu(MF_SEPARATOR,0, _T(""));
	}

	_OnCustomMenu(menu);

	return TRUE;

}


void CNodeTreeCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (!_Tree())
		return;

	CMenu menu;
	menu.CreatePopupMenu();

	BuildContextMenu(&menu);
	if (menu.GetMenuItemCount()<=0)
	{
		CXTTreeCtrl::OnRButtonDown(nFlags,point);
		return;
	}
 
	ClientToScreen(&point);

	XTFuncContextMenu(&menu,TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this,IDR_TOOLBAREXT);
//		menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, point.x,point.y,this,NULL );

	_ClearMenu();

	CXTTreeCtrl::OnRButtonDown(nFlags,point);
}

std::string CNodeTreeCtrl::_GenNewName(NodeType type,const char *nameType)
{
	std::string s;
	FormatString(s,"New %s",nameType);
	return s;
}

BOOL CNodeTreeCtrl::_GenUniqueName(NodeType type,std::string &name)
{
	return IncreaseTailOrdinal(name,3);
}


void CNodeTreeCtrl::_NewItem(NodeType type,const char *name_,void *param,HTREEITEM hParent)
{
	NodeHandle hParentNode=NodeHandle_Null;
	if (!hParent)
	{
		hParent=TVI_ROOT;
		hParentNode=NodeHandle_Root;

		if (_sel.total.size()>0)
		{
			hParent=_sel.total[0];
			hParentNode=_GetItemData(hParent);
		}
	}
	else
		hParentNode=_GetItemData(hParent);

	NodeHandle hNode=NodeHandle_Null;
	std::string name=name_;

	if(_CanAutoGenUniqueName(type))
	{
		while(!_Tree()->CheckChildName(hParentNode,name.c_str()))
		{
			if (FALSE==_GenUniqueName(type,name))
				break;
		}
	}

	hNode=_Tree()->AddChild(hParentNode,type,name.c_str(),param);

	if (!hNode)
		return;

	HTREEITEM hItem = InsertItem(_T(""), 0, 0, hParent);
	SetItemData(hItem,(DWORD_PTR)hNode);
	_UpdateItem(hItem,hNode);

	EnsureVisible(hItem);

	SscState stateSsc = _GetActualItemSscState(hItem);
	if ((stateSsc==SSC_NOTCONTROLLED)||(stateSsc==SSC_UNKNOWN))
	{
		if (_CanRename(type))
			EditLabel(hItem);
	}
}


void CNodeTreeCtrl::OnNew(NodeType type)
{
	if (!_Tree())
		return;

	std::string name;
	name=_GenNewName(type,_Tree()->GetTypeInfo(type)->name.c_str());
	
	_NewItem(type,name.c_str(),NULL);
}


BOOL CNodeTreeCtrl::_DeleteItemR(HTREEITEM hItem)
{
	std::vector<HTREEITEM>items;

	//first get all the children
	TREEVIEW_BEGIN_RECURSIVE(this,hChild,hItem)
		items.push_back(hChild);
	TREEVIEW_END_RECURSIVE();


	BOOL bAllDeleted=TRUE;
	if (items.size()>0)
	{//Has children,remove them first
		for (int i=items.size()-1;i>=0;i--)
		{
			if (FALSE==_DeleteItemR(items[i]))
				bAllDeleted=FALSE;
		}
	}

	if (!bAllDeleted)
		return FALSE;

	if (TRUE)
	{//no child now,remove item
		SscState stateSsc = _GetActualItemSscState(hItem);
		if ((stateSsc!=SSC_NOTCONTROLLED)&& (stateSsc != SSC_UNKNOWN))
			return FALSE;//处于source safe 控制下的文件不能删
		NodeHandle hNode=_GetItemData(hItem);

		if (FALSE==_Tree()->RemoveChild(hNode))
			return FALSE;

		DeleteItem(hItem);
	}

	return TRUE;
}


void CNodeTreeCtrl::_DeleteSels()
{
	ItemGroupDesc2 sel;
	sel.Copy(_sel);//复制一份,以防在DeleteItem过程中_sel的内容被修改

	for (int i=0;i<sel.total.size();i++)
		_DeleteItemR(sel.total[i]);
}

BOOL CNodeTreeCtrl::_PromptDelSel()
{
	if (IDOK!=AfxMessageBox(_T("Are you sure?"),MB_OKCANCEL))
		return FALSE;

	return TRUE;
}


void CNodeTreeCtrl::OnDelete()
{
	if (!_Tree())
		return;

	if (!_PromptDelSel())
		return;

	_DeleteSels();
}


void CNodeTreeCtrl::OnCopy()
{
}
void CNodeTreeCtrl::OnCut()
{
	_RecordClip();
}
void CNodeTreeCtrl::OnPaste()
{
}
void CNodeTreeCtrl::OnRename()
{
	if (!_Tree())
		return;

	EditLabel(_sel.total[0]);
}


struct CompareContext
{
	CTreeCtrl *ctrl;
	std::map<DWORD_PTR,DWORD > serials;
};

static int CALLBACK Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CompareContext *cc=(CompareContext *)lParamSort;
	CTreeCtrl* ctrl= cc->ctrl;

	DWORD idx1,idx2;
	idx1=cc->serials[(DWORD_PTR)lParam1];
	idx2=cc->serials[(DWORD_PTR)lParam2];


	if (idx1<idx2)
		return -1;
	if (idx1>idx2)
		return 1;
	return 0;
}


static BOOL MoveItem(CTreeCtrl *ctrl,HTREEITEM hItem,BOOL bUp)
{
	HTREEITEM hParent=ctrl->GetParentItem(hItem);
	if (hParent==NULL)
		hParent=TVI_ROOT;

	std::vector<HTREEITEM>siblings;
	int idx=-1,idx2=-1;
	TREEVIEW_BEGIN_RECURSIVE(ctrl,hChild,hParent)
		siblings.push_back(hChild);
	if(hChild==hItem)
		idx=siblings.size()-1;
	TREEVIEW_END_RECURSIVE()

		assert(idx!=-1);

	if (bUp)
	{
		if (idx<=0)
			return FALSE;
		idx2=idx-1;
	}
	else
	{
		if (idx>=siblings.size()-1)
			return FALSE;
		idx2=idx+1;
	}

	Swap<HTREEITEM>(siblings[idx],siblings[idx2]);

	CompareContext cc;

	cc.ctrl=ctrl;
	for (int i=0;i<siblings.size();i++)
	{
		cc.serials[ctrl->GetItemData(siblings[i])]=i;
	}

	TVSORTCB tvs;

	tvs.hParent = hParent;
	tvs.lpfnCompare = Compare;
	tvs.lParam = (LPARAM) &cc;

	ctrl->SortChildrenCB(&tvs);

	return TRUE;
}


void CNodeTreeCtrl::OnMoveUp()
{
	if (!_Tree())
		return;
	NodeHandle hNode=(NodeHandle )GetItemData(_sel.total[0]);
	if (_Tree()->Move(hNode,TRUE))
		MoveItem(this,_sel.total[0],TRUE);
}

void CNodeTreeCtrl::OnMoveDown()
{
	if (!_Tree())
		return;
	NodeHandle hNode=(NodeHandle )GetItemData(_sel.total[0]);
	if (_Tree()->Move(hNode,FALSE))
		MoveItem(this,_sel.total[0],FALSE);
}



void CNodeTreeCtrl::OnTvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	if (!_Tree())
		return;

	if (!_IsEditable())
	{
		*pResult = TRUE;
		return;
	}

	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	HTREEITEM hItem;
	hItem=pTVDispInfo->item.hItem;

	NodeHandle hNode=(NodeHandle)	GetItemData(hItem);

	NodeType type=_Tree()->GetType(hNode);
	if ((_GetActualItemSscState(hItem)!=SSC_NOTCONTROLLED)&&
		(_GetActualItemSscState(hItem) != SSC_UNKNOWN))
	{
		*pResult = TRUE;
		return;
	}
	if (!_CanRename(type))
	{
		*pResult = TRUE;
		return;
	}
		

	HWND hEdit=TreeView_GetEditControl(m_hWnd);
	::SetWindowText(hEdit, fromMBCS(_Tree()->GetName(hNode)));

	*pResult = 0;
	_sEditItem = toMBCS((LPCTSTR)GetItemText(hItem));

}


void CNodeTreeCtrl::OnTvnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	if (!_Tree())
		return;

	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;


	if (TRUE)
	{
		HTREEITEM hItem=pTVDispInfo->item.hItem;
		NodeHandle hNode=(NodeHandle )GetItemData(pTVDispInfo->item.hItem);
		std::string sNew;
		if (pTVDispInfo->item.pszText != NULL)
		{
			sNew = toMBCS(pTVDispInfo->item.pszText);
			_ModifyEdit(hNode,sNew);
		}
		else
			sNew = toMBCS((LPCTSTR)GetItemText(hItem));

		if (sNew=="")
		{
			EditLabel(pTVDispInfo->item.hItem);
			return;
		}


		if  (sNew==_sEditItem)
		{
			SetItemText(pTVDispInfo->item.hItem, fromMBCS(_GetShowName(hNode)));
			return;
		}


		std::string name=_Tree()->GetName(hNode);

		if (!_Tree()->Rename(hNode,sNew.c_str()))
		{
//			LogFile::Prompt("Illegal name!");
			EditLabel(pTVDispInfo->item.hItem);
			return;
		}

		SetItemText(pTVDispInfo->item.hItem, fromMBCS(_GetShowName(hNode)));
	}
}

BOOL CNodeTreeCtrl::UpdateShowName(NodeHandle hNode)
{
	if (!_Tree())
		return FALSE;
	std::string path=_Tree()->GetPath(hNode);
	HTREEITEM hItem=ItemFromPath(path);
	if (hItem==NULL)
		return FALSE;
	SetItemText(hItem, fromMBCS(_GetShowName(hNode)));
	return TRUE;
}


NodeHandle CNodeTreeCtrl::GetCurSel()
{
	if (!_Tree())
		return NULL;

	HTREEITEM hItem=GetFirstSelectedItem();
	if (!hItem)
		return NULL;
	if (GetNextSelectedItem(hItem))
		return NULL;//More than 1 is selected
	return _GetItemData(hItem);
}

NodeHandle* CNodeTreeCtrl::GetCurSels(DWORD &c)
{
	static std::vector<NodeHandle>handles;

	c=0;

	if (!_Tree())
		return NULL;


	ItemGroupDesc2 sel;
	_RecordSel(&sel);

	handles.resize(sel.total.size());
	for(int i=0;i<sel.total.size();i++)
		handles[i]=_GetItemData(sel.total[i]);

	c=handles.size();
	if (c<=0)
		return NULL;

	return handles.data();
}

//由于在执行SscOp时,可能会弹出一些对话框,从而导致在执行SscOp时有可能会调到IncUpdateSsc
//因为_ApplyItemSscOp(..)/_FlushSscOp(..)目前不能重入,所以我们用_bInSscOp来避免重入
void CNodeTreeCtrl::IncUpdateSsc()
{
	if (!_Tree())
		return;
	if ((!_ssc)||(!_ssc->IsConnected()))
		return;
	if (_bInSscOp)
		return;

	IFileSystem *pFS=NULL;
	if (_ssc)
		pFS=_ssc->GetFS();

	for (int i=0;i<30;i++)
	{
		HTREEITEM hItem=_GetNextSscUpdate();
		if (hItem)
		{
			NodeHandle hNode=_GetItemData(hItem);
			BOOL bFolder;
			std::string path=_Tree()->GetSscPath(hNode,bFolder);
			if (bFolder)
				continue;
			if(_GetItemSscState(hItem)==SSC_UNKNOWN)
			{
				SscOp op=Refresh;
				_pathesSsc.clear();
				_itemsSsc.clear();
				_ApplyItemSscOp(hItem,Refresh,FALSE);
				_FlushSscOp(Refresh);
				break;
			}
			if (pFS)
			{
				DWORD col;
				if (pFS->GetFileAttrAbs(path.c_str())==File_ReadOnly)
					col=RGB(128,128,128);
				else
					col=RGB(0,0,0);
				DWORD colOld=GetItemColor(hItem);
				if (colOld!=col)
					SetItemColor(hItem,col);
			}
		}
	}
}


LRESULT CNodeTreeCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message==WM_COMMAND)
	{
		DWORD id=LOWORD(wParam);
		if ((id>=ID_COMMON_NEW_START)&&(id<ID_COMMON_NEW_END))
		{
			OnNew((NodeType)(id-ID_COMMON_NEW_START));
			return 0;
		}

		if (TRUE)//ssc operation
		{
			SscOp op=None;
			switch(id)
			{
				case ID_SSC_CHECKIN:
				{
					op=CheckIn_;
					break;
				}
				case ID_SSC_CHECKIN_KEEPOUT:
				{
					op=CheckIn_KeepOut;
					break;
				}
				case ID_SSC_CHECKOUT:
				{
					op=CheckOut;
					break;
				}
				case ID_SSC_GET:
				{
					op=Get;
					break;
				}
				case ID_SSC_ADDTOVSS:
				{
					op=Add;
					break;
				}
				case ID_SSC_RMFROMVSS:
				{
					if (IDOK==AfxMessageBox(_T("将会从SourceSafe删除这个(些)文件,确认吗?"),MB_OKCANCEL))
						op=Remove;
					break;
				}
				case ID_SSC_REFRESH:
				{
					op=Refresh;
					break;
				}
				case ID_SSC_GETFOLDER:
				{
					op=GetFolder;
					break;
				}
			}

			if (op!=None)
			{
				if (!_Tree())
					return 0;

				if (_sel.total.size()<=0)
				{
					if (IDOK != AfxMessageBox(_T("将会对所有文件操作,确认吗?"), MB_OKCANCEL))
						return 0;
				}

				_OnSscOp(&_sel.total[0],_sel.total.size(),op,FALSE);

				return 0;
			}
		}
	}

	return CXTTreeCtrl::WindowProc(message, wParam, lParam);
}
//modify by star. 2007-11-6
//{{
BOOL  CNodeTreeCtrl::_CheckLegal(HTREEITEM  item,UINT state)// state 旧的状态
{
	//从选中状态到为选中状态，不需要改变父节点
	if(state==2)  
		return TRUE;
	  
	//从非选中状态 到选中状态时 确保父节点是被选中的
	HTREEITEM  parentItem=GetParentItem(item);
	if(!parentItem) 
		return TRUE;
	
	//设置父节点为选中状态
	UINT  parentState=GetItemState(parentItem,TVIS_STATEIMAGEMASK)>>12;
	if(parentState==1) 
	{
		// set check state to check
		SetItemState(parentItem,INDEXTOSTATEIMAGEMASK(2),TVIS_STATEIMAGEMASK);	
		//notify check state.
		_OnCheckStateChange(parentItem,2);

		NodeHandle  handle=_GetItemData(parentItem);
		if(_Tree())
			_Tree()->SetNodeCheck(handle,TRUE);
	}

	return   _CheckLegal(parentItem,parentState);
}
void CNodeTreeCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	if (!_Tree())
		return;

	UINT  flag;

	//////////////////////////////////////////////////////////////////////////
	//check the NodeTree whether supports check-box state. 

	if(_Tree()&&(!_SupportCheckBox()))  return;  
	//check the point occupied by which HTREEITEM.
	CPoint  point;  
	GetCursorPos(&point);
	ScreenToClient(&point);
	HTREEITEM  item=HitTest(point,&flag);
	if(item==NULL)  return;
//	SelectItem(item);  //select current item.
	//check, it is whether click on state icon.
	if(!(flag&(TVHT_ONITEMSTATEICON)))  return;
	
	UINT  state=GetItemState(item,TVIS_STATEIMAGEMASK)>>12;	
	UINT  lateState;
	lateState= (state==1)?2:1;

	if(_bRecursiveCheck&&!(_CheckLegal(item,state)))  
	{   //rollback to old check status.
		SetItemState(item,INDEXTOSTATEIMAGEMASK(lateState),TVIS_STATEIMAGEMASK);	
		return;
	}
//	//change the current node.
	_OnCheckStateChange(item,lateState);
	NodeHandle  handle=_GetItemData(item);
	BOOL  bChecked=(lateState==1)? FALSE :TRUE;

	if(_Tree())
		_Tree()->SetNodeCheck(handle,bChecked);
	//change the sub node
	if(_bRecursiveCheck)
		_TranslateItem(item,lateState);		
}
void  CNodeTreeCtrl::_OnCheckStateChange(HTREEITEM  item,UINT state)
{
		 // do some thing ,when the state change.
}
void CNodeTreeCtrl::_TranslateItem(HTREEITEM  item,UINT state)
{
		if(!ItemHasChildren(item))  return;
		HTREEITEM  hNextItem=GetChildItem(item);
		while(hNextItem)
		{
			_TranslateItem(hNextItem,state);
			UINT  curState=GetItemState(hNextItem,TVIS_STATEIMAGEMASK)>>12;
			if(curState!=state)
			{
				SetItemState(hNextItem,INDEXTOSTATEIMAGEMASK(state),TVIS_STATEIMAGEMASK);
				_OnCheckStateChange(hNextItem,state);
				NodeHandle  handle=_GetItemData(hNextItem);
				BOOL  bChecked=(state==1)? FALSE :TRUE;
				_Tree()->SetNodeCheck(handle,bChecked);
			}
			HTREEITEM  temp=GetNextItem(hNextItem,TVGN_NEXT);
			hNextItem=temp;
		}
}
//}
//////////////////////////////////////////////////////////////////////////

NodeHandle CNodeTreeCtrl::_GetItemData(HTREEITEM item)
{
	if (item==TVI_ROOT)
		return NodeHandle_Root;
	return (NodeHandle)GetItemData(item);
}


SscState CNodeTreeCtrl::_GetItemSscState(HTREEITEM item)
{
	if (item==TVI_ROOT)
		return SscState(SSC_NOTCONTROLLED);
	int ii;
	GetItemImage(item,ii,ii);
	NodeHandle hNode=_GetItemData(item);
	NodeType type=_Tree()->GetType(hNode);

	SscState states[]={		SSC_UNKNOWN,
										SSC_NOTCONTROLLED,
										SSC_NOTCHECKEDOUT,
										SSC_CHECKEDOUT,
										SSC_CHECKEDOUT_ME};

	for (int i=0;i<ARRAY_SIZE(states);i++)
	{
		if (_GetImageIdx(hNode,states[i])==ii)
			return states[i];
	}

	return SSC_NOTCONTROLLED;
}

SscState CNodeTreeCtrl::_GetActualItemSscState(HTREEITEM item)
{
	if (!_Tree())
		return SscState(SSC_UNKNOWN);
	if (!_Tree()->SupportSsc())
		return SscState(SSC_NOTCONTROLLED);
	if (item==TVI_ROOT)
		return SscState(SSC_NOTCONTROLLED);
	if (!_ssc)
		return SscState(SSC_UNKNOWN);

	NodeHandle hNode=_GetItemData(item);
	BOOL bFolder;
	const char *path=_Tree()->GetSscPath(hNode,bFolder);
	if (!path[0])
		return SSC_NOTCONTROLLED;

	SscState state;
	if (FALSE==_ssc->GetState(path,state))
		return SscState(SSC_UNKNOWN);

	return state;
}



//这个函数用来对某个item执行递归的Ssc操作
void CNodeTreeCtrl::_ApplyItemSscOp(HTREEITEM item,SscOp op,BOOL bTest)
{
	BOOL bOk=FALSE;

	SscState state=_GetItemSscState(item);//原来记录在item里的state
	NodeHandle hNode=(NodeHandle)_GetItemData(item);
	BOOL bFolder;
	std::string path=_Tree()->GetSscPath(hNode,bFolder);
	NodeType type=_Tree()->GetType(hNode);

	if (bFolder)
		path="";//忽略folder

	if (path!="")//是一个合法的文件名
	{
		if (state==SSC_UNKNOWN)
			bOk=TRUE;
		else
		{
			switch(op)
			{
				case CheckIn_:
				case CheckIn_KeepOut:
				{
					if (state==SSC_CHECKEDOUT_ME)
						bOk=TRUE;
					break;
				}
				case CheckOut:
				{
					if (state==SSC_NOTCHECKEDOUT)
						bOk=TRUE;
					break;
				}
				case Get:
				{
					if ((state!=SSC_NOTCONTROLLED)&&(state!=SSC_CHECKEDOUT_ME))
						bOk=TRUE;
					break;
				}
				case Add:
				{
					if (state==SSC_NOTCONTROLLED)
						bOk=TRUE;
					break;
				}
				case Remove:
				{
					if (state!=SSC_NOTCONTROLLED)
						bOk=TRUE;
					break;
				}
				case Refresh:
				{
					bOk=TRUE;
					break;
				}
			}
		}
	}

	if (bOk)
	{
		_pathesSsc.push_back(path);
		_itemsSsc.push_back(item);
	}

	if (bTest&&_pathesSsc.size()>0)
		return;

	TREEVIEW_BEGIN_RECURSIVE(this,itemChild,item);

		_ApplyItemSscOp(itemChild,op,bTest);
		if (bTest&&_pathesSsc.size()>0)
			return;

	TREEVIEW_END_RECURSIVE();

}

void CNodeTreeCtrl::_FlushSscOp(SscOp op)
{
	ProfilerStart_Recent(_FlushSscOp);
	std::vector<char*>buf;
	buf.resize(_pathesSsc.size());
	for (int i=0;i<_pathesSsc.size();i++)
		buf[i]=(char*)_pathesSsc[i].c_str();

	switch(op)
	{
		case CheckIn_:
		{
			ProfilerStart_Recent(_FlushSscOp_CheckIn);
			for (int i=0;i<buf.size();i++)
				_ssc->CheckIn(buf[i]);
			ProfilerEnd();
			break;
		}
		case CheckIn_KeepOut:
		{
			for (int i=0;i<buf.size();i++)
				_ssc->CheckIn(buf[i],131072);//VSSFLAG_KEEPYES
			break;
		}
		case CheckOut:
		{
			ProfilerStart_Recent(_FlushSscOp_CheckOut);
			_ssc->CheckOut((const char**)buf.data(),buf.size());
			ProfilerEnd();
			break;
		}
		case Get:
		case GetFolder:
		{
			_ssc->GetLatestVersion((const char**)buf.data(),buf.size());
			break;
		}
		case Add:
		{
			for (int i=0;i<buf.size();i++)
				_ssc->CheckIn(buf[i],131072);//VSSFLAG_KEEPYES
			break;
		}
		case Remove:
		{
			for (int i=0;i<buf.size();i++)
				_ssc->Delete(buf[i]);
			break;
		}
		case Refresh:
		{
			break;
		}
	}
	for (int i=0;i<buf.size();i++)
	{
		if (_itemsSsc[i]!=TVI_ROOT)
		{
			SscState stateNew;
			if (!_ssc->GetState(buf[i],stateNew))
				stateNew=SSC_UNKNOWN;
			NodeHandle hNode=(NodeHandle)_GetItemData(_itemsSsc[i]);
			DWORD ii=_GetImageIdx(hNode,stateNew);
			SetItemImage(_itemsSsc[i],ii,ii);
		}
	}

	ProfilerEnd();
}

void CNodeTreeCtrl::_ApplySscOp(HTREEITEM *items,DWORD nItems,SscOp op,BOOL bTest)
{
	if (!_ssc)
		return;

	_pathesSsc.clear();
	_itemsSsc.clear();

	for (int i=0;i<nItems;i++)
		_ApplyItemSscOp(items[i],op,bTest);

	if (!bTest)
		_FlushSscOp(op);
}

BOOL CNodeTreeCtrl::_TestSscOp(HTREEITEM *items,DWORD nItems,SscOp op)
{
	_OnSscOp(items,nItems,op,TRUE);
	if (_pathesSsc.size()>0)
		return TRUE;
	return FALSE;
}


BOOL CNodeTreeCtrl::_OnSscOp(HTREEITEM *items,DWORD nItems,SscOp op,BOOL bTest)
{
	_bInSscOp=TRUE;
	_pathesSscFolder.clear();

	HTREEITEM hRoot=TVI_ROOT;
	if (nItems<=0)
	{
		items=&hRoot;
		nItems=1;
	}

	//对于GetFolder操作来说,目录需要特殊处理
	if (op==GetFolder)
	{
		_pathesSsc.clear();
		_itemsSsc.clear();
		for (int i=0;i<nItems;i++)
		{
			NodeHandle hNode=_GetItemData(items[i]);
			BOOL bFolder;
			std::string path=_Tree()->GetSscPath(hNode,bFolder);
			if ((path!="")&&bFolder)
			{
				_pathesSsc.push_back(path);
				_itemsSsc.push_back(items[i]);
				continue;
			}
		}
		
		_pathesSscFolder=_pathesSsc;//把它记录下来

		if (!bTest)
			_FlushSscOp(op);

		_bInSscOp=FALSE;

		return TRUE;
	}

	_ApplySscOp(items,nItems,op,bTest);

	_bInSscOp=FALSE;

	return TRUE;
}


void CNodeTreeCtrl::_ClearMenu()
{
	for (int i=0;i<_menus.size();i++)
		SAFE_DELETE(_menus[i]);
	_menus.clear();
}

HTREEITEM CNodeTreeCtrl::_GetNextSscUpdate()
{
	DWORD c=CTreeCtrl::GetVisibleCount();
	if (c==0)
		return NULL;
	int idx=_iSscUpdate%(c+1);
	_iSscUpdate++;
	HTREEITEM hItem=CTreeCtrl::GetFirstVisibleItem();
	idx--;
	while(idx>0)
	{
		hItem=CTreeCtrl::GetNextVisibleItem(hItem);
		if (!hItem)
			return NULL;
		idx--;
	}
	return hItem;
}

const char *CNodeTreeCtrl::_GetShowName(NodeHandle hNode)
{
	return _Tree()->GetShowName(hNode);
}

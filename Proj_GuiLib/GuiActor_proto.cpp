/********************************************************************
	created:	2008/4/25   17:36
	file path:	d:\IxEngine\Proj_GuiLib
	author:		cxi
	
	purpose:	proto edit panel
*********************************************************************/
#include "stdh.h"
 
#include <vector>
#include <string> 

#include "resource.h"
 
#include "stringparser/stringparser.h"

#include "commondefines/general_stl.h"

#include "GuiActor_proto.h"

#include "GuiData_protologic.h" 
#include "GuiData_proto.h"
#include "GuiData_frameproxy.h"
#include "GuiData_debugger.h"
#include "GuiData_RichGrids.h"

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IRecords.h"
#include "RenderSystem/IBehaviorGraph.h"
#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/IEntitySystem.h"
#include "WorldSystem/stubparams/param_sys.h"

#include "WndBase.h"
#include "CommonCtrlBase.h"
#include "TreeCtrlBase.h"

#include "FileDialogBase.h"
#include ".\guiactor_prl.h"

#include "ProtoSelectDlg.h"

#include "GuiAgent_ShellTransform.h"
#include "GuiAgent_ShellModifier.h"

#include "GuiAgent_MatSet.h"

#include "stringparser/stringparser.h"
#include "records/recordsdefine.h"
#include "records/records.h"
#include "resdata/BehaviorGraphData.h"
#include "behaviorgraph/BgnHelper.h"

//需要和proto.cpp的宏定义保持一致
#define NODETYPE_PROTO 1
#define NODETYPE_SCRIPT 2
#define NODETYPE_DATA 3
#define NODETYPE_CLASS_BEGIN 2
#define NODETYPE_CLASS_END 5002
#define NODETYPE_ISCLASS(type) (((type)>=NODETYPE_CLASS_BEGIN)&&((type)<=NODETYPE_CLASS_END))


//////////////////////////////////////////////////////////////////////////
//CPNClipboard
CPNClipboard::~CPNClipboard()
{
	Clear();
	_poolNode.Reset(FALSE);
	_poolConn.Reset(FALSE);
	_poolStub.Reset(FALSE);
}

void CPNClipboard::Clear()
{
	for (int i=0;i<_dataes.size();i++)
		_poolNode.Free(_dataes[i]);
	_dataes.clear();
	for (int i=0;i<_connes.size();i++)
		_poolConn.Free(_connes[i]);
	_connes.clear();
	for (int i=0;i<_stubs.size();i++)
		_poolStub.Free(_stubs[i]);
	_stubs.clear();
}

void CPNClipboard::Add(ProtoNodeID id,const char *path,NodeType type,BYTE *data,DWORD szData,i_math::pos2di &pos)
{
	ProtoNodeData *t=_poolNode.Alloc();
	t->id=id;
	t->path=path;
	t->type=type;
	VEC_SET_BUFFER(t->data,data,szData);
	_dataes.push_back(t);
	t->pos=pos;
}

ProtoNodeData*CPNClipboard::Get(DWORD idx)
{
	if (idx>=_dataes.size())
		return NULL;
	return _dataes[idx];
}

//加入的connect的src和target node必须都存在于_dataes里
void CPNClipboard::SafeAddConn(PNConnect &conn)
{
	for (int i=0;i<2;i++)
	{
		int j;
		for (j=0;j<_dataes.size();j++)
		{
			if (conn.id[i]==_dataes[j]->id)
				break;
		}
		if (j>=_dataes.size())
			return;//conn中的某一个id不在_dataes中
	}

	PNConnData *p=_poolConn.Alloc();
	for (int i=0;i<2;i++)
	{
		p->id[i]=conn.id[i];
		p->name[i]=conn.name[i];
	}
	_connes.push_back(p);
}

void CPNClipboard::SafeAddStub(ProtoStubInfo&stub)
{
	for (int i=0;i<_dataes.size();i++)
	{
		if (_dataes[i]->id==stub.idInner)
		{
			PNStubData *p=_poolStub.Alloc();
			p->name=stub.name;
			p->nameInner=stub.nameInner;
			p->idInner=stub.idInner;
			p->pos=stub.pos-_dataes[i]->pos;
			_stubs.push_back(p);
			return;
		}
	}
}




//////////////////////////////////////////////////////////////////////////
//CProtoTree

#define ID_PROTONODE_DYNAMIC (ID_NODETREE_CUSTOM_START+0)
#define ID_PROTONODE_VIRTUAL (ID_NODETREE_CUSTOM_START+1)
#define ID_PROTONODE_LAB (ID_NODETREE_CUSTOM_START+2)
#define ID_PROTONODE_EDITHELPER (ID_NODETREE_CUSTOM_START+3)

#define ID_PROTONODE_INVDYNAMIC (ID_NODETREE_CUSTOM_START+6)
#define ID_PROTONODE_INVVIRTUAL (ID_NODETREE_CUSTOM_START+7)
#define ID_PROTONODE_INVLAB (ID_NODETREE_CUSTOM_START+8)
#define ID_PROTONODE_INVEDITHELPER (ID_NODETREE_CUSTOM_START+9)

#define ID_PROTONODE_COPY (ID_NODETREE_CUSTOM_START+13)
#define ID_PROTONODE_CUT (ID_NODETREE_CUSTOM_START+14)
#define ID_PROTONODE_PASTE (ID_NODETREE_CUSTOM_START+15)

#define ID_PROTONODE_DEFERRED_START (ID_NODETREE_CUSTOM_START+30)
#define ID_PROTONODE_DEFERRED_END (ID_NODETREE_CUSTOM_START+30+64)


BEGIN_MESSAGE_MAP(CProtoTree, CNodeTreeCtrl)
	ON_COMMAND(ID_PROTONODE_DYNAMIC,OnDynamic)
	ON_COMMAND(ID_PROTONODE_INVDYNAMIC,OnInvDynamic)
	ON_COMMAND(ID_PROTONODE_VIRTUAL,OnVirtual)
	ON_COMMAND(ID_PROTONODE_INVVIRTUAL,OnInvVirtual)
	ON_COMMAND(ID_PROTONODE_LAB,OnLab)
	ON_COMMAND(ID_PROTONODE_INVLAB,OnInvLab)
	ON_COMMAND(ID_PROTONODE_EDITHELPER,OnEditHelper)
	ON_COMMAND(ID_PROTONODE_INVEDITHELPER,OnInvEditHelper)

//	ON_COMMAND(ID_PROTONODE_DEFERRED,OnDeferred)
//	ON_COMMAND(ID_PROTONODE_INVDEFERRED,OnInvDeferred)
	ON_COMMAND_RANGE(ID_PROTONODE_DEFERRED_START, ID_PROTONODE_DEFERRED_END,OnDeferredGrp)

	ON_COMMAND(ID_PROTONODE_COPY,OnCopy)
	ON_COMMAND(ID_PROTONODE_CUT,OnCut)
	ON_COMMAND(ID_PROTONODE_PASTE,OnPaste)
END_MESSAGE_MAP()

CPNClipboard CProtoTree::_cbPN;


UINT CProtoTree::_GetImageID()
{
	return IDB_PROTOTREEICON;

}

DWORD CProtoTree::_GetImageIdx(NodeHandle hNode,SscState state)
{
	if (!_Tree())
		return 0;
	NodeType type=_Tree()->GetType(hNode);

	if (type==NODETYPE_PROTO)
		return 0;
	if (type==NODETYPE_SCRIPT)
		return 2;
	return 1;
}

void CProtoTree::OnNew(NodeType type)
{
	if (type==NODETYPE_PROTO)
	{
		static CProtoSelectDlg dlg;
		dlg.SetProtoLib(_lib);
		if (IDOK!=dlg.DoModal())
			return;

		if (std::string("")==dlg.GetSelPath())
			return;

		if (!_Tree())
			return;

		std::string nameNew=_GenNewName(NODETYPE_PROTO,"");
		CNodeTreeCtrl::_NewItem(type,nameNew.c_str(),(void*)dlg.GetSelPath());
		return;
	}
	CNodeTreeCtrl::OnNew(type);

}

std::string CProtoTree::GenNewAutoName()
{
    std::string s;
    s += PROTO_AUTONAME_PREFIX;
    s += "AA";
    return s;
}

std::string CProtoTree::GenUniqueAutoName()
{
    char buf[] = "$AA";
    buf[0] = PROTO_AUTONAME_PREFIX;
    buf[1] += rand() % 26;
    buf[2] += rand() % 26;

    return std::string(buf);
}


std::string CProtoTree::_GenNewName(NodeType type,const char *nameType)
{
	if (type==NODETYPE_SCRIPT)
		return std::string("script");

    return GenNewAutoName();
}

BOOL CProtoTree::_GenUniqueName(NodeType type,std::string &name)
{
	if (name.c_str()[0]!=PROTO_AUTONAME_PREFIX)
		return CNodeTreeCtrl::_GenUniqueName(type,name);

	name=GenUniqueAutoName();
	return TRUE;
}

void CProtoTree::NewProtoes(i_math::pos2di &ptStart,const char *pathes,BOOL bAssetOrProto)
{
	IProto *proto=_lib->ObtainProto(_protoid);
	if (!proto)
		return;
	if (!_Tree())
		return;
	std::vector<std::string>buf;
	SplitStringBy(",",std::string(pathes),&buf);

	i_math::pos2di ptBack=proto->GetNextNodePos();
	proto->SetNextNodePos(ptStart);

	HTREEITEM hParent=TVI_ROOT;
	NodeHandle hParentNode=NodeHandle_Root;
	NodeHandle hNode=NodeHandle_Null;

	if (!bAssetOrProto)
	{
		for (int i=0;i<buf.size();i++)
		{
			std::string name=_GenNewName(NODETYPE_PROTO,"");

			while(!_Tree()->CheckChildName(hParentNode,name.c_str()))
			{
				if (FALSE==_GenUniqueName(NODETYPE_PROTO,name))
					break;
			}

			hNode=_Tree()->AddChild(hParentNode,NODETYPE_PROTO,name.c_str(),(void*)buf[i].c_str());
			if (!hNode)
				continue;

			HTREEITEM hItem = InsertItem(_T(""), 0, 0, hParent);
			SetItemData(hItem,(DWORD_PTR)hNode);
			_UpdateItem(hItem,hNode);

			ptStart.x+=40;
			ptStart.y+=40;
			proto->SetNextNodePos(ptStart);
		}
	}
	else
	{
		std::unordered_map<std::string,NodeType>mp;
		if (TRUE)//建一个class name到NodeType的查询表
		{
			DWORD nTypes;
			NodeType *tps=_Tree()->GetChildType(NodeType_Root,nTypes);
			for (int i=0;i<nTypes;i++)
			{
				NodeTypeInfo *info=_Tree()->GetTypeInfo(tps[i]);
				mp[info->name]=info->type;
			}
		}

		for (int i=0;i<buf.size();i++)
		{
			std::unordered_map<std::string,NodeType>::iterator it=mp.find(buf[i]);
			if (it==mp.end())
				continue;

			NodeType tp=(*it).second;

			std::string name=_GenNewName(tp,"");
			while(!_Tree()->CheckChildName(hParentNode,name.c_str()))
			{
				if (FALSE==_GenUniqueName(tp,name))
					break;
			}

			hNode=_Tree()->AddChild(hParentNode,tp,name.c_str());
			if (!hNode)
				continue;

			HTREEITEM hItem = InsertItem(_T(""), 0, 0, hParent);
			SetItemData(hItem,(DWORD_PTR)hNode);
			_UpdateItem(hItem,hNode);

			ptStart.x+=40;
			ptStart.y+=40;
			proto->SetNextNodePos(ptStart);

		}
	}

	proto->SetNextNodePos(ptBack);
}

void CProtoTree::_ModifyEdit(NodeHandle hNode,std::string &str)
{
	BOOL bNeedGen=FALSE;
	if ((str.c_str()[0]==PROTO_AUTONAME_PREFIX)||(str.c_str()[0]==0))
		bNeedGen=TRUE;
	if (!bNeedGen)
		return;

	NodeType tp=_Tree()->GetType(hNode);
	if (tp==NODETYPE_SCRIPT)
		return;

	str=_GenNewName(tp,"");
	while(!_Tree()->CheckChildName(NodeHandle_Null,str.c_str()))
		_GenUniqueName(tp,str);

}



IProtoNode **CProtoTree::_GetSelProtoNodes(DWORD &count)
{
	count=0;
	_temp.clear();
	for (int i=0;i<_sel.total.size();i++)
	{
		NodeHandle hNode=(NodeHandle )GetItemData(_sel.total[i]);

		CNodeTree *ntree=GetNodeTree()->GetTree();
		if (ntree)
		{
			std::string path=ntree->GetPath(hNode);

			IProto *proto=_lib->ObtainProto(_protoid);
			if (proto)
			{
				IProtoNode *p=proto->FindNode(path.c_str());
				_temp.push_back(p);
			}
		}
	}
	count=_temp.size();
	if (count>0)
		return _temp.data();
	return NULL;
}


IProtoNode *CProtoTree::_GetSelProtoNode()
{
	DWORD count;
	IProtoNode **p=_GetSelProtoNodes(count);
	if (count!=1)
		return NULL;
	return p[0];
}



DWORD GetPNDeferGrpColor(PNDeferGrp grp)
{
	switch(grp)
	{
		case 1:	return RGB(255,0,0);
		case 2:	return RGB(255,255,0);
		case 3:	return RGB(0,0,255);
		case 4:	return RGB(0,255,255);
		case 5:	return RGB(255,0,255);
		case 6:	return RGB(0,255,0);
		case 7:	return RGB(0,164,255);
		case 8:	return RGB(128,0,64);
	}
	return 0;
}

class CDeferredGrpMenu:public CMenu
{
public:
	void MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
	{
		// all items are of fixed size
		lpMIS->itemWidth = 64;
	}

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS)
	{
		CDC* pDC = CDC::FromHandle(lpDIS->hDC);
		DWORD cr=GetPNDeferGrpColor(lpDIS->itemID-ID_PROTONODE_DEFERRED_START);

		CRect rc=lpDIS->rcItem;
		rc.left+=14;
		rc.DeflateRect(2,2,2,2);

		if (lpDIS->itemState & ODS_SELECTED)
		{
			// item has been selected - hilite frame
			COLORREF crHilite = RGB(49,106,197);
			CBrush br(crHilite);
			pDC->FillRect(&lpDIS->rcItem, &br);

		}

		if (!(lpDIS->itemState & ODS_SELECTED))
		{
			// Item has been de-selected -- remove frame
			CBrush br(RGB(255,255,255));
			pDC->FillRect(&lpDIS->rcItem, &br);
		}

		if (TRUE)
		{
			CBrush br(cr);
			pDC->FillRect(&rc, &br);
		}

		if ((lpDIS->itemState & ODS_CHECKED))
		{
			CPoint pt;
			pt.x=lpDIS->rcItem.left+3;
			pt.y=lpDIS->rcItem.top+6;
			CPen pen;
			pen.CreatePen(0,1,(COLORREF )((lpDIS->itemState & ODS_SELECTED)?RGB(255,255,255):0));
			CGdiObject *t=pDC->SelectObject(&pen);
			for (int i=0;i<3;i++)
			{
				pDC->MoveTo(pt+CPoint(0,0));
				pDC->LineTo(pt+CPoint(2,2));
				pDC->LineTo(pt+CPoint(7,-3));
				pt.y++;
			}
			pDC->SelectObject(t);
		}

	}
};


void CProtoTree::_ClearSubMenus()
{
	for (int i=0;i<_menuAllocs.size();i++)
		SAFE_DELETE(_menuAllocs[i]);
	_menuAllocs.clear();
}

void CProtoTree::_OnCustomMenu(CMenu *menu)
{
	_ClearSubMenus();
	BOOL bEditable=_IsEditable();
	if (bEditable)
	{
		BOOL bDyn=TRUE,bLab=TRUE,bVirtual=TRUE,bEditHelper=TRUE;
		PNDeferGrp grp=PNDeferGrp_None;
		DWORD nExist=0;
		DWORD nNodes;
		IProtoNode **nodes=_GetSelProtoNodes(nNodes);
		for (int i=0;i<nNodes;i++)
		{
			IProtoNode *node=nodes[i];
			if (!node)
				continue;
			if (node->GetType()==PN_LuaObj)
				continue;

			nExist++;
			if (!node->IsDynamic())
				bDyn=FALSE;
			if (!node->IsLab())
				bLab=FALSE;
			if (!node->IsVirtual())
				bVirtual=FALSE;
			if (!node->IsEditHelper())
				bEditHelper=FALSE;
			grp=node->GetDeferGrp();
		}
		if (nExist>0)
		{
			menu->AppendMenu(MF_SEPARATOR,0, _T(""));

			if (bVirtual)
				menu->AppendMenu(MF_STRING|MF_CHECKED,ID_PROTONODE_INVVIRTUAL, _T("Disable"));
			else
				menu->AppendMenu(MF_STRING,ID_PROTONODE_VIRTUAL, _T("Disable"));

			if (bDyn)
				menu->AppendMenu(MF_STRING|MF_CHECKED,ID_PROTONODE_INVDYNAMIC, _T("Dynamic"));
			else
				menu->AppendMenu(MF_STRING,ID_PROTONODE_DYNAMIC, _T("Dynamic"));


			if (TRUE)
			{
				CDeferredGrpMenu *menuSub=new CDeferredGrpMenu;
				menuSub->CreateMenu();
				menu->AppendMenu(MF_POPUP|MF_ENABLED|MF_STRING,(UINT_PTR)menuSub->m_hMenu, _T("Deferred Group"));

				menuSub->AppendMenu((grp==0)?MF_CHECKED:MF_BYCOMMAND,ID_PROTONODE_DEFERRED_START, _T("<None>"));
				for (int i=1;i<PNDeferGrp_Max;i++)
					menuSub->AppendMenu(grp==i?MF_CHECKED|MF_OWNERDRAW:MF_OWNERDRAW,ID_PROTONODE_DEFERRED_START+i, _T(""));

				_menuAllocs.push_back(menuSub);
			}
			if (bLab)
				menu->AppendMenu(MF_STRING|MF_CHECKED,ID_PROTONODE_INVLAB, _T("Lab"));
			else
				menu->AppendMenu(MF_STRING,ID_PROTONODE_LAB, _T("Lab"));

			if (bEditHelper)
				menu->AppendMenu(MF_STRING|MF_CHECKED,ID_PROTONODE_INVEDITHELPER, _T("Edit Helper"));
			else
				menu->AppendMenu(MF_STRING,ID_PROTONODE_EDITHELPER, _T("Edit Helper"));

		}
	}

	IProtoNode *node=_GetSelProtoNode();
	BOOL bCanCopy=(_sel.total.size()>0);
	BOOL bCanPaste=FALSE;
	if (_cbPN.GetCount()>0)
	{
		if (node)
		{
			if (node->GetType()!=PN_LuaObj)
				bCanPaste=TRUE;
		}
		else
		{
			if (_sel.total.size()==0)
				bCanPaste=TRUE;
		}
	}

	if (!bEditable)
		bCanPaste=FALSE;

	if ((bCanCopy)||(bCanPaste))
	{
		menu->AppendMenu(MF_SEPARATOR,0, _T(""));
		if (bCanCopy)
		{
			menu->AppendMenu(MF_STRING,ID_PROTONODE_COPY, _T("Copy"));
			if(bEditable)
				menu->AppendMenu(MF_STRING,ID_PROTONODE_CUT, _T("Cut"));
		}
		if (bCanPaste)
			menu->AppendMenu(MF_STRING,ID_PROTONODE_PASTE, _T("Paste"));
	}

}


#define SWITCH_NODETREE_FLAG(func,bFlag)									\
{																												\
	DWORD nNodes;																				\
	IProtoNode **nodes=_GetSelProtoNodes(nNodes);						\
	for (int i=0;i<nNodes;i++)																	\
	{																											\
		IProtoNode *node=nodes[i];															\
		if (!node)																							\
			continue;																						\
		if (node->GetType()==PN_LuaObj)												\
			continue;																						\
		node->func(bFlag);																		\
	}																											\
}


void CProtoTree::OnDynamic()
{
	SWITCH_NODETREE_FLAG(SetDynamic,TRUE);
}

void CProtoTree::OnInvDynamic()
{
	SWITCH_NODETREE_FLAG(SetDynamic,FALSE);
}

void CProtoTree::OnVirtual()
{
	SWITCH_NODETREE_FLAG(SetVirtual,TRUE);
}

void CProtoTree::OnInvVirtual()
{
	SWITCH_NODETREE_FLAG(SetVirtual,FALSE);
}

void CProtoTree::OnLab()
{
	SWITCH_NODETREE_FLAG(SetLab,TRUE);
}

void CProtoTree::OnInvLab()
{
	SWITCH_NODETREE_FLAG(SetLab,FALSE);
}

void CProtoTree::OnEditHelper()
{
	SWITCH_NODETREE_FLAG(SetEditHelper,TRUE);
}

void CProtoTree::OnInvEditHelper()
{
	SWITCH_NODETREE_FLAG(SetEditHelper,FALSE);
}


void CProtoTree::OnDeferredGrp(UINT idCmd)
{
	SWITCH_NODETREE_FLAG(SetDeferGrp,idCmd-ID_PROTONODE_DEFERRED_START);
}




void CProtoTree::Bind(ProtoID id)
{
	IProto *proto=NULL;
	if (_lib)
		proto=_lib->ObtainProto(id);
	if (!proto)
	{
		_protoid=id;
		_ver=0;
		SetNodeTree(NULL);
		return;
	}

	DWORD ver=0;
	if (proto)
		ver=proto->GetVer();

	BOOL bUpdate=FALSE;
	if (!_Tree())
		bUpdate=TRUE;
	else
	{
		if (_protoid==id)
		{
			if (ver!=_ver)
				bUpdate=TRUE;
			else
				return;
		}
	}

	if (bUpdate)
		UpdateNodeTree(proto->GetNodeTree());
	else
		SetNodeTree(proto->GetNodeTree());

	_protoid=id;
	_ver=ver;

}


void CProtoTree::OnCopy()
{
	CNodeTree *ntree=GetNodeTree()->GetTree();
	if (!ntree)
		return;
	IProto *proto=_lib->ObtainProto(_protoid);
	if (!proto)
		return;

	_cbPN.Clear();
	std::vector<BYTE>buf;

	for (int i=0;i<_sel.total.size();i++)
	{
		NodeHandle hNode=(NodeHandle )GetItemData(_sel.total[i]);

		std::string path=ntree->GetPath(hNode);
		NodeType type=ntree->GetType(hNode);

		IProtoNode *node=NULL;
		node=proto->FindNode(path.c_str());

		DP_BeginSave(dp,buf)
			node->SaveForCopy(dp);
		DP_EndSave();

		_cbPN.Add(node->GetID(),path.c_str(),type,buf.data(),buf.size(),node->GetGraphPos());
	}

	//Add the connection
	if (TRUE)
	{
		DWORD c=	proto->GetConnectCount();
		for (int i=0;i<c;i++)
		{
			PNConnect conn;
			if (FALSE==proto->GetConnect(i,conn))
				continue;
			_cbPN.SafeAddConn(conn);
		}
	}

	if (TRUE)
	{
		DWORD c=	proto->GetStubCount();
		for (int i=0;i<c;i++)
		{
			ProtoStubInfo stub;
			if (FALSE==proto->GetStubInfo(i,stub))
				continue;
			_cbPN.SafeAddStub(stub);
		}

	}

	//调整下位置
	if (TRUE)
	{
		i_math::recti rc;
		for (int i=0;i<_cbPN.GetCount();i++)
			rc.merge(_cbPN.Get(i)->pos);
		for (int i=0;i<_cbPN.GetCount();i++)
			_cbPN.Get(i)->pos-=rc.UpperLeftCorner;
	}


}

void CProtoTree::OnCut()
{
	OnCopy();
	if (_IsEditable())
		_DeleteSels();
}


void CProtoTree::OnPaste()
{
	if (!_IsEditable())
		return;

	if (_cbPN.GetCount()<=0)
		return;

	CNodeTree *tree=_Tree();
	if (!tree)
		return;

	IProto *proto=_lib->ObtainProto(_protoid);
	if (!proto)
		return;

	i_math::pos2di posStart=proto->GetNextNodePos();

	HTREEITEM hParent=TVI_ROOT;
	NodeHandle hParentNode=(void*)NodeHandle_Root;
	if (_sel.total.size()>0)
	{
		hParent=_sel.total[0];
		hParentNode=_GetItemData(hParent);
	}

	std::unordered_map<ProtoNodeID,ProtoNodeID>remap;
	for (int i=0;i<_cbPN.GetCount();i++)
	{
		ProtoNodeData *data=_cbPN.Get(i);
		std::vector<std::string>pieces;
		SplitStringBy(".",data->path,&pieces);
		if (pieces.size()<=0)
			continue;

		std::string name=pieces[pieces.size()-1];

		NodeHandle hNode=NodeHandle_Null;

		if(_CanAutoGenUniqueName(NODETYPE_SCRIPT))
		{
			while(!_Tree()->CheckChildName(hParentNode,name.c_str()))
			{
				if (FALSE==_GenUniqueName(NODETYPE_SCRIPT,name))
					break;
			}
		}

		hNode=tree->AddChild(hParentNode,data->type,name.c_str(),NULL);

		if (!hNode)
			continue;

		ProtoNodeID nodeid=proto->FindNodeID(tree->GetPath(hNode));
		proto->ReplaceProtoNode(nodeid,&data->data[0],data->data.size());

		remap[data->id]=nodeid;

		HTREEITEM hItem=InsertItem(_T(""),0,0,hParent);
		SetItemData(hItem,FORCE_TYPE(DWORD,hNode));
		_UpdateItem(hItem,hNode);

		EnsureVisible(hItem);

		//重新设置位置
		if (posStart!=i_math::pos2di(-10000,-10000))
		{
			IProtoNode *node=proto->GetNode(nodeid);
			if (node)
				node->SetGraphPos(posStart+data->pos);
		}
	}

	if (TRUE)//Now apply the connect
	{
		for (int i=0;i<_cbPN.GetConnCount();i++)
		{
			PNConnData *conn=_cbPN.GetConn(i);
			PNConnect t;
			for (int j=0;j<2;j++)
			{
				std::unordered_map<ProtoNodeID,ProtoNodeID>::iterator it=remap.find(conn->id[j]);
				if (it!=remap.end())
					t.id[j]=(*it).second;
				else
					t.id[j]=ProtoNodeID_Null;
				t.name[j]=conn->name[j].c_str();
			}
			proto->AddConnect(t);
		}
	}

	if (TRUE)//Now add the stubs
	{
		for (int i=0;i<_cbPN.GetStubCount();i++)
		{
			PNStubData *stub=_cbPN.GetStub(i);
			ProtoStubInfo info;

			std::unordered_map<ProtoNodeID,ProtoNodeID>::iterator it=remap.find(stub->idInner);
			if (it==remap.end())
				continue;
			ProtoNodeID nodeid=(*it).second;
			IProtoNode *node=proto->GetNode(nodeid);
			if (!node)
				continue;

			info.name=stub->name.c_str();
			info.idInner=nodeid;
			info.nameInner=stub->nameInner.c_str();

			i_math::pos2di pos=node->GetGraphPos();
			info.pos=pos+stub->pos;

			proto->AddStub(info);
		}
	}
}

//检查某个protonode是否有expose的stub
BOOL CheckProtoNodeExposed(IProtoNode *protonode)
{
	if (!protonode)
		return FALSE;
	IProto *proto=protonode->GetOwner();
	DWORD c=proto->GetStubCount();
	ProtoStubInfo info;
	for (int i=0;i<c;i++)
	{
		if (proto->GetStubInfo(i,info))
		{
			if (info.idInner==protonode->GetID())
				return TRUE;
		}
	}

	return FALSE;
}


BOOL CProtoTree::_PromptDelSel()
{
	DWORD c;
	IProtoNode **nodes=_GetSelProtoNodes(c);

	int i;
	for (i=0;i<c;i++)
	{
		if (CheckProtoNodeExposed(nodes[i]))
			break;
	}
	if (i<c)
	{
		if (IDYES!=AfxMessageBox(_T("这个(些)Node的某些接口被外连了,删除它(们)将会导致相关的外部连接失效,确认要继续吗?"),MB_YESNO))
			return FALSE;
	}

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CProtoNodePage
GuiLib_Api DWORD SeekAgentGUID(IEntity *en)
{
	IAsset *asts[512];
	DWORD c=en->EnumAllAsset(asts,sizeof(asts))	;

	BOOL bRepaired=FALSE;
	for (int i=0;i<c;i++)
	{
		IAsset *ast=asts[i];
		if (ast)
		{
			if (ast->SupportUID())
				return ast->GetUID();
		}
	}
	return 0;
}


RecordID SeekAgentRecordID(IProto *proto)
{
	DWORD c;
	GProperty **props=proto->EnumProps(c);
	for (int i=0;i<c;i++)
	{
		GProperty *prop=props[i];
		if (prop)
		{
			if(prop->GetClass()->CheckName("PropRef"))
			{
				PropRefTarget *r=((PropRef*)prop)->stuff;
				if (r)
				{
					if (r->GetClass()->CheckName("PropLosGeneralAgent"))
					{
						GObjBase *gobj=r->GetGObj();
						GElemBase *elem=gobj->GetElems();
						GObjBase *gobjLos;
						if (elem->GetObj(gobj->GetOwner(),&gobjLos))
						{
							elem=gobjLos->GetElems();

							void *var;
							if (std::string("_idRec")==elem->GetElemName())
							{
								if (elem->GetVar(gobjLos->GetOwner(),&var))
								{
									RecordID idRec=*(RecordID*)var;
									return idRec;
								}
							}
						}
						return RecordID_Invalid;
					}
				}
			}
		}
	}
	return RecordID_Invalid;
}

RecordID SeekAgentRecordID(IProto *proto,GProperty *propToFind)
{
	DWORD c;
	GProperty **props=proto->EnumAssetProps(c,"AstGeneralAgent");
	BOOL bFoundLop=FALSE;
	for (int i=c-1;i>=0;i--)
	{
		GProperty *prop=props[i];
		if (prop)
		{
			if (prop==propToFind)
				bFoundLop=TRUE;

			if(prop->GetClass()->CheckName("PropRef"))
			{
				PropRefTarget *r=((PropRef*)prop)->stuff;
				if (r)
				{
// 					if (r->GetClass()->CheckName("PropLopGeneralAgent"))
// 					{
// 						GObjBase *gobj=r->GetGObj();
// 						GElemBase *elem=gobj->GetElems();
// 						GObjBase *gobjLop;
// 						if (elem->GetObj(gobj->GetOwner(),&gobjLop))
// 						{
// 							if (gobjLopToFind==gobjLop)
// 								bFoundLop=TRUE;
// 						}
// 					}
					if (bFoundLop)
					{
						if (r->GetClass()->CheckName("PropLosGeneralAgent"))
						{
							GObjBase *gobj=r->GetGObj();
							GElemBase *elem=gobj->GetElems();
							GObjBase *gobjLos;
							if (elem->GetObj(gobj->GetOwner(),&gobjLos))
							{
								elem=gobjLos->GetElems();

								void *var;
								if (std::string("_idRec")==elem->GetElemName())
								{
									if (elem->GetVar(gobjLos->GetOwner(),&var))
									{
										RecordID idRec=*(RecordID*)var;
										return idRec;
									}
								}
							}
							return RecordID_Invalid;
						}
					}
				}
			}
		}
	}
	return RecordID_Invalid;
}


BOOL SeekRecordName(RecordID idRec,const char *nmRecords,std::string &nm)
{
	if (idRec!=RecordID_Invalid)
	{
		IRecords *records=(IRecords *)g_ssGuiLib.pRS->GetRecordsMgr()->ObtainRes(nmRecords);
		if (records)
		{
			CRecords *recs=records->GetRecords();
			if (recs)
			{
				GElemBase *elem=recs->FindElem("Name");
				if (elem)
				{
					CRecord *rec=recs->GetRecord(idRec);
					if (rec)
					{
						void *var;
						if (elem->GetVar(rec->GetGObj()->GetOwner(),&var))
						{
							nm=*(std::string *)var;
							SAFE_RELEASE(records);
							return TRUE;
						}
					}
				}
			}
		}
		SAFE_RELEASE(records);
	}
	return FALSE;
}

BOOL SeekAgentName(RecordID idRec,std::string &nm)
{
	return SeekRecordName(idRec,"agents.rcs",nm);
}

BOOL SeekMapName(RecordID idRec,std::string &nm)
{
	return SeekRecordName(idRec,"maps.rcs",nm);
}


BOOL SeekAgentName(IProto *proto,std::string &nm)
{
	RecordID idRec=SeekAgentRecordID(proto);
	return SeekAgentName(idRec,nm);
}

StringID SeekBehaviorGraphName(RecordID idRec)
{
	if (idRec!=RecordID_Invalid)
	{
		IRecords *records=(IRecords *)g_ssGuiLib.pRS->GetRecordsMgr()->ObtainRes("agents.rcs");
		if (records)
		{
			CRecords *recs=records->GetRecords();
			if (recs)
			{
				GElemBase *elem=recs->FindElem("idBG");
				if (elem)
				{
					CRecord *rec=recs->GetRecord(idRec);
					if (rec)
					{
						void *var;
						if (elem->GetVar(rec->GetGObj()->GetOwner(),&var))
						{
							StringID nm=*(StringID*)var;
							SAFE_RELEASE(records);
							return nm;
						}
					}
				}
			}
		}
		SAFE_RELEASE(records);
	}
	return StringID_Invalid;
}

StringID SeekBehaviorGraphName(IProto *proto)
{
	RecordID idRec=SeekAgentRecordID(proto);
	return SeekBehaviorGraphName(idRec);
}

StringID SeekBehaviorGraphName(IProto *proto,GProperty *propToFind)
{
	RecordID idRec=SeekAgentRecordID(proto,propToFind);
	return SeekBehaviorGraphName(idRec);
}

BEGIN_MESSAGE_MAP(CProtoNodePage, CGPropGrid)
END_MESSAGE_MAP()


void CProtoNodePage::Reset()
{
	CGObjGrid::Bind(NULL);
	Zero();
}

void CProtoNodePage::SetLib(IProtoLib *lib)
{		
	if (_lib==lib)
		return;

	_lib=lib;

	Bind(ProtoID_Null,ProtoNodeID_Null);
}

void CProtoNodePage::SetMgr(CGuiMgr *mgr)
{
	_mgr=mgr;
}


void CProtoNodePage::_Bind(BOOL bUpdate)
{

	RGState state;
	if (bUpdate)
	{
		CGObjGrid::RecordState(state);
		CGObjGrid::LockPaint();
	}

	if (TRUE)
	{
		CGObjGrid::ResetContent();

		if (_lib)
		{
			IProto *proto=_lib->ObtainProto(_id);
			if (proto)
			{
				IProtoNode *node=proto->GetNode(_nodeid);
				if (node)
				{

					BeginInsert();

					InsertCategory("Properties","");

					PushInsert();

					DWORD c=node->GetPropCount();
					for (int i=0;i<c;i++)
					{
						GProperty *prop=node->GetPropData(i);
						std::string name=node->GetPropName(i);
						GStubBase *stb=node->FindStub(name.c_str());
						std::string desc;
						GSem sem;
						if (stb)
						{
							sem=stb->sem;
							desc=stb->desc;
						}

						InsertProp(prop,name.c_str(),sem,desc.c_str());
					}

					PopInsert();

					EndInsert();
				}
			}
		}
	}

	if (bUpdate)
	{
		CGObjGrid::RestoreState(state);
		CGObjGrid::UnLockPaint();
	}
	else
	{
		if (!_RestoreNodeRGState())
			CGObjGrid::ExpandAll();
	}
}

void CProtoNodePage::Bind(ProtoID id,ProtoNodeID nodeid)
{
	_util.Init(g_ssGuiLib.pRS->GetPath(Path_BehaviorGraph),(BgpClasses*)g_ssGuiLib.pRS->GetBehaviorGraphMgr()->GetClasses());

	_RecordNodeRGState();

	BOOL bUpdate=FALSE;
	DWORD ver=0;
	if (_lib)
	{
		IProto *proto=_lib->ObtainProto(id);
		if (proto)
			ver=proto->GetVer();
	}

	if ((_nodeid==nodeid)&&(_id==id))
	{
		if (ver!=_ver)
			bUpdate=TRUE;
		else
			return;
	}

	for (int i=0;i<_cachesBhvValues.size();i++)
		_cachesBhvValues[i].Clear();
	_cachesBhvValues.clear();

	_nodeid=nodeid;
	_id=id;
	_ver=ver;
	_Bind(bUpdate);
}


BOOL CProtoNodePage::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle)
{
	if (!CGObjGrid::Create(rect,pParentWnd,nID,dwListStyle))
		return FALSE;

	ShowToolBar(FALSE);
	return TRUE;
}


int g_sss=0;

void CProtoNodePage::OnBeginItemChange(CXTPPropertyGridItem *item)
{
//	CGObjGrid::OnBeginItemChange(item);
	if (_mgr)
	{
		GuiData_Proto*dataProto=(GuiData_Proto*)_mgr->FindData("proto");
		if(dataProto)
			dataProto->bChanging=TRUE;
	}

}

void CProtoNodePage::OnItemChange(CXTPPropertyGridItem *item)
{
//	CGObjGrid::OnItemChange(item);

}

void CProtoNodePage::OnEndItemChange(CXTPPropertyGridItem *item)
{
//	CGObjGrid::OnEndItemChange(item);
	CGObjGrid::_RepairMtrlData(item);

	extern void BehaviorValueCache_PreSave(BhvValuesCache*cache);
	for (int i=0;i<_cachesBhvValues.size();i++)
		BehaviorValueCache_PreSave(&_cachesBhvValues[i]);

	IProto *proto=_lib->ObtainProto(_id);
	if (proto)
	{
		proto->IncVer();
		IProtoNode *node=proto->GetNode(_nodeid);
		if (node)
			node->IncVer();

		((CGuiPanel_Proto *)GetParent())->InvalidateView();
	}

	if (_mgr)
	{
		GuiData_Proto*dataProto=(GuiData_Proto*)_mgr->FindData("proto");
		if(dataProto)
			dataProto->bChanging=FALSE;
	}
}

void CProtoNodePage::RedrawDueToItemChange()
{
	((CGuiPanel_Proto *)GetParent())->InvalidateView();
}

void CProtoNodePage::_ClearNodeRGStates()
{
	std::unordered_map<ProtoID,ProtoRGState*>::iterator it;
	for (it=_rgstates.begin();it!=_rgstates.end();it++)
	{
		Safe_Class_Delete((*it).second);
	}
	_rgstates.clear();
}

NodeRGState *CProtoNodePage::_ObtainNodeRGState(ProtoID id,ProtoNodeID nodeid)
{
	std::unordered_map<ProtoID,ProtoRGState*>::iterator it;
	it=_rgstates.find(id);
	ProtoRGState *p=NULL;
	if(it!=_rgstates.end())
		p=((*it).second);
	else
	{
		p=Class_New2(ProtoRGState);
		_rgstates[id]=p;
	}
	return p->ObtainRGState(nodeid);
}


void CProtoNodePage::_RecordNodeRGState()
{
	if ((_id!=ProtoID_Null)&&(_nodeid!=ProtoNodeID_Null))
	{
		NodeRGState *p=_ObtainNodeRGState(_id,_nodeid);
		assert(p);

		RecordState(p->state);
	}
}

BOOL CProtoNodePage::_RestoreNodeRGState()
{
	if ((_id!=ProtoID_Null)&&(_nodeid!=ProtoNodeID_Null))
	{
		NodeRGState *p=_ObtainNodeRGState(_id,_nodeid);
		assert(p);

		if (p->state.empty())
			return FALSE;

		RestoreState(p->state);
		return TRUE;
	}
	return TRUE;
}

void BehaviorValueCache_PreLoad(BhvValuesCache *cache,BhvValues *values,CBehaviorGraphPads &pads,BOOL bSource,BOOL bParam)
{
	cache->values=values;

	std::vector<BhvConstDeclare *>declares;
	pads.EnumConstsDeclare(declares,bSource,bParam);

	cache->declares.resize(declares.size());
	for (int i=0;i<declares.size();i++)
		cache->declares[i]=declares[i];

	cache->LoadBind(pads);

}

void BehaviorValueCache_PreSave(BhvValuesCache*cache)
{
	cache->SaveBind();
}

void MakeItemColor(CXTPPropertyGridItem *item,DWORD col)
{
	if (!item)
		return;
	item->GetCaptionMetrics()->m_clrFore=col;
	item->GetValueMetrics()->m_clrFore=col;

	CXTPPropertyGridItems*items=item->GetChilds();
	for (int i=0;i<items->GetCount();i++)
		MakeItemColor(items->GetAt(i),col);
}

void BehaviorValueCache_InsertItems(CGObjGrid *grid,BhvValuesCache *cache)
{
	grid->InsertButtonItem("AI行为树参数","AI行为树参数",0);
	grid->PushInsert();

	cache->items.resize(cache->binds.size());
	for (int i=0;i<cache->binds.size();i++)
	{
		BhvValuesCache::Bind &bind=cache->binds[i];
		BhvValDeclare *declare=cache->declares[i];
		cache->items[i]=NULL;

		if (bind.nm==StringID_Invalid)
			continue;

		BOOL bDef=bind.bDef;

		CXTPPropertyGridItem *item=NULL;

		if (bind.nmRef==StringID_BhvValInvalidRef)
		{
			if (bind.elem)
			{
				std::unordered_map<std::string,std::string> overrides;
				grid->_BindElem(bind.pad->GetGObj(),bind.elem,StrLib_GetStr(bind.nm),overrides);
			}
		}

		cache->items[i]=item;

		if (item)
		{
			if (bDef)
				MakeItemColor(item,0x9f9f9f);
			else
				MakeItemColor(item,0);
		}
	}

	grid->PopInsert();
}


BOOL CProtoNodePage::_InsertElem(GObjBase *obj,GElemBase *elem)
{
	GObjBase *objSub;
	if (elem->GetObj(obj->GetOwner(),&objSub))
	{
		if (!(std::string("BhvValues")==objSub->GetName()))
			return FALSE;
	}
	else
		return FALSE;

	BOOL bSource=TRUE,bParam=TRUE;
	if (TRUE)
	{
		const char *name=obj->GetName();
		if (name[0]=='L')
		{
			if (name[1]=='o')
			{
				if (name[2]=='s')
					bParam=FALSE;
				if (name[2]=='p')
					bSource=FALSE;
			}
		}
	}

	IProto *proto=_lib->ObtainProto(_id);
	if (proto)
	{
		StringID nmBG=SeekBehaviorGraphName(proto);
		if (nmBG!=StringID_Invalid)
		{
			CBehaviorGraphPads pads;
			if (_util.LoadBGPads(nmBG,pads))
			{
				_cachesBhvValues.resize(_cachesBhvValues.size()+1);
				BhvValuesCache*cache=&_cachesBhvValues[_cachesBhvValues.size()-1];

				_util.ResolveBGPads(pads);
				BehaviorValueCache_PreLoad(cache,(BhvValues *)objSub->GetOwner(),pads,bSource,bParam);
				BehaviorValueCache_InsertItems(this,cache);
			}
// 			IBehaviorGraph *bg=(IBehaviorGraph*)g_ssGuiLib.pRS->GetBehaviorGraphMgr()->ObtainRes(nmBG);
// 			if (bg)
// 			{
// 				BehaviorGraphData *dataBg=bg->GetData();
// 				_cachesBhvValues.resize(_cachesBhvValues.size()+1);
// 				BhvValuesCache*cache=&_cachesBhvValues[_cachesBhvValues.size()-1];
// 
// 				BehaviorValueCache_PreLoad(cache,(BhvValues *)objSub->GetOwner(),dataBg->pads);
// 				BehaviorValueCache_InsertItems(this,cache);
// 			}
		}
	}

	return TRUE;
}




//////////////////////////////////////////////////////////////////////////
//CGuiActor_Proto
CGuiActor_Proto::CGuiActor_Proto()
{
	_panel=NULL;
	_nodeidSel=ProtoNodeID_Null;
	_controller=NULL;
	_matedit.AddRef();
	_tester.AddRef();
	_thumbnailmaker.AddRef();
}

CGuiActor_Proto::~CGuiActor_Proto()
{
	SAFE_DELETE(_controller);
}


void CGuiActor_Proto::Reset()
{
	CGuiView *view=(CGuiView *)FindView("proto_appear");
	DEFINE_GUIDATA_PROTO(dataProto);
	GuiData_ProtoLogic *dataLogic=(GuiData_ProtoLogic*)FindData("proto_logic");
	if (dataProto&&view)
	{   
		view->AttachActor(0,this);
		_controller=new CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>(dataProto->cam);
		_controller->AddRef();
		view->AddAgent(0,new CGuiAgent_ViewSwitcher);

		view->AddAgent(0,_controller);
		view->AddAgent(0,new CGuiAgent_CameraFov,AGENTPRIORITY_STANDARD+20);

		view->AddAgent(0,new CGuiAgent_OperatePN,AGENTPRIORITY_STANDARD+5);
		view->AddAgent(0,new CGuiAgent_ViewTimeCtrl,AGENTPRIORITY_STANDARD+40);

		if (TRUE)
		{
			CGuiAgent_MatSet *agent=new CGuiAgent_MatSet;
			agent->EnableRemote(FALSE);
			view->AddAgent(0,agent,AGENTPRIORITY_STANDARD+30);
		}

		view->AddAgent(0,&_tester,AGENTPRIORITY_STANDARD+20);
		view->AddAgent(0,&_thumbnailmaker,AGENTPRIORITY_STANDARD+19);

		if (TRUE)//the matrix editor agent
		{
			_matedit.SetWorkable(EditMode_All,TRUE);
			_matedit.ShowMoveToCamera(TRUE);
			_matedit.ShowResetPRS(TRUE);

			view->AddAgent(0,&_matedit,AGENTPRIORITY_STANDARD+12);
		}

		view->AddAgent(0,new CGuiAgent_ShellTransform());
		view->AddAgent(0,new CGuiAgent_ShellModifier(),AGENTPRIORITY_STANDARD+15);
	}

	view=(CGuiView *)FindView("proto_logic"); 
	if (view)
	{
		view->AttachActor(0,this);
		if (TRUE)
		{
			CGuiAgent_GraphScroll *t=new CGuiAgent_GraphScroll();
			view->SetTransformGG(i_math::pos2df((float)dataProto->xlate.x,(float)dataProto->xlate.y),i_math::pos2df(dataProto->scale,dataProto->scale));
			view->AddAgent(0,t);
		}
		view->AddAgent(0,new CGuiAgent_GraphNodeSel,AGENTPRIORITY_STANDARD+30);
		view->AddAgent(0,new CGuiAgent_GraphNodeRectSel,AGENTPRIORITY_STANDARD+40);
		view->AddAgent(0,new CGuiAgent_GraphNodeConnect,AGENTPRIORITY_STANDARD+20);
		view->AddAgent(0,new CGuiAgent_GraphNodeCommand,AGENTPRIORITY_STANDARD+20);
	}

	GuiData_RichGrids*dataRG=(GuiData_RichGrids*)FindData("richgrids");
	if (dataRG)
		dataRG->RegisterRichGrid("ProtoNodePage",_panel->GetPage());


}

CWnd *CGuiActor_Proto::GetWnd()	
{		
	return _panel->GetWnd();	
}


void CGuiActor_Proto::UpdateUI()
{
	DEFINE_GUIDATA_PROTO(dataProto);

	_matedit.UpdateBind();

	if (_controller)
	{
		i_math::matrix43f *mat=_matedit.GetBindMat();

		if (mat)
		{
			i_math::matrix43f matT=*mat;
			mat=_matedit.GetBindParentMat();
			if (mat)
				matT=matT*(*mat);
			_controller->SetFocusPos(_posFocus=matT.getTranslation());
		}
 		else
			_controller->SetFocusPos(_posFocus);
	}


}

void CGuiActor_Proto::OnLostPanel()
{
//	_tester.Stop();
}

void CGuiActor_Proto::OnOccupyPanel()
{

}

BOOL CGuiActor_Proto::BuildProtoTreeMenu(CMenu *menu)
{
	_panel->UpdateUI();
	return _panel->GetTree()->BuildContextMenu(menu);
}

void CGuiActor_Proto::SendProtoTreeCmd(DWORD idCmd)
{
	_panel->GetTree()->SendMessage(WM_COMMAND,MAKEWPARAM(idCmd,0),0);
}





//这个函数的功能是把当前的proto的数据取出来,将data里的数据载入到当前的proto,然后
//将之前取出来的数据存放到data里返回
BOOL CGuiActor_Proto::ReplaceProto(std::vector<BYTE>&data,BOOL bTest)
{
	DEFINE_GUIDATA_PROTO(dataProto);
	DEFINE_GUIDATA_DEBUGGER(dataDebugger);
	if (!dataProto)
		return FALSE;
	CPrlFrameProxy *proxy=((GuiData_PrlFrameProxy*)FindData("prlframeproxy"))->proxy;

	IProto *proto=dataProto->proto();
	if (!proto)
		return FALSE;
	if (dataDebugger->context->IsRunning())
		return FALSE;
	if (dataProto->IsReadOnly())
		return FALSE;

	if (bTest)
		return TRUE;


	std::vector<BYTE>buf;
	DP_BeginSave(dp,buf);
	proto->Save(dp);
	DP_EndSave();

	proto->Unload();
	CDataPacket dp;
	dp.SetDataBufferPointer(data.data());
	proto->Load(dp);

	proxy->UpdateLuaSrcToProto(proto);

	proto->Save();//存到硬盘上

	proto->RepairReferring();

	data=buf;

	return TRUE;
}


void CGuiActor_Proto::DoCommand(DWORD idCmd)
{
	if ((idCmd==GuiCmd_Undo)||(idCmd==GuiCmd_Redo))
	{
		DEFINE_GUIDATA_PROTO(dataProto);
		if (!dataProto)
			return;
		if (dataProto->IsReadOnly())
			return;
		DEFINE_GUIDATA_DEBUGGER(dataDebugger);
		if (!dataDebugger)
			return;
		if (dataDebugger->context->IsRunning())
			return;
	}

	return CGuiActor::DoCommand(idCmd);
}


//////////////////////////////////////////////////////////////////////////
//CMod_ReplaceProto
class CMod_ReplaceProto:public CModBase
{
public:
	virtual BOOL TestUndo()
	{
		return TestRedo();
	}
	virtual BOOL TestRedo()
	{
		return actor->ReplaceProto(data,TRUE);
	}

	virtual BOOL Undo()
	{
		return Redo();
	}
	virtual BOOL Redo()
	{
		if (FALSE==actor->ReplaceProto(data,FALSE))
			return FALSE;

		panel->InvalidateView();

		return TRUE;
	}
	virtual BOOL IsEmpty()	{		return data.size()<=0;	}


	std::vector<BYTE>data;

	CGuiActor_Proto *actor;
	CGuiPanel_Proto *panel;

};


//////////////////////////////////////////////////////////////////////////
//CGuiPanel_Proto

#define ID_TREE 40
#define ID_PAGE 41


BEGIN_MESSAGE_MAP(CGuiPanel_Proto, CGuiPanel)
	ON_WM_DESTROY()
	ON_WM_SIZE()

	ON_NOTIFY(TVN_SELCHANGED, ID_TREE, OnTvnSelchangedTree)
	ON_NOTIFY(NM_DBLCLK, ID_TREE, OnNMDblclkTree)
END_MESSAGE_MAP()


CGuiPanel_Proto::CGuiPanel_Proto(CWnd* pParent):CGuiPanel(IDD_EDITPANEL_PROTO, pParent)
{
	_actor=NULL;
	_bInUpdateTreeSel=FALSE;
}

BOOL CGuiPanel_Proto::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_EDITPANEL_PROTO,pParent);	
}
 

BOOL CGuiPanel_Proto::OnInitDialog()
{
	CGuiPanel::OnInitDialog();

	CRect rc;
	rc.SetRect(0,0,1,1);
	_tree.Create(this,rc,ID_TREE);
	_page.Create(rc,this,ID_PAGE);
	_page.SetWindowText(_T("ProtoNode属性"));

	_RecalcLayout();




	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CGuiPanel_Proto::_RecalcLayout()
{
	extern void SetWindowPos(CWnd *pWnd,i_math::recti &rc);

	i_math::recti rc;

	GetClientRect((LPRECT)&rc);

	if (TRUE)
	{
		const int gap=4;

		i_math::recti rc2,rc3;
		rc2=rc;
		rc2.cutout(1,300,rc3);//from top
		rc2.cutout(1,gap,rc);//from top

		SetWindowPos(&_tree,rc3);
		SetWindowPos(&_page,rc2);
	}
}


void CGuiPanel_Proto::OnDestroy()
{
	_tree.SetNodeTree(NULL);
	CGuiPanel::OnDestroy();

	// TODO: Add your message handler code here
}

CGuiMgr *CGuiPanel_Proto::_CurMgr()
{
	if (!_actor)
		return NULL;

	return (CGuiMgr *)_actor->GetMgr();
}



void CGuiPanel_Proto::OnSize(UINT nType, int cx, int cy)
{
	CGuiPanel::OnSize(nType, cx, cy);

	_RecalcLayout();
}


BOOL CGuiPanel_Proto::_UpdateTreeSel(HTREEITEM hItem,GuiData_Proto*data)
{
	BOOL bModified=FALSE;

	if (!_tree.GetNodeTree())
		return FALSE;
	CNodeTree *tree=_tree.GetNodeTree()->GetTree();
	if (!tree)
		return FALSE;

	if (hItem!=TVI_ROOT)
	{

		NodeHandle hNode=(NodeHandle )_tree.GetItemData(hItem);

		std::string path=tree->GetPath(hNode);
		ProtoNodeID id=ProtoNodeID_Null;
		if (data->proto())
			id=data->proto()->FindNodeID(path.c_str());

		BOOL bSelOld=_tree.IsSelected(hItem);
		BOOL bSelNew=FALSE;
		if (TRUE)
		{
			int idx;
			VEC_FIND(data->sels,id,idx);
			if (idx!=-1)
				bSelNew=TRUE;
		}

		if (bSelNew!=bSelOld)
		{
			bModified=TRUE;
			if (bSelNew)
			{
				_tree.SetItemState(hItem,TVIS_SELECTED,TVIS_SELECTED);
				extern void TreeCtrlEnsureItemExpanded(CTreeCtrl *ctrl,HTREEITEM hItem);
				TreeCtrlEnsureItemExpanded(&_tree,hItem);
			}
			else
				_tree.SetItemState(hItem,0,TVIS_SELECTED);
		}
	}

	TREEVIEW_BEGIN_RECURSIVE(&_tree,hChild,hItem)

		if (_UpdateTreeSel(hChild,data))
			bModified=TRUE;

	TREEVIEW_END_RECURSIVE()

	return bModified;
}


void CGuiPanel_Proto::UpdateUI()
{
	CGuiMgr *mgr=_CurMgr();
	GuiData_Proto*dataProto=NULL;
	if (mgr)
		dataProto=(GuiData_Proto*)mgr->FindData("proto");

	CPrlFrameProxy *proxy=NULL;
	if (mgr)
		proxy=((GuiData_PrlFrameProxy*)mgr->FindData("prlframeproxy"))->proxy;

	GuiData_Debugger*dataDebugger=NULL;
	if (mgr)
		dataDebugger=(GuiData_Debugger*)mgr->FindData("debugger");


	//检查是否只读
	BOOL bReadOnly=FALSE;
	if (dataProto)
	{
		if (dataProto->IsReadOnly())
			bReadOnly=TRUE;
	}
	if (dataDebugger)
	{
		if (dataDebugger->context->IsRunning())
			bReadOnly=TRUE;
	}

	//Flush the drops
	if ((!bReadOnly)&&mgr)
	{
		GuiData_ProtoLogic *dataLogic=(GuiData_ProtoLogic*)mgr->FindData("proto_logic");
		if (dataLogic)
		{
			if (!dataLogic->drops.empty())
			{
				_tree.NewProtoes(dataLogic->ptDrop,dataLogic->drops.c_str(),dataLogic->bAssetOrProto);
				dataLogic->drops="";

				mgr->InvalidateView("proto_logic");
			}
		}
	}


	//检查是否要存盘
	if (dataProto&&(!dataProto->bChanging))
	{
		IProto *proto=dataProto->proto();
		if (proto)
		{
			if (proto->GetModified()&&proto->IsLoaded())
			{
				if ((_actor->GetModMgr())&&(!dataDebugger->context->IsRunning()))
				{
					BYTE *data;
					DWORD cData;
					
					if (data=dataProto->lib->MakeUndoData(proto,cData))
					{
						CMod_ReplaceProto *mod=new CMod_ReplaceProto;

						mod->actor=_actor;
						mod->panel=this;
						VEC_SET_BUFFER(mod->data,data,cData);

						_actor->GetModMgr()->NewModGroup();
						_actor->GetModMgr()->PushBack(mod,FALSE);
					}
				}
				proto->Save();
			}
		}
	}

	//更新可编辑与否
	if (bReadOnly)
	{
//		_tree.EnableWindow(FALSE);
		_page.SetReadOnly(TRUE);
//		_page.EnableWindow(FALSE);
		if (proxy)
			proxy->EnableAllLuaSrc(FALSE);
	}
	else
	{
//		_tree.EnableWindow(TRUE);
		_page.SetReadOnly(FALSE);
		if (proxy)
			proxy->EnableAllLuaSrc(TRUE);
	}

	if (TRUE)//update the tree
	{
		if (dataProto)
		{
			_tree.SetLib(dataProto->lib);
			_tree.Bind(dataProto->protoid);
		}
		else
		{
			_tree.SetLib(NULL);
			_tree.Bind(ProtoID_Null);
		}
	}

	//update the selection in the tree
	if (dataProto)
	{
		_bInUpdateTreeSel=TRUE;

		if(_UpdateTreeSel(TVI_ROOT,dataProto))
			_tree.InvalidateRect(NULL);

		_bInUpdateTreeSel=FALSE;
	}

	//update the page content
	if (dataProto&&(!dataProto->bChanging))
	{
		ProtoID id=ProtoID_Null;
		ProtoNodeID nodeid=ProtoNodeID_Null;

		if (dataProto)
		{
			_page.SetLib(dataProto->lib);
			if (dataProto->proto())
			{
				id=dataProto->proto()->GetID();

				NodeHandle hSel=_tree.GetCurSel();
				if (hSel!=NodeHandle_Null)
				{
					CNodeTree *tree=_tree.GetNodeTree()->GetTree();
					if (tree)
					{
						std::string path=tree->GetPath(hSel);
						nodeid=dataProto->proto()->FindNodeID(path.c_str());
					}
				}
			}
		}
		else
			_page.SetLib(NULL);

		_page.Bind(id,nodeid);
	}

}


void CGuiPanel_Proto::OnNMDblclkTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	CGuiMgr *mgr=_CurMgr();

	if (!mgr)
		return;

	GuiData_Proto*dataProto=(GuiData_Proto*)mgr->FindData("proto");
	CPrlFrameProxy *proxy=((GuiData_PrlFrameProxy*)mgr->FindData("prlframeproxy"))->proxy;


	if (dataProto)//record the selections in the dataProto
	{
		if (dataProto->sels.size()==1)
		{
			IProto *proto=dataProto->proto();
			if (proto)
			{
				IProtoNode *node=proto->GetNode(dataProto->sels[0]);
				if (node)
				{
					if (node->GetType()==PN_LuaObj)
						proxy->GotoLuaSrc(dataProto->protoid,node->GetID(),0);
					if (node->GetType()==PN_Entity)
						proxy->GotoAppearance(node->GetProtoID());
				}
			}
		}
	}


}


void CGuiPanel_Proto::BindActor(CGuiActor_Proto *actor)
{
	if (_actor==actor)
		return;//no change

	if (_actor)
		_actor->OnLostPanel();
	_actor=actor;
	if (actor)
		_page.SetMgr((CGuiMgr*)actor->GetMgr());
	else
		_page.SetMgr(NULL);
	if (_actor)
		_actor->OnOccupyPanel();

	InvalidateView();

	EnableWindow(TRUE);
}

void CGuiPanel_Proto::OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;

	if (_bInUpdateTreeSel)
		return;

	if (_actor)
	{
		DWORD c;
		NodeHandle *handles=_tree.GetCurSels(c);

		CGuiMgr *mgr=_CurMgr();

		GuiData_Proto*dataProto=NULL;
		if (mgr)
			dataProto=(GuiData_Proto*)mgr->FindData("proto");

		if (dataProto)//record the selections in the dataProto
		{
			dataProto->sels.clear();
			CNodeTree *ntree=_tree.GetNodeTree()->GetTree();
			if ((ntree)&&(dataProto->proto()))
			{
				for (int i=0;i<c;i++)
				{
					std::string path=ntree->GetPath(handles[i]);

					ProtoNodeID id=dataProto->proto()->FindNodeID(path.c_str());
					if (id!=ProtoNodeID_Null)
						dataProto->sels.push_back(id);
				}
			}
		}
	}

	InvalidateView();
}

void CGuiPanel_Proto::InvalidateView()
{
	CGuiMgr *mgr=_CurMgr();

	if (!mgr)
		return;

	CGeView *view=mgr->FindView("proto_appear");
	view->Invalidate();
	view=mgr->FindView("proto_logic");
	view->Invalidate();
}

BOOL CGuiPanel_Proto::IsChangingProto()
{
	CGuiMgr *mgr=_CurMgr();
	GuiData_Proto*dataProto=NULL;
	if (mgr)
		dataProto=(GuiData_Proto*)mgr->FindData("proto");

	if (dataProto)
		return dataProto->bChanging;

	return FALSE;
}

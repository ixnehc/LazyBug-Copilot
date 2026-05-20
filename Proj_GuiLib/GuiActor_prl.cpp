/********************************************************************
	created:	2007/8/24   15:52
	filename: 	e:\IxEngine\Proj_GuiLib\WEditorPanel_Acl.cpp
	author:		cxi
	
	purpose:	asset class lib edit panel
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"
 
#include <vector>
#include <string>

#include "resource.h"

#include "WMGuiLib.h"

#include "stringparser/stringparser.h"

#include "GuiActor_prl.h"
#include "GuiData.h"

#include "GuiActor_proto.h"

#include "GuiData_frameproxy.h"

#include "WMGuiLib.h"


#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IEntitySystem.h"
#include "WorldSystem/IAssetShell.h"


#include "WndBase.h"
#include "CommonCtrlBase.h"

#include "GObjRefPath.h"

#include "FileDialogBase.h"
#include "TreeCtrlBase.h"
#include ".\guiactor_prl.h"

#include "RefResDlg.h"

#include "Log/LogDump.h"


//得到一个Proto引用的所有的资源路径
void EnumProtoRes(IProto *proto,std::vector<std::string>&buf)
{
	GProperty **props;
	DWORD c;
	props=proto->EnumProps(c);

	for (int i=0;i<c;i++)
	{
		GProperty *p=props[i];

		extern void GetGObjRefPath(GObjBase *obj,std::vector<std::string>*bufRes,std::vector<std::string>*bufTrrnBrLib,std::vector<std::string>*bufBrLib
			,std::vector<std::string>*bufMapFile,std::vector<unsigned __int64>*bufProto);
		GObjBase *gobj=p->GetGObj();
		if (gobj)
			GetGObjRefPath(gobj,&buf,NULL,NULL,NULL,NULL);
	}

}


//should be sychronized with the value in protolib.cpp
#define NODETYPE_FOLDER 1
#define NODETYPE_PROTO 2


#define ID_PROTO_SETASSTARTUP (ID_NODETREE_CUSTOM_START+3)
#define ID_PROTO_REFRESH (ID_NODETREE_CUSTOM_START+4)
#define ID_PROTO_COPYPATH (ID_NODETREE_CUSTOM_START+5)
#define ID_PROTO_REFRES (ID_NODETREE_CUSTOM_START+6)
#define ID_PROTO_CLEARSTARTUP (ID_NODETREE_CUSTOM_START+7)
#define ID_PROTO_COPYIDPATH (ID_NODETREE_CUSTOM_START+8)
#define ID_PROTO_BROWSEFOLDER (ID_NODETREE_CUSTOM_START+9)
#define ID_PROTO_MAKETHUMBNAIL (ID_NODETREE_CUSTOM_START+10)

//////////////////////////////////////////////////////////////////////////
//CPrlTree

BEGIN_MESSAGE_MAP(CPrlTree, CNodeTreeCtrl)
	ON_COMMAND(ID_PROTO_SETASSTARTUP,OnSetStartup)
	ON_COMMAND(ID_PROTO_CLEARSTARTUP,OnClearStartup)
	ON_COMMAND(ID_PROTO_REFRESH,OnRefresh)
	ON_COMMAND(ID_PROTO_COPYPATH,OnCopyPath)
	ON_COMMAND(ID_PROTO_COPYIDPATH,OnCopyIDPath)
	ON_COMMAND(ID_PROTO_REFRES,OnRefRes)
	ON_COMMAND(ID_PROTO_BROWSEFOLDER,OnBrowseFolder)
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnTvnBegindrag)  //star
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CPrlTree::SetLib(IProtoLib *lib)
{		
	_lib=lib;
}

void CPrlTree::SetES(IEntitySystem*pES)
{		
	_pES=pES;
	if (pES)
		_pWS=pES->GetWS();
	else
		_pWS=NULL;
}





UINT CPrlTree::_GetImageID()
{
	return IDB_PRLTREEICON;

}

DWORD CPrlTree::_GetImageIdx(NodeHandle hNode,SscState state)
{
	if (!_Tree())
		return 0;
	NodeType type=_Tree()->GetType(hNode);
	if (type==NODETYPE_FOLDER)
		return 0;
	if (_bEditable)
	{
		if (type==NODETYPE_PROTO)
		{
			std::string path=_Tree()->GetPath(hNode);
			RemoveFileSuffix(path);
			if (_lib->FindProto(path.c_str())==ProtoID_Null)
			{
				switch(state)
				{
					case SSC_NOTCONTROLLED:
						return 6;
					case SSC_NOTCHECKEDOUT:
						return 6;
					case SSC_CHECKEDOUT_ME:
						return 7;
					case SSC_CHECKEDOUT:
						return 8;
				}
			}

			switch(state)
			{
			case SSC_NOTCONTROLLED:
				return 1;
			case SSC_NOTCHECKEDOUT:
				return 4;
			case SSC_CHECKEDOUT_ME:
				return 3;
			case SSC_CHECKEDOUT:
				return 2;
			case SSC_UNKNOWN:
				return 5;
			}
		}
	}
	else
		return 1;
//	assert(FALSE);
	return 0;
}


BOOL CPrlTree::_IsEditable()
{
	if (!CNodeTreeCtrl::_IsEditable())
		return FALSE;
	if (_pES)
	{
		if (!_pES->IsEditMode())
			return FALSE;
	}
	return TRUE;
}

BOOL CPrlTree::_CanRename(NodeType type)
{
	if ((type==NODETYPE_PROTO)||(type==NODETYPE_FOLDER))
		return TRUE;
	return FALSE;
}

BOOL CPrlTree::_CanNew(NodeType type)
{
	if ((type==NODETYPE_PROTO)||(type==NODETYPE_FOLDER))
		return TRUE;
	return FALSE;
}

void CPrlTree::_ModifyEdit(NodeHandle hNode,std::string &str)
{
	if (_Tree())
	{
		NodeType type=_Tree()->GetType(hNode);
		if (type==NODETYPE_PROTO)
		{
			RemoveFileSuffix(str);
			MakeFileSuffix(str,"prt");
		}
	}
}

std::string CPrlTree::_GenNewName(NodeType type,const char *nameType)
{
	if (type==NODETYPE_FOLDER)
		return std::string("NewFolder");
	if (type==NODETYPE_PROTO)
		return std::string("NewProto.prt");
	return std::string("");
}


BOOL CPrlTree::_CanAutoGenUniqueName(NodeType type)
{
	return TRUE;
}

BOOL CPrlTree::_GenUniqueName(NodeType type,std::string &name)
{
	if (type!=NODETYPE_PROTO)
		return CNodeTreeCtrl::_GenUniqueName(type,name);
	RemoveFileSuffix(name);
	CNodeTreeCtrl::_GenUniqueName(type,name);
	MakeFileSuffix(name,"prt");
	return TRUE;
}


std::string CPrlTree::_GetProtoPath()
{
	std::string path;
	if ((_sel.total.size()==1)&&(_lib))
	{
		HTREEITEM hItem=_sel.total[0];
		path=PathFromItem(this,hItem,"\\");
		RemoveFileSuffix(path);

		std::string pathLib=_lib->GetLibFolderPath();

		path=pathLib+"\\"+path;

		std::string s=g_ssGuiLib.pWS->GetPath(WSPath_ProtoLib);

		path=CutHeadPath(path.c_str(),s.c_str());
	}

	return path;

}

std::string CPrlTree::_GetProtoIDPath()
{
	std::string pathProto=_GetProtoPath();
	ProtoID id=_lib->FindProto(pathProto.c_str());
	if (id==ProtoID_Null)
		return std::string("");
	std::string s;
	s=_lib->BuildIDPath(id,pathProto.c_str());
	return s;
}

void CPrlTree::_OnCustomMenu(CMenu *menu)
{
	if (!_Tree())
		return;

	if (_IsEditable())
	{
		menu->AppendMenu(MF_SEPARATOR, 0, _T(""));
		menu->AppendMenu(MF_STRING,ID_PROTO_REFRES, _T("引用资源..."));
	}

	if (_sel.total.size()>1)
		return;

	if (_IsEditable())
	{

		menu->AppendMenu(MF_SEPARATOR,0, _T(""));
		if (_sel.total.size()==1)
		{
			NodeHandle hNode=(NodeHandle )GetItemData(_sel.total[0]);
			NodeType typeParent=_Tree()->GetType(hNode);
			if (typeParent!=NODETYPE_FOLDER)
			{
				if (typeParent==NODETYPE_PROTO)
				{
					menu->AppendMenu(MF_STRING,ID_PROTO_SETASSTARTUP, _T("设为起始Proto"));
				}
			}
		}
		else
		{
			if (_sel.total.size()==0)
			{
				menu->AppendMenu(MF_STRING,ID_PROTO_CLEARSTARTUP, _T("清除起始Proto(使用客户端启动)"));
			}
		}

		menu->AppendMenu(MF_STRING,ID_PROTO_REFRESH, _T("Refresh"));
		menu->AppendMenu(MF_SEPARATOR,0, _T(""));
	}

	if (TRUE)
	{
		std::string path=_GetProtoPath();
		std::string idpath=_GetProtoIDPath();
		if ((path!="")||(idpath!=""))
		{
			menu->AppendMenu(MF_SEPARATOR, 0, _T(""));
			if (path!="")
			{
				path=std::string("复制字符串\"")+path+"\"";
				menu->AppendMenu(MF_STRING, ID_PROTO_COPYPATH, fromMBCS(path.c_str()));
			}
			if (idpath!="")
			{
				idpath=std::string("复制字符串\"")+idpath+"\"";
				menu->AppendMenu(MF_STRING,ID_PROTO_COPYIDPATH, fromMBCS(idpath.c_str()));
			}
			menu->AppendMenu(MF_SEPARATOR,0, _T(""));
		}

	}

	if (_sel.total.size()==1)
	{
		menu->AppendMenu(MF_SEPARATOR,0, _T(""));
		NodeHandle hNode=(NodeHandle )GetItemData(_sel.total[0]);
		NodeType type=_Tree()->GetType(hNode);
		if (type==NODETYPE_FOLDER)
			menu->AppendMenu(MF_STRING,ID_PROTO_BROWSEFOLDER, _T("打开目录位置"));
		if (type==NODETYPE_PROTO)
			menu->AppendMenu(MF_STRING,ID_PROTO_BROWSEFOLDER, _T("打开文件位置"));


// 		if (TRUE)
// 		{
// 			std::string pathProto=_GetProtoPath();
// 
// 			if (((CGuiPanel_Prl*)GetParent())->CanMakeThumbnail(pathProto.c_str()))
// 				menu->AppendMenu(MF_STRING,ID_PROTO_MAKETHUMBNAIL,"生成预览图");
// 		}
		


	}

}

void CPrlTree::OnSetStartup()
{
	if (!_Tree())
		return;

	NodeHandle hNode=GetCurSel();
	if (hNode!=NodeHandle_Null)
		((CGuiPanel_Prl*)GetParent())->SetStartupProto(_Tree()->GetPath(hNode));
}

void CPrlTree::OnClearStartup()
{
	((CGuiPanel_Prl*)GetParent())->SetStartupProto("");
}


void CPrlTree::OnRefresh()
{
	if (_lib)
	if (AfxMessageBox(_T("重新载入Proto Lib,确认吗?"),MB_YESNO)==IDYES)
	{
		_lib->Reload("");
		UpdateNodeTree(_lib->GetNodeTree());
	}

}

void CPrlTree::OnCopyPath()
{
	extern void CopyToClipboard(CWnd *wnd,const char *str);

	std::string path=_GetProtoPath();
	if (path=="")
		return;
	CopyToClipboard(this,path.c_str());
}

void CPrlTree::OnCopyIDPath()
{
	extern void CopyToClipboard(CWnd *wnd,const char *str);

	std::string idpath=_GetProtoIDPath();
	if (idpath=="")
		return;
	CopyToClipboard(this,idpath.c_str());
}

void CPrlTree::OnBrowseFolder()
{
	if (_sel.total.size()!=1)
		return;
	if (!_lib)
		return;

	std::string path=_GetProtoPath();
	std::string pathRoot=g_ssGuiLib.pWS->GetPath(WSPath_ProtoLib);
	path=pathRoot+"\\"+path;
	NodeHandle hNode=(NodeHandle )GetItemData(_sel.total[0]);
	NodeType type=_Tree()->GetType(hNode);
	if (type==NODETYPE_PROTO)
	{
		path=path+".prt";
		std::string arg="/select,";
		arg=arg+path;
		ShellExecute(NULL, _T("open"), _T("explorer.exe"), fromMBCS(arg.c_str()), NULL, SW_SHOWNORMAL);
	}
	else
	{
		std::string arg=path;
		ShellExecute( NULL, _T("open"), _T("explorer.exe"), fromMBCS(arg.c_str()), NULL, SW_SHOWNORMAL );
	}

}


void CPrlTree::OnRefRes()
{
	if (_sel.total.size()<=0)
		return;
	if (!_lib)
		return;

	std::vector<HTREEITEM>items;
	for (int i=0;i<_sel.total.size();i++)
		CollectItemList(this,_sel.total[i],items);

	std::vector<std::string>buf;
	std::string path;
	for (int i=0;i<items.size();i++)
	{
		HTREEITEM hItem=items[i];
		path=PathFromItem(this,hItem,"\\");
		RemoveFileSuffix(path);

		IProto *proto=_lib->ObtainProto(path.c_str());
		if (!proto)
			continue;

		EnumProtoRes(proto,buf);
		DWORD nRefs;
		ProtoID *refs=proto->EnumReference(nRefs);

		for (int i=0;i<nRefs;i++)
		{
			proto=_lib->ObtainProto(refs[i]);
			if (proto)
				EnumProtoRes(proto,buf);
		}

	}

	std::string pathResRoot=g_ssGuiLib.pRS->GetPath(Path_Res);
	pathResRoot+="\\";
	for (int i=0;i<buf.size();i++)
		buf[i]=pathResRoot+buf[i];

	CRefResDlg dlg;
	dlg.Set(buf);
	dlg.DoModal();

}





void CPrlTree::_ClearBold(HTREEITEM hParent,std::vector<HTREEITEM>&bolds)
{
	TREEVIEW_BEGIN_RECURSIVE(this,child,hParent)

		if (GetItemBold(child))
		{
			bolds.push_back(child);
			_ClearBold(child,bolds);
		}

	TREEVIEW_END_RECURSIVE();

}

void CPrlTree::UpdateStartup(const char *path)
{
	if (!_Tree())
		return;

	std::vector<HTREEITEM>boldsOld,boldsNew;

	//first,clear all the bold
	_ClearBold(TVI_ROOT,boldsOld);

	NodeHandle hNode=_Tree()->Find(path);
	if (hNode!=NodeHandle_Null)
	{
		HTREEITEM item=ItemFromNodeHandle(hNode);
		while(item&&(item!=TVI_ROOT))
		{
			boldsNew.push_back(item);
			item=GetParentItem(item);
		}
	}

	VEC_ASCEND(boldsNew,HTREEITEM);
	VEC_ASCEND(boldsOld,HTREEITEM);

	BOOL bSame;
	VEC_COMPARE(boldsNew,boldsOld,bSame);

	if (!bSame)
	{
		for (int i=0;i<boldsOld.size();i++)
			SetItemBold(boldsOld[i],FALSE);
		for (int i=0;i<boldsNew.size();i++)
			SetItemBold(boldsNew[i],TRUE);
	}

}
 
BOOL CPrlTree::_OnSscOp(HTREEITEM *items,DWORD c,SscOp op,BOOL bTest)
{
	if (bTest)
		return CNodeTreeCtrl::_OnSscOp(items,c,op,bTest);

	BOOL bRet=CNodeTreeCtrl::_OnSscOp(items,c,op,FALSE);

	std::string pathLib=_lib->GetLibFolderPath();

	if (_pathesSscFolder.size()>0)
	{

		for (int i=0;i<_pathesSscFolder.size();i++)
		{
			if (CheckPathContaining(pathLib.c_str(),_pathesSscFolder[i].c_str()))
			{
				std::string path=CutHeadPath(_pathesSscFolder[i].c_str(),pathLib.c_str());

				_lib->Reload(path.c_str());
			}
			else
			{
				if (pathLib==_pathesSscFolder[i])
					_lib->Reload("");
			}
		}

		UpdateNodeTree(_lib->GetNodeTree());
	}

	_lib->UnloadAllProto();
	if ((op==CNodeTreeCtrl::CheckOut)||(op==CNodeTreeCtrl::Get))
	{
		for (int i=0;i<_pathesSsc.size();i++)
		{
			if (CheckPathContaining(pathLib.c_str(),_pathesSsc[i].c_str()))
			{
				std::string path=CutHeadPath(_pathesSsc[i].c_str(),pathLib.c_str());
				RemoveFileSuffix(path);
				_lib->RepairProtoID(path.c_str());
			}
		}
	}

	_lib->SetModified();//强制为modified,这么做是为了发出LibModified的消息

	return bRet;
}

void CPrlTree::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	if (!_Tree())
		return;
	// TODO: Add your control notification handler code here

	NodeHandle hNode=GetCurSel();

	if (hNode!=NodeHandle_Null)
	{
		NodeType type=_Tree()->GetType(hNode);
		if (type==NODETYPE_PROTO)
		{
			BOOL bFolder;
			const char *path=_Tree()->GetSscPath(hNode,bFolder);
			if (_pWS->GetFS()->ExistFileAbs(path))
			{//该文件存在
				std::string s=_Tree()->GetPath(hNode);
				RemoveFileSuffix(s);
				path=s.c_str();
				NotifyOwner(GLM_PrlTree_DblClick,FORCE_TYPE(DWORD_PTR,path),0);
			}
			else
			{
				std::string ss;
				FormatString(ss,"无法找到文件\"%s\"!",path);
				AfxMessageBox(fromMBCS(ss.c_str()), MB_OK);
			}
		}
	}

}


void CPrlTree::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{		
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;

	_sDrag="";
	if (TRUE)
	{
		std::string path;
		CNodeTree *tree=_Tree();
		if (tree)
		{
			DWORD c;
			NodeHandle *handles=GetCurSels(c);
			for (int i=0;i<c;i++)
			{
				if (NODETYPE_PROTO==tree->GetType(handles[i]))
				{
					path=tree->GetPath(handles[i]);
					RemoveFileSuffix(path);
					if (!_sDrag.empty())
						_sDrag+=",";
					_sDrag+=path;
				}
			}
		}
	}
	if (!_sDrag.empty())
	{
		_bDrag=TRUE;
		::SetCapture(m_hWnd);
	}
}

void UpdateProtoDragCursor()
{
	POINT pt;
	GetCursorPos(&pt);
	CWnd *wnd=CWnd::WindowFromPoint(pt);
	BOOL bCanDrop=FALSE;
	if (wnd)
		bCanDrop=(BOOL)wnd->SendMessage(GLM_Proto_DragOver);
	HCURSOR h;
	extern HINSTANCE g_hInstance;
	if (bCanDrop)
		h=(HCURSOR)::LoadImage(g_hInstance,MAKEINTRESOURCE(IDC_POINTER_DRAGPROTO),
		IMAGE_CURSOR,0,0,LR_SHARED);
	else
		h=(HCURSOR)::LoadImage(g_hInstance,MAKEINTRESOURCE(IDC_NODROP),
		IMAGE_CURSOR,0,0,LR_SHARED);

	::SetCursor(h);
}


BOOL  CPrlTree::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (!_bDrag)
		return CNodeTreeCtrl::OnSetCursor(pWnd, nHitTest, message);

	UpdateProtoDragCursor();
	return 0;
}

void CPrlTree::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(!_bDrag)
	{
		CNodeTreeCtrl::OnLButtonUp(nFlags,point);
		return;
	}
	POINT pt;
	GetCursorPos(&pt);
	CWnd *wnd=CWnd::WindowFromPoint(pt);
	if (wnd)
		wnd->SendMessage(GLM_Proto_DragDrop,(WPARAM)_sDrag.c_str(),0);

	_bDrag=FALSE;
	ReleaseCapture();
}

void CPrlTree::OnMouseMove(UINT nFlags, CPoint point)
{		
	if(_bDrag) 
	{  
		UpdateProtoDragCursor();
	}
	CNodeTreeCtrl::OnMouseMove(nFlags,point);
}



//////////////////////////////////////////////////////////////////////////
//CGuiPanel_Prl

#define ID_TREE 40
#define ID_SYNC 50
#define ID_THUMBNAILLIST 60


BEGIN_MESSAGE_MAP(CGuiPanel_Prl, CGuiPanel)
	ON_WM_DESTROY()
	ON_WM_SIZE()

	ON_MESSAGE(GLM_PrlTree_DblClick,OnPrlTreeDblClk)

	ON_COMMAND(ID_SYNC,OnSync)
END_MESSAGE_MAP()


CGuiPanel_Prl::CGuiPanel_Prl(CWnd* pParent):CGuiPanel(IDD_EDITPANEL_PRL, pParent)
{
}

BOOL CGuiPanel_Prl::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_EDITPANEL_PRL,pParent);	
}
 

BOOL CGuiPanel_Prl::OnInitDialog()
{
	CGuiPanel::OnInitDialog();

	CRect rc;
	rc.SetRect(0,0,1,1);
	_tree.Create(this,rc,ID_TREE);
	_tree.SetOwner(m_hWnd);

	_btnSync.Create(_T(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rc, this, ID_SYNC);
	_btnSync.SetBitmap(CSize(16,16),IDB_SYNCPROTO);

	_RecalcLayout();




	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CGuiPanel_Prl::_RecalcLayout()
{
	extern void SetWindowPos(CWnd *pWnd,i_math::recti &rc);

	i_math::recti rc;

	GetClientRect((LPRECT)&rc);

	if (TRUE)
	{
		i_math::recti rc2,rc3;
		rc.cutout(1,24,rc2);//from top side

		rc2.cutout(2,24,rc3);
		rc2.inflate(-5,-5,-2,-2);
		rc3.inflate(-2,-2,-2,-2);

		SetWindowPos(GetDlgItem(IDC_INFO),rc2);

		SetWindowPos(GetDlgItem(ID_SYNC),rc3);
	}

	if (TRUE)
	{
		const int gap=4;

		i_math::recti rc2,rc3;
		rc2=rc;

		SetWindowPos(&_tree,rc2);
	}
}


void CGuiPanel_Prl::OnDestroy()
{
	_tree.SetNodeTree(NULL);
	CGuiPanel::OnDestroy();

	// TODO: Add your message handler code here
}

void CGuiPanel_Prl::Reset()
{
	EnableWindow(FALSE);
	GuiData_Prl *dataPrl=NULL;
	if (_mgr)
		dataPrl=(GuiData_Prl*)_mgr->FindData("protolib");
	if (dataPrl)
	{
		_tree.SetSsc(dataPrl->ssc);
		_tree.SetLib(dataPrl->lib);
		_tree.SetES(dataPrl->pES);

		_tree.SetNodeTree(dataPrl->lib->GetNodeTree());


		EnableWindow(TRUE);
	}
	else
	{
		_tree.SetSsc(NULL);
		_tree.SetLib(NULL);
		_tree.SetES(NULL);

		_tree.SetNodeTree(NULL);

		EnableWindow(FALSE);
	}

}


void CGuiPanel_Prl::OnSize(UINT nType, int cx, int cy)
{
	CGuiPanel::OnSize(nType, cx, cy);

	_RecalcLayout();
}

void CGuiPanel_Prl::UpdateUI()
{
	GuiData_Prl *dataPrl=(GuiData_Prl*)_mgr->FindData("protolib");
	if (!dataPrl)
		return;


	if (dataPrl->lib->GetNodeTree()!=_tree.GetNodeTree())
	{
		_tree.SetNodeTree(dataPrl->lib->GetNodeTree());
	}

 	_tree.UpdateStartup(dataPrl->pathStartupProto.c_str());
 	_tree.IncUpdateSsc();


	CNodeTree *ntree=dataPrl->lib->GetNodeTree()->GetTree();
	if (ntree)
	{
		if (ntree->IsModified())
		{
			ntree->ClearModified();
			GStubFireVoid(LibModified);
		}
	}

	if (TRUE)
	{
		std::string path;
		if (dataPrl->lib)
			path=dataPrl->lib->GetPath();
		SET_CONTROL_TEXT(this, IDC_INFO, fromMBCS(path.c_str()));
	}

}



LRESULT CGuiPanel_Prl::OnPrlTreeDblClk(WPARAM wParam,LPARAM lParam)
{

	GStubFireString(DblClickProto,(const char *)wParam);


	return 0;
}


void CGuiPanel_Prl::SetStartupProto(const char *path)
{
	GuiData_Prl *dataPrl=(GuiData_Prl*)_mgr->FindData("protolib");
	if (!dataPrl)
		return;

	dataPrl->pathStartupProto=path;

}

void CGuiPanel_Prl::EnsureVisible(ProtoID idProto)
{
	GuiData_Prl *dataPrl=(GuiData_Prl*)_mgr->FindData("protolib");
	if (!dataPrl)
		return;

	std::string path=dataPrl->lib->FindPath(idProto);
	MakeFileSuffix(path,"prt");

	HTREEITEM hItem=_tree.ItemFromPath(path);
	if ((hItem!=NULL)&&(hItem!=TVI_ROOT))
	{
		_tree.EnsureVisible(hItem);
		_tree.SelectAll(FALSE);
		_tree.SelectItem(hItem);
	}

}


void CGuiPanel_Prl::OnSync()
{
	GuiData_PrlFrameProxy*dataProxy=(GuiData_PrlFrameProxy*)_mgr->FindData("prlframeproxy");
	if (!dataProxy)
		return;

	ProtoID id=dataProxy->proxy->GetActiveProto();
	if (id==ProtoID_Null)
		return;

	EnsureVisible(id);
}

BOOL CGuiPanel_Prl::CanMakeThumbnail(const char *pathProto)
{
	GuiData_Prl *dataPrl=(GuiData_Prl*)_mgr->FindData("protolib");
	if (!dataPrl)
		return FALSE;
	GuiData_PrlFrameProxy*dataProxy=(GuiData_PrlFrameProxy*)_mgr->FindData("prlframeproxy");
	if (!dataProxy)
		return FALSE;

	ProtoID idProto=dataProxy->proxy->GetActiveProto();
	if (idProto==ProtoID_Null)
		return FALSE;
	std::string path=dataPrl->lib->FindPath(idProto);
	if (path==pathProto)
		return TRUE;
	return FALSE;
}

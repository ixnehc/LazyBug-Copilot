/********************************************************************
	created:	2007/8/24   15:52
	filename: 	e:\IxEngine\Proj_GuiLib\WEditorPanel_Acl.cpp
	author:		cxi
	
	purpose:	asset lib edit panel
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

#include "WMGuiLib.h"


#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IEntitySystem.h"
#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/assetpackage/IAssetPackage.h"

#include "WndBase.h"
#include "CommonCtrlBase.h"

#include "FileDialogBase.h"
#include "TreeCtrlBase.h"
#include ".\guiactor_astlib.h"

#include "Log/LogDump.h"


//should be sychronized with the value in protolib.cpp
#define NODETYPE_PACKAGE 1
#define NODETYPE_ASSET 2


//////////////////////////////////////////////////////////////////////////
//CAssetLibTree

BEGIN_MESSAGE_MAP(CAssetLibTree, CNodeTreeCtrl)
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnTvnBegindrag)  //star
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()



UINT CAssetLibTree::_GetImageID()
{
	return IDB_ASSETLIBICON;

}

DWORD CAssetLibTree::_GetImageIdx(NodeHandle hNode,SscState state)
{
	if (!_Tree())
		return 0;
	NodeType type=_Tree()->GetType(hNode);
	if (type==NODETYPE_PACKAGE)
		return 0;
	if (type==NODETYPE_ASSET)
		return 1;
	return 0;
}

void CAssetLibTree::_OnInitType()
{
	CNodeTree::_AddType(NODETYPE_PACKAGE,"Package");

	//Native Types
	CNodeTree::_AddType(NODETYPE_ASSET,"Asset");

}

BOOL CAssetLibTree::_OnCheckTypeRelation(NodeType typeParent,NodeType typeChild)//return whether typeChild could be under typeParent
{
	if ((typeParent==NodeType_Root)&&(typeChild==NODETYPE_PACKAGE))
		return TRUE;

	if ((typeParent==NODETYPE_PACKAGE)&&(typeChild==NODETYPE_ASSET))
		return TRUE;

	return FALSE;
}

NodePtr CAssetLibTree::_OnNew(const char *path,NodeType type,void *param)
{
	return NodePtr_Null;
}
BOOL CAssetLibTree::_OnDelete(NodeHandle hNode)
{
	return FALSE;
}


const char*CAssetLibTree::_GetShowName(NodeHandle hNode,const char *nameOrg)
{
	return nameOrg;
}


void CAssetLibTree::SetContent(IAssetSystem *pAS)
{
	_pAS=pAS;

	std::map<std::string,int>categories;
	std::vector<std::string>pathes;
	_clsses.clear();
	IAssetPackage *p=pAS->GetAPs();
	while(p)
	{
		DWORD c;
		const char **names;
		names=p->GetClassNames(c);
		std::string s,s2,nm;
		for (int i=0;i<c;i++)
		{
			nm=names[i];
			CClass *clss=p->GetClass(nm.c_str());
			IAsset *ast=(IAsset*)clss->New();
			s=ast->GetCategory();
			s2=ast->GetShowName();
			Class_Delete(ast);
			categories[s]=1;
			if (!s2.empty())
				nm=s2;
			s=s+"."+nm;
			pathes.push_back(s);

			_clsses[nm]=clss;

		}

		p=p->GetNext();
	}
	VEC_ASCEND(pathes,std::string);

	if (TRUE)
	{
		std::map<std::string,int>::iterator it;
		for (it=categories.begin();it!=categories.end();it++)
			CNodeTree::AddNode((*it).first.c_str(),NODETYPE_PACKAGE,NodePtr_Null);
	}

	if (TRUE)
	{
		for(int i=0;i<pathes.size();i++)
			CNodeTree::AddNode(pathes[i].c_str(),NODETYPE_ASSET,NodePtr_Null);
	}


	CNodeTreeCtrl::SetNodeTree(CNodeTree::ObtainRef());
}

const char *CAssetLibTree::GetCurSelHelp()
{
	if (!_pAS)
		return "";
	NodeHandle hNode=CNodeTreeCtrl::GetCurSel();
	if (hNode==NodeHandle_Null)
		return "n/a";
	if (CNodeTree::GetType(hNode)!=NODETYPE_ASSET)
		return "n/a";

	std::string nm=CNodeTree::GetName(hNode);

	std::unordered_map<std::string,CClass *>::iterator it=_clsses.find(nm);
	if (it==_clsses.end())
		return "n/a";

	CClass *clss=(*it).second;
	IAsset *ast=(IAsset *)clss->New();
	static std::string ret;
	ret=ast->GetHelp();
	Safe_Class_Delete(ast);
	if (ret.empty())
		return "n/a,yet";
	return ret.c_str();
}




void CAssetLibTree::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	if (!_Tree())
		return;

}


void CAssetLibTree::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{		
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;

	_sDrag="";
	if (TRUE)
	{
		std::string nm;
		if(TRUE)
		{
			DWORD c;
			NodeHandle *handles=GetCurSels(c);
			std::unordered_map<std::string,CClass *>::iterator it;
			for (int i=0;i<c;i++)
			{
				if (NODETYPE_ASSET==CNodeTree::GetType(handles[i]))
				{
					nm=CNodeTree::GetName(handles[i]);
					it=_clsses.find(nm);
					if (it==_clsses.end())
						continue;
					CClass *clss=(*it).second;
					nm=clss->GetName();
						
					if (!_sDrag.empty())
						_sDrag+=",";
					_sDrag+=nm;
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


BOOL  CAssetLibTree::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (!_bDrag)
		return CNodeTreeCtrl::OnSetCursor(pWnd, nHitTest, message);

	extern void UpdateProtoDragCursor();
	UpdateProtoDragCursor();
	return 0;
}

void CAssetLibTree::OnLButtonUp(UINT nFlags, CPoint point)
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
		wnd->SendMessage(GLM_Proto_DragDrop,(WPARAM)_sDrag.c_str(),1);

	_bDrag=FALSE;
	ReleaseCapture();
}

void CAssetLibTree::OnMouseMove(UINT nFlags, CPoint point)
{		
	if(_bDrag) 
	{  
		extern void UpdateProtoDragCursor();
		UpdateProtoDragCursor();
	}
	CNodeTreeCtrl::OnMouseMove(nFlags,point);
}



//////////////////////////////////////////////////////////////////////////
//CGuiPanel_AssetLib

#define ID_TREE 40


BEGIN_MESSAGE_MAP(CGuiPanel_AssetLib, CGuiPanel)
	ON_WM_DESTROY()
	ON_WM_SIZE()

END_MESSAGE_MAP()


CGuiPanel_AssetLib::CGuiPanel_AssetLib(CWnd* pParent):CGuiPanel(IDD_EDITPANEL_PRL, pParent)
{
}

BOOL CGuiPanel_AssetLib::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_EDITPANEL_ASSETLIB,pParent);	
}
 

BOOL CGuiPanel_AssetLib::OnInitDialog()
{
	CGuiPanel::OnInitDialog();

	CRect rc;
	rc.SetRect(0,0,1,1);
	_tree.Create(this,rc,ID_TREE);
	_tree.SetOwner(m_hWnd);

	_RecalcLayout();




	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CGuiPanel_AssetLib::_RecalcLayout()
{
	extern void SetWindowPos(CWnd *pWnd,i_math::recti &rc);

	i_math::recti rc;

	GetClientRect((LPRECT)&rc);

	if (TRUE)
	{
		i_math::recti rc2;
		rc.cutout(3,120,rc2);//from bottom side

		rc2.inflate(-2,-2,-2,-2);
		rc.inflate(-2,-2,-2,-2);

		SetWindowPos(GetDlgItem(IDC_DOCUMENT),rc2);
		SetWindowPos(&_tree,rc);
	}

}


void CGuiPanel_AssetLib::OnDestroy()
{
	_tree.ResetContent();
	_tree.SetNodeTree(NULL);
	CGuiPanel::OnDestroy();

	// TODO: Add your message handler code here
}

void CGuiPanel_AssetLib::Reset()
{
	GuiData_Prl *dataPrl=NULL;
	if (_mgr)
		dataPrl=(GuiData_Prl*)_mgr->FindData("protolib");
	if (dataPrl)
		_tree.SetContent(dataPrl->pES->GetAS());

}


void CGuiPanel_AssetLib::OnSize(UINT nType, int cx, int cy)
{
	CGuiPanel::OnSize(nType, cx, cy);

	_RecalcLayout();
}

void CGuiPanel_AssetLib::UpdateUI()
{
	const char *help=_tree.GetCurSelHelp();

	SET_CONTROL_TEXT(this, IDC_DOCUMENT, fromMBCS(help));
}



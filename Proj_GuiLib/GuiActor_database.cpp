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

#include "GuiData_database.h"
#include "GuiActor_database.h"

#include "WMGuiLib.h"


#include "WndBase.h"
#include "CommonCtrlBase.h"

#include "GObjRefPath.h"

#include "FileDialogBase.h"
#include "TreeCtrlBase.h"

#include "Log/LogDump.h"



// 节点类型定义
#define NODETYPE_FOLDER 1
#define NODETYPE_FILE 2

//////////////////////////////////////////////////////////////////////////
//CDbTreeCtrl

BEGIN_MESSAGE_MAP(CDbTreeCtrl, CNodeTreeCtrl)
 	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
// 	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnTvnBegindrag)  //star
// 	ON_WM_SETCURSOR()
// 	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

CDbTreeCtrl::CDbTreeCtrl()
{		
	_pDB = nullptr;
	_requestOpenSelTime = 0;
}

CDbTreeCtrl::~CDbTreeCtrl()
{
}

UINT CDbTreeCtrl::_GetImageID()
{
	return IDB_PRLTREEICON;

}

DWORD CDbTreeCtrl::_GetImageIdx(NodeHandle hNode,SscState state)
{
	if (!_Tree())
		return 0;
	NodeType type=_Tree()->GetType(hNode);
	if (type==NODETYPE_FOLDER)
		return 0;
		return 1;
}


BOOL CDbTreeCtrl::_IsEditable()
{
			return FALSE;
}

BOOL CDbTreeCtrl::_CanRename(NodeType type)
{
	return FALSE;
}

BOOL CDbTreeCtrl::_CanNew(NodeType type)
{
	return FALSE;
}

// void CDbTreeCtrl::_OnCustomMenu(CMenu *menu)
// {
// 	if (!_Tree())
// 		return;
// 
// 
// 	if (_sel.total.size()>1)
// 		return;
// 
// 	menu->AppendMenu(MF_SEPARATOR,0, _T(""));
// 
// 	menu->AppendMenu(MF_STRING,ID_PROTO_REFRESH, _T("Refresh"));
// 	menu->AppendMenu(MF_SEPARATOR,0, _T(""));
// 
// 	if (_sel.total.size()==1)
// 	{
// 		menu->AppendMenu(MF_SEPARATOR,0, _T(""));
// 		NodeHandle hNode=(NodeHandle )GetItemData(_sel.total[0]);
// 		NodeType type=_Tree()->GetType(hNode);
// 		if (type==NODETYPE_FILE)
// 			menu->AppendMenu(MF_STRING,ID_PROTO_BROWSEFOLDER, _T("打开文件位置"));
// 
// 	}
// 
// }
// 
// 
// void CDbTreeCtrl::OnRefresh()
// {
// 	if (_lib)
// 	if (AfxMessageBox(_T("重新载入Proto Lib,确认吗?"),MB_YESNO)==IDYES)
// 	{
// 		_lib->Reload("");
// 		UpdateNodeTree(_lib->GetNodeTree());
// 	}
// 
// }
// 
// void CDbTreeCtrl::OnCopyPath()
// {
// 	extern void CopyToClipboard(CWnd *wnd,const char *str);
// 
// 	std::string path=_GetProtoPath();
// 	if (path=="")
// 		return;
// 	CopyToClipboard(this,path.c_str());
// }
// 
// void CDbTreeCtrl::OnCopyIDPath()
// {
// 	extern void CopyToClipboard(CWnd *wnd,const char *str);
// 
// 	std::string idpath=_GetProtoIDPath();
// 	if (idpath=="")
// 		return;
// 	CopyToClipboard(this,idpath.c_str());
// }
// 
// void CDbTreeCtrl::OnBrowseFolder()
// {
// // 	if (_sel.total.size()!=1)
// // 		return;
// // 	if (!_lib)
// // 		return;
// // 
// // 	std::string path=_GetProtoPath();
// // 	std::string pathRoot=g_ssGuiLib.pWS->GetPath(WSPath_ProtoLib);
// // 	path=pathRoot+"\\"+path;
// // 	NodeHandle hNode=(NodeHandle )GetItemData(_sel.total[0]);
// // 	NodeType type=_Tree()->GetType(hNode);
// // 	if (type==NODETYPE_FILE)
// // 	{
// // 		path=path+".prt";
// // 		std::string arg="/select,";
// // 		arg=arg+path;
// // 		ShellExecute(NULL, _T("open"), _T("explorer.exe"), fromMBCS(arg.c_str()), NULL, SW_SHOWNORMAL);
// // 	}
// 
// }
// 
// 
void CDbTreeCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	if (!_Tree())
		return;
	// TODO: Add your control notification handler code here

	NodeHandle hNode=GetCurSel();
	if (hNode!=NodeHandle_Null)
	{
		NodeType type=_Tree()->GetType(hNode);
		if (type==NODETYPE_FILE)
		{
			_requestOpenSelTime = GetAbsTick();
		}
	}

}
 
bool CDbTreeCtrl::EnsureVisible(const char* fullPath)
{
	if (!_pDB)
		return false;

	if (!CheckPathContaining(_pDB->_setting.pathWorkspace.c_str(), fullPath))
		return false;

	std::string localPath = CutHeadPath(fullPath, _pDB->_setting.pathWorkspace.c_str());

	std::string virtualPath = _pDB->_entries.GetVirtualPath(localPath);

	NodeHandle handle = Find(virtualPath.c_str());
	if (handle == NodeHandle_Null)
		return false;

	HTREEITEM hItem = ItemFromNodeHandle(handle);
	if ((hItem != NULL) && (hItem != TVI_ROOT))
	{
		CNodeTreeCtrl::EnsureVisible(hItem);
		SelectAll(FALSE);
		SelectItem(hItem);
		return true;
	}

	return false;
}


void CDbTreeCtrl::SetDatabase(CVcxprojDatabaseCore* pDB)
{
	if (!pDB)
	{
		_pDB = nullptr;
		return;
	}
	
	_pDB = pDB;
	
	// 初始化节点树
	ResetContent();
	_OnInitType();
	
	// 获取所有条目
	const std::unordered_map<std::string, CVcxprojEntries::Entry>& entries = _pDB->_entries._entries;
	
	// 第一步:创建所有目录节点
	for (const std::pair<const std::string, CVcxprojEntries::Entry>& entry : entries)
	{
		// 获取目录路径
		std::string dirPath = GetFileFolderPath(entry.second.pathVirtual);
		
		// 如果目录为空,跳过
		if (dirPath.empty())
		{
			continue;
		}
		
		// 分割目录路径
		std::vector<std::string> dirParts;
		SplitStringBy("\\", dirPath, &dirParts);
		
		// 逐级创建目录节点
		std::string currentPath;
		for (size_t i = 0; i < dirParts.size(); i++)
		{
			if (i > 0)
			{
				currentPath += "\\";
			}
			currentPath += dirParts[i];
			
			// 创建目录节点
			AddNode(currentPath.c_str(), NODETYPE_FOLDER, NodePtr_Null);
		}
	}
	
	// 第二步:创建所有文件节点
	for (const std::pair<const std::string, CVcxprojEntries::Entry>& entry : entries)
	{
		// 创建文件节点
		AddNode(entry.second.pathVirtual.c_str(), NODETYPE_FILE, NodePtr_Null);
	}
	
	// 设置树控件显示
	SetNodeTree(CNodeTree::ObtainRef());
}

BOOL CDbTreeCtrl::_OnCheckTypeRelation(NodeType typeParent, NodeType typeChild)
{
	// 文件夹可以包含文件和子文件夹
	if (typeParent == NODETYPE_FOLDER)
	{
		return (typeChild == NODETYPE_FOLDER || typeChild == NODETYPE_FILE);
	}
	return FALSE;
}

const char* CDbTreeCtrl::_GetSep()
{
	return "\\";
}

void CDbTreeCtrl::_OnInitType()
{
	_AddType(NODETYPE_FOLDER, "目录");
	_AddType(NODETYPE_FILE, "文件");
}

NodePtr CDbTreeCtrl::_OnNew(const char* path, NodeType type, void* param)
{
	return NodePtr_Null;
}

BOOL CDbTreeCtrl::_OnDelete(NodeHandle hNode)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiPanel_Db

#define ID_TREE 40


BEGIN_MESSAGE_MAP(CGuiPanel_Db, CGuiPanel)
	ON_WM_DESTROY()
	ON_WM_SIZE()

	ON_MESSAGE(GLM_DbTree_DblClick,OnDbTreeDblClk)

END_MESSAGE_MAP()


CGuiPanel_Db::CGuiPanel_Db(CWnd* pParent):CGuiPanel(IDD_EDITPANEL_PRL, pParent)
{
}

BOOL CGuiPanel_Db::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_EDITPANEL_PRL,pParent);	
}
 

BOOL CGuiPanel_Db::OnInitDialog()
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

void CGuiPanel_Db::_RecalcLayout()
{
	extern void SetWindowPos(CWnd *pWnd,i_math::recti &rc);

	i_math::recti rc;

	GetClientRect((LPRECT)&rc);

	if (TRUE)
	{
		const int gap=4;

		i_math::recti rc2,rc3;
		rc2=rc;

		SetWindowPos(&_tree,rc2);
	}
}


void CGuiPanel_Db::OnDestroy()
{
	_tree.SetNodeTree(NULL);
	CGuiPanel::OnDestroy();

	// TODO: Add your message handler code here
}

void CGuiPanel_Db::Reset()
{
	EnableWindow(FALSE);
	GuiData_Database *dataDB=NULL;
	if (_mgr)
		dataDB =(GuiData_Database*)_mgr->FindData("database");
	if (dataDB)
	{
		_tree.SetDatabase(dataDB->db);

		EnableWindow(TRUE);
	}
	else
	{
		_tree.SetNodeTree(NULL);

		EnableWindow(FALSE);
	}

}


void CGuiPanel_Db::OnSize(UINT nType, int cx, int cy)
{
	CGuiPanel::OnSize(nType, cx, cy);

	_RecalcLayout();
}

void CGuiPanel_Db::UpdateUI()
{
	GuiData_Database* dataDB = NULL;
	if (_mgr)
		dataDB = (GuiData_Database*)_mgr->FindData("database");
	if (!dataDB)
		return;

// 
// 	if (dataPrl->lib->GetNodeTree()!=_tree.GetNodeTree())
// 	{
// 		_tree.SetNodeTree(dataPrl->lib->GetNodeTree());
// 	}

 	_tree.IncUpdateSsc();


// 	CNodeTree *ntree=dataPrl->lib->GetNodeTree()->GetTree();
// 	if (ntree)
// 	{
// 		if (ntree->IsModified())
// 		{
// 			ntree->ClearModified();
// 			GStubFireVoid(LibModified);
// 		}
// 	}

// 	if (TRUE)
// 	{
// 		std::string path;
// 		if (dataPrl->lib)
// 			path=dataPrl->lib->GetPath();
// 		SET_CONTROL_TEXT(this, IDC_INFO, fromMBCS(path.c_str()));
// 	}

}


LRESULT CGuiPanel_Db::OnDbTreeDblClk(WPARAM wParam,LPARAM lParam)
{

	GStubFireString(DblClickProto,(const char *)wParam);


	return 0;
}


void CGuiPanel_Db::EnsureVisible(ProtoID idProto)
{
// 	GuiData_Prl *dataPrl=(GuiData_Prl*)_mgr->FindData("protolib");
// 	if (!dataPrl)
// 		return;
// 
// 	std::string path=dataPrl->lib->FindPath(idProto);
// 	MakeFileSuffix(path,"prt");
// 
// 	HTREEITEM hItem=_tree.ItemFromPath(path);
// 	if ((hItem!=NULL)&&(hItem!=TVI_ROOT))
// 	{
// 		_tree.EnsureVisible(hItem);
// 		_tree.SelectAll(FALSE);
// 		_tree.SelectItem(hItem);
// 	}

}

const char *CGuiPanel_Db::GetCurSelPath()
{
    if (!_tree.GetSafeHwnd())
    {
        return "";
    }

    NodeHandle hNode = _tree.GetCurSel();
    if (hNode == NodeHandle_Null)
    {
        return "";
    }

    // 获取节点类型
	if (!_tree.GetNodeTree())
		return "";
	CNodeTree* tree = _tree.GetNodeTree()->GetTree();
	if (!tree)
		return "";
    NodeType type = tree->GetType(hNode);
    if (type != NODETYPE_FILE)
    {
        return "";
    }

    // 获取节点的虚拟路径
    return tree->GetPath(hNode);
}



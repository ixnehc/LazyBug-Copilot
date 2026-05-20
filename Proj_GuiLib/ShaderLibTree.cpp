#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IUtilRS.h"


#include "interface/interface.h"

#include "assert.h"

#include "ShaderLibGlobal.h"

#include "ShaderLibTree.h"

#include "stringparser/stringparser.h"

#include "TreeCtrlBase.h"

#include "WMGuiLib.h"



//////////////////////////////////////////////////////////////////////////
//CShaderLibTree
BEGIN_MESSAGE_MAP(CShaderLibTree, CNodeTreeCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
END_MESSAGE_MAP()


CShaderLibTree::CShaderLibTree()
{
	_global=NULL;
}

CShaderLibTree::~CShaderLibTree()
{
}

int CShaderLibTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CNodeTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

//	CNodeTreeCtrl::SetSsc(g_ssGuiLib.ssc);


	return 0;
}


void CShaderLibTree::OnDestroy()
{

	Clear();
	CNodeTreeCtrl::OnDestroy();
}


void CShaderLibTree::Clear()
{
	CNodeTree::ResetContent();
	CNodeTreeCtrl::SetNodeTree(NULL);
}



void CShaderLibTree::SetContent(CShaderLibGlobal *global)
{
	CNodeTree::ResetContent();

	_global=global;

	CNodeTree::_LoadTypes();

	for (int i=0;i<_global->_libs.size();i++)
	{
		CShaderLib2 *lib=_global->_libs[i];
		CNodeTree::AddNode(lib->_nm.c_str(),SHADERLIB_NODE_LIB,NodePtr_Null);
	}

	CNodeTree::AddNode("Features",SHADERLIB_NODE_FEATUREFOLDER,NodePtr_Null);

	for (int i=0;i<_global->_features.size();i++)
	{
		CShaderFeature*feature=_global->_features[i];
		std::string s;
		s="Features\\";
		s+=feature->_nm;
		CNodeTree::AddNode(s.c_str(),SHADERLIB_NODE_FEATURE,NodePtr_Null);
	}

	CNodeTreeCtrl::SetNodeTree(CNodeTree::ObtainRef());
}

void CShaderLibTree::_OnInitType()
{
	CNodeTree::_AddType(SHADERLIB_NODE_LIB,"ShaderLib");
	CNodeTree::_AddType(SHADERLIB_NODE_LIBFEATURE,"Feature");
	CNodeTree::_AddType(SHADERLIB_NODE_FEATUREFOLDER,"FeatureFolder");
	CNodeTree::_AddType(SHADERLIB_NODE_FEATURE,"Feature");
}

BOOL CShaderLibTree::_OnCheckTypeRelation(NodeType typeParent,NodeType typeChild)
{
	if (typeParent==NodeType_Root)
	{
		if (typeChild==SHADERLIB_NODE_LIB)
			return TRUE;
		if (typeChild==SHADERLIB_NODE_FEATUREFOLDER)
			return TRUE;
	}

	if (typeParent==SHADERLIB_NODE_LIB)
	{
		if (typeChild==SHADERLIB_NODE_LIBFEATURE)
			return TRUE;
	}
	if (typeParent==SHADERLIB_NODE_FEATUREFOLDER)
	{
		if (typeChild==SHADERLIB_NODE_FEATURE)
			return TRUE;
	}

	return FALSE;
}

const char *CShaderLibTree::_GetSep()
{
	return "\\";
}

NodePtr CShaderLibTree::_OnNew(const char *path,NodeType type,void *param)
{

	NodePtr ret=NodePtr_Null;

	if (type==SHADERLIB_NODE_LIB)
	{
		std::string nm=GetFileTitle(std::string(path));
		if (!_global->AddLib(nm.c_str()))
			return NodePtr_Invalid;
	}

	if (type==SHADERLIB_NODE_FEATURE)
	{
		std::string nm=GetFileTitle(std::string(path));
		if (!_global->AddFeature(nm.c_str()))
			return NodePtr_Invalid;
	}

	return ret;
}

BOOL CShaderLibTree::_OnDelete(NodeHandle hNode)
{
	const char *path=CNodeTree::GetPath(hNode);
	NodeType type=CNodeTree::GetType(hNode);

	std::string nm=GetFileTitle(std::string(path));

	if (type==SHADERLIB_NODE_LIB)
		return _global->DeleteLib(nm.c_str());
	if (type==SHADERLIB_NODE_FEATURE)
		return _global->DeleteFeature(nm.c_str());

	return FALSE;
}

BOOL CShaderLibTree::_OnRename(NodeHandle hNode,const char *nameNew)
{
	const char *path=CNodeTree::GetPath(hNode);
	NodeType type=CNodeTree::GetType(hNode);

	std::string nm=GetFileTitle(std::string(path));
	if (nm==nameNew)
		return TRUE;

	if (type==SHADERLIB_NODE_LIB)
		return _global->RenameLib(nm.c_str(),nameNew);
	if (type==SHADERLIB_NODE_FEATURE)
		return _global->RenameFeature(nm.c_str(),nameNew);

	return FALSE;
}

std::string CShaderLibTree::_GenNewName(NodeType type,const char *nameType)
{
	if (type==SHADERLIB_NODE_LIB)
		return std::string("ShaderLib");
	if (type==SHADERLIB_NODE_FEATURE)
		return std::string("Feature");
	return std::string("");
//	return std::string(_MakeShowName(nameType,(ResType)type));
}

BOOL CShaderLibTree::_GenUniqueName(NodeType type,std::string &name)
{
	return IncreaseFileTailOrdinal(name,3);
}


void CShaderLibTree::_ModifyEdit(NodeHandle hNode,std::string &str)
{
// 	NodeType type=CNodeTree::GetType(hNode);
// 	if (Res_IsNative(type))
// 	{
// 		std::string pathOld=CNodeTree::GetPath(hNode);
// 		std::string suffix=GetFileSuffix(pathOld);
// 		RemoveFileSuffix(str);
// 		MakeFileSuffix(str,suffix.c_str());
// 	}
// 	str=_MakeShowName(str.c_str(),(ResType)type);
}


const char *CShaderLibTree::_GetSscPath(NodeHandle hNode,BOOL &bFolder)
{
// 	if (hNode==NodeHandle_Root)
// 	{
// 		bFolder=TRUE;
// 		return _pathRoot.c_str();
// 	}
// 
// 	bFolder=FALSE;
// 
// 	NodeType type=GetType(hNode);
// 	const char *path=CNodeTree::GetPath(hNode);
// 
// 	if (Res_IsFolder(type))
// 	{
// 		bFolder=TRUE;
// 		return _MakeFolderPath(path);
// 	}
// 
// 	return _MakeResPath(path,(ResType)type);
	return "";
}

const char*CShaderLibTree::_GetShowName(NodeHandle hNode,const char *nameOrg)
{
// 	NodeType type=GetType(hNode);
// 	if (type==Res_Folder)
// 		return nameOrg;
// 
// 	return _MakeShowName(nameOrg,(ResType)type);
	return nameOrg;
}


UINT CShaderLibTree::_GetImageID()
{
	return IDB_SHADERLIBTREEICON;
}

DWORD CShaderLibTree::_GetImageIdx(NodeHandle hNode,SscState state)
{
	NodeType type=GetType(hNode);
	if (type==SHADERLIB_NODE_FEATUREFOLDER)
		return 0;
	return 1;
}


BOOL CShaderLibTree::_IsEditable()
{
	if (!CNodeTreeCtrl::_IsEditable())
		return FALSE;
	return TRUE;
}

BOOL CShaderLibTree::_CanRename(NodeType type)
{
	if ((type==SHADERLIB_NODE_LIB)||(type==SHADERLIB_NODE_FEATURE))
		return TRUE;
	return FALSE;
}

BOOL CShaderLibTree::_CanNew(NodeType type)
{
	if ((type==SHADERLIB_NODE_LIB)||(type==SHADERLIB_NODE_FEATURE))
		return TRUE;
	return FALSE;
}

BOOL CShaderLibTree::_CanAutoGenUniqueName(NodeType type)
{
	return TRUE;
}

void CShaderLibTree::_OnCustomMenu(CMenu *menu)
{
	return;
}


void CShaderLibTree::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	if (!_Tree())
		return;
	// TODO: Add your control notification handler code here

	NodeHandle hNode=GetCurSel();

	if (hNode!=NodeHandle_Null)
	{
		const char *path=CNodeTree::GetPath(hNode);
		NodeType tp=CNodeTree::GetType(hNode);

		std::string nm=GetFileTitle(std::string(path));
		if ((tp==SHADERLIB_NODE_LIB)||(tp==SHADERLIB_NODE_LIBFEATURE))
			NotifyOwner(GLM_ShaderTree_DblClick,(DWORD)nm.c_str(),(DWORD)tp);
	}

}

void CShaderLibTree::Update()
{
// 	if (CNodeTree::IsModified())
// 	{
// 		NotifyOwner(GLM_ResTree_ContentChange,0,0);
// 		CNodeTree::ClearModified();
// 	}
	CNodeTreeCtrl::IncUpdateSsc();
}

BOOL CShaderLibTree::_OnSscOp(HTREEITEM *items,DWORD c,SscOp op,BOOL bTest)
{
	return TRUE;
// 	BOOL bRet=CNodeTreeCtrl::_OnSscOp(items,c,op,bTest);
// 
// 	if (!bTest)
// 	{
// 		if ((op==CheckOut)||(op==Get)||(op==GetFolder))
// 		{
// 			for (int i=0;i<c;i++)
// 			if (op==GetFolder)
// 				_Refresh();
// 
// 			NotifyOwner(GLM_ResTree_ContentChange,1,0);
// 		}
// 	}
// 	return bRet;
}

void CShaderLibTree::Refresh()
{
	TreeCtrlState state;
	RecordTreeCtrlState(this,state,"\\");

	SetContent(_global);

	RestoreTreeCtrlState(this,state,"\\");
}


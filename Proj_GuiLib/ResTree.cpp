#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IUtilRS.h"


#include "interface/interface.h"

#include "assert.h"
#include ".\restree.h"

#include "Log/LogFile.h"
#include "Log/LogDump.h"
#include "resdata/ResData.h"

#include "stringparser/stringparser.h"

#include "TreeCtrlBase.h"

#include "resource.h"

#include "WMGuiLib.h"


//////////////////////////////////////////////////////////////////////////
//CResTree

//////////////////////////////////////////////////////////////////////////
//CResTree
BEGIN_MESSAGE_MAP(CResTree, CNodeTreeCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_RESTREE_COPYPATH,OnCopyPath)
	ON_COMMAND(ID_RESTREE_BROWSEFOLDER,OnBrowseFolder)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
END_MESSAGE_MAP()


CResTree::CResTree()
{
	_idTimer=0;
	_hOwner=NULL;
}

CResTree::~CResTree()
{
}

int CResTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CNodeTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	CNodeTreeCtrl::SetSsc(g_ssGuiLib.ssc);



	return 0;
}


void CResTree::OnDestroy()
{

	Clear();
	CNodeTreeCtrl::OnDestroy();
}


void CResTree::Clear()
{
	_pathRoot="";
	CNodeTree::ResetContent();
	CNodeTreeCtrl::SetNodeTree(NULL);
}

void CResTree::SetContent(const char *path,ResType filter,BOOL bRecursive)
{
	IFileSystem *pFS=g_ssGuiLib.pUtilRS->GetFS();
	std::vector<std::string> filelist,folderlist;
	if (bRecursive)
	{
		IFileSystem_EnumAllR(pFS,path,filelist,folderlist);
	}
	else
	{
		IFileSystem_EnumAll(pFS,path,filelist,folderlist);
	}

	SetContent(path,filelist,folderlist,filter);

}

void CResTree::SetContent(const char *path,std::vector<std::string> &filelist,std::vector<std::string> &folderlist,ResType filter)
{
	CNodeTree::ResetContent();

	_pathRoot=path;

	std::string s;
	for (int i=0;i<folderlist.size();i++)
		CNodeTree::AddNode(folderlist[i].c_str(),Res_Folder,NodePtr_Null);

	std::unordered_map<std::string,ResType>mp;

	if (TRUE)
	{
		CNodeTree::_LoadTypes();
		std::map<NodeType,NodeTypeInfo>::iterator it;
		std::string ss;
		for (it=_types.begin();it!=_types.end();it++)
		{
			if ((*it).first==NodeType_Root)
				continue;
			ResType type=(ResType)(*it).first;
			if(Res_IsFolder(type))
				continue;
			if ((filter!=Res_None)&&(filter!=type))
				continue;

			if (Res_IsNative(type))
			{
				switch(type)
				{
					case Res_Texture:
					{
						mp[std::string("dds")]=Res_Texture;
						mp[std::string("jpg")]=Res_Texture;
						mp[std::string("tga")]=Res_Texture;
						break;
					}
					case Res_Sound:
					{
						mp[std::string("ogg")]=Res_Sound;
						mp[std::string("mp3")]=Res_Sound;
						mp[std::string("wav")]=Res_Sound;
						break;
					}
					case Res_Ragdoll:
					{
						mp[std::string("hkx")]=Res_Ragdoll;
						mp[std::string("rgd")]=Res_Ragdoll;
						break;
					}
				}
				//XXXXX:more res type
				continue;
			}


			ResData *data=ResData_New(type);
			if (data)
			{
				ss=std::string(data->GetTypeSuffix());
				StringLower(ss);
				mp[ss]=type;
				ResData_Delete(data);
			}
		}
	}

	for (int i=0;i<filelist.size();i++)
	{
		s=GetFileSuffix(filelist[i]);
		if (s=="")
			continue;
		StringLower(s);

		std::unordered_map<std::string,ResType>::iterator it=mp.find(s);
		if (it!=mp.end())
		{
			ResType tp=(*it).second;
			s=filelist[i];

			CNodeTree::AddNode(s.c_str(),tp,NodePtr_Null);
		}

	}

	CNodeTreeCtrl::SetNodeTree(CNodeTree::ObtainRef());

}

const char *CResTree::_MakeFolderPath(const char *path)
{
	_temp=_pathRoot+"\\"+path;
	return _temp.c_str();
}

const char *CResTree::_MakeResPath(const char *path,ResType tp)
{
	_temp=_pathRoot+"\\"+path;
	if (Res_IsStandard(tp))
	{
		ResData *data=ResData_New(tp);
		if (data)
		{
			RemoveFileSuffix(_temp);
			MakeFileSuffix(_temp,data->GetTypeSuffix());

			ResData_Delete(data);
		}
	}
	return _temp.c_str();
}

const char *CResTree::_MakeShowName(const char *name,ResType tp)
{
	_temp=name;
	if (Res_IsStandard(tp))
	{
		ResData *data=ResData_New(tp);
		if (data)
		{
			RemoveFileSuffix(_temp);
			MakeFileSuffix(_temp,data->GetTypeSuffix());

			ResData_Delete(data);
		}
	}
	return _temp.c_str();
}


void CResTree::_AddType(ResType tp)
{
	ResData *data=ResData_New(tp);
	CNodeTree::_AddType(tp,data->GetTypeName());
	ResData_Delete(data);
}


void CResTree::_OnInitType()
{
	//Folder
	CNodeTree::_AddType(Res_Folder,"Folder");

	//Native Types
	CNodeTree::_AddType(Res_Texture,"Texture");

	//Standard Types
	_AddType(Res_Mesh);
	_AddType(ResA_XForm);
	_AddType(Res_Mtrl);
	_AddType(ResA_MtrlColor);
	_AddType(ResA_MapCoord);
	_AddType(Res_Dummies);
	_AddType(Res_Spt);
	_AddType(Res_Mopp);
	_AddType(Res_Spg);
	_AddType(Res_AnimTree);
	_AddType(ResA_Bones2);
	_AddType(Res_MtrlExt);
	_AddType(Res_Sound);
	_AddType(Res_Records);
	_AddType(Res_Ragdoll);
	_AddType(Res_Dtr);
	_AddType(Res_BehaviorGraph);
	//XXXXX:more res type
}

BOOL CResTree::_OnCheckTypeRelation(NodeType typeParent,NodeType typeChild)
{
	if (typeParent==NodeType_Root)
		return TRUE;
	if (typeParent==Res_Folder)
		return TRUE;
	return FALSE;
}

const char *CResTree::_GetSep()
{
	return "\\";
}

NodePtr CResTree::_OnNew(const char *path,NodeType type,void *param)
{
	IFileSystem *pFS=g_ssGuiLib.pUtilRS->GetFS();

	NodePtr ret=NodePtr_Null;

	assert(!Res_IsNative(type));

	if (Res_IsFolder(type))
	{
		const char *pathFolder=_MakeFolderPath(path);
		if (!pFS->ExistFolderAbs(pathFolder))
		{
			IFolder *folder=pFS->OpenFolderAbs(pathFolder,FileAccessMode_Write);
			if (!folder)
			{
				LOG_DUMP_1P("ResTree",Log_Error,"无法创建文件夹(\"%s\")!",pathFolder);
				return NodePtr_Invalid;
			}
			folder->Close();
		}
	}
	if (Res_IsStandard(type))
	{
		const char *path2=_MakeResPath(path,(ResType)type);
		ResData *data=ResData_New((ResType)type);
		if (data)
		{
			if (FALSE==g_ssGuiLib.pUtilRS->SaveRes(path2,data))
			{
				LOG_DUMP_1P("ResTree",Log_Error,"无法创建资源文件(\"%s\")!",path2);
				return NodePtr_Invalid;
			}
			ResData_Delete(data);
		}
	}
	return ret;
}

BOOL CResTree::_OnDelete(NodeHandle hNode)
{
	NodePtr pNode=CNodeTree::GetPtr(hNode);
	const char *path=CNodeTree::GetPath(hNode);
	NodeType type=CNodeTree::GetType(hNode);

	IFileSystem *pFS=g_ssGuiLib.pUtilRS->GetFS();

	if (type!=Res_Folder)
	{
		if (FALSE==pFS->RemoveFileAbs(_MakeResPath(path,(ResType)type)))
		{
			LOG_DUMP_1P("ResTree",Log_Error,"无法删除资源文件(\"%s\")",_MakeResPath(path,(ResType)type));
			return FALSE;
		}
	}
	if (type==Res_Folder)
	{
		if (FALSE==pFS->RemoveFolderAbs(_MakeFolderPath(path)))
		{
			LOG_DUMP_1P("ResTree",Log_Error,"无法删除文件夹(\"%s\")",_MakeFolderPath(path));
			return FALSE;
		}
	}

	return TRUE;

}

BOOL CResTree::_OnRename(NodeHandle hNode,const char *nameNew)
{
	IFileSystem *pFS=g_ssGuiLib.pUtilRS->GetFS();

	NodePtr pNode=CNodeTree::GetPtr(hNode);
	NodeType type=GetType(hNode);
	const char *path=CNodeTree::GetPath(hNode);
	if (type==Res_Folder)
	{
		if (GetChildCount(hNode)>0)
		{
			LOG_DUMP_1P("ResTree",Log_Error,"无法修改目录名称(\"%s\"),因为目录不为空!",_MakeFolderPath(path));
			return FALSE;
		}
		if (FALSE==pFS->RenameAbs(_MakeFolderPath(path),nameNew))
		{
			LOG_DUMP_2P("ResTree",Log_Error,"无法为目录改名(\"%s\"改为\"%s\")",_MakeFolderPath(path),nameNew);
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		std::string name=_MakeShowName(nameNew,(ResType)type);
		std::string pathOld=_MakeResPath(path,(ResType)type);
		if (StringEqualNoCase(GetFileName(pathOld).c_str(),name.c_str()))
			return TRUE;//没有变化

		if (Res_IsNative(type))
		{//Native的资源,我们必须使用原来的后缀
			std::string suffix=GetFileSuffix(pathOld);
			RemoveFileSuffix(name);
			MakeFileSuffix(name,suffix.c_str());
		}


		if (FALSE==pFS->RenameAbs(pathOld.c_str(),name.c_str()))
		{
			LOG_DUMP_2P("ResTree",Log_Error,"无法修改资源名称(\"%s\"改为\"%s\")",pathOld.c_str(),name.c_str());
			return FALSE;
		}
	}

	return TRUE;
}

std::string CResTree::_GenNewName(NodeType type,const char *nameType)
{
	return std::string(_MakeShowName(nameType,(ResType)type));
}

BOOL CResTree::_GenUniqueName(NodeType type,std::string &name)
{
	return IncreaseFileTailOrdinal(name,3);
}


void CResTree::_ModifyEdit(NodeHandle hNode,std::string &str)
{
	NodeType type=CNodeTree::GetType(hNode);
	if (Res_IsNative(type))
	{
		std::string pathOld=CNodeTree::GetPath(hNode);
		std::string suffix=GetFileSuffix(pathOld);
		RemoveFileSuffix(str);
		MakeFileSuffix(str,suffix.c_str());
	}
	str=_MakeShowName(str.c_str(),(ResType)type);
}


const char *CResTree::_GetSscPath(NodeHandle hNode,BOOL &bFolder)
{
	if (hNode==NodeHandle_Root)
	{
		bFolder=TRUE;
		return _pathRoot.c_str();
	}

	bFolder=FALSE;

	NodeType type=GetType(hNode);
	const char *path=CNodeTree::GetPath(hNode);

	if (Res_IsFolder(type))
	{
		bFolder=TRUE;
		return _MakeFolderPath(path);
	}

	return _MakeResPath(path,(ResType)type);
}

const char*CResTree::_GetShowName(NodeHandle hNode,const char *nameOrg)
{
	NodeType type=GetType(hNode);
	if (type==Res_Folder)
		return nameOrg;

	return _MakeShowName(nameOrg,(ResType)type);
}


UINT CResTree::_GetImageID()
{
	return IDB_RESTREEICON;

}

DWORD CResTree::_GetImageIdx(NodeHandle hNode,SscState state)
{
	if (!_Tree())
		return 0;
	NodeType type=_Tree()->GetType(hNode);
	if (type==Res_Folder)
		return 0;
	if (_bEditable)
	{
		switch(state)
		{
			case SSC_NOTCONTROLLED:
				return 1;
			case SSC_NOTCHECKEDOUT:
				return 3;
			case SSC_CHECKEDOUT_ME:
				return 4;
			case SSC_CHECKEDOUT:
				return 2;
			default:
				return 5;
		}
	}
	else
		return 1;
	assert(FALSE);
	return 0;
}


BOOL CResTree::_IsEditable()
{
	if (!CNodeTreeCtrl::_IsEditable())
		return FALSE;
	return TRUE;
}

BOOL CResTree::_CanRename(NodeType type)
{
	return TRUE;
}

BOOL CResTree::_CanNew(NodeType type)
{
	if ((type==Res_Folder)||(type==Res_Mtrl)||(type==ResA_XForm)||(type==Res_AnimTree)||(type==Res_MtrlExt)||(type==Res_Records)||(type==Res_BehaviorGraph))
		return TRUE;
	//XXXXX:more res type
	return FALSE;
}

BOOL CResTree::_CanAutoGenUniqueName(NodeType type)
{
	return TRUE;
}

std::string CResTree::_GetResPath()
{
	std::string path;
	if (g_ssGuiLib.pRS)
	{
		if (_sel.total.size()==1)
		{
			HTREEITEM hItem=_sel.total[0];
			path=PathFromItem(this,hItem,"\\");

			path=_pathRoot+"\\"+path;

			std::string s=g_ssGuiLib.pRS->GetPath(Path_Res);

			path=CutHeadPath(path.c_str(),s.c_str());
		}
	}

	return path;


}


void CResTree::_OnCustomMenu(CMenu *menu)
{
	if (!_Tree())
		return;

	if (_sel.total.size()>1)
		return;

	if (!_IsEditable())
		return;

	menu->AppendMenu(MF_SEPARATOR,0,_T(""));

	std::string path=_GetResPath();
	if (path!="")
	{
		path=std::string("Copy \"")+path+"\"";
		menu->AppendMenu(MF_STRING, ID_RESTREE_COPYPATH, fromMBCS(path.c_str()));
	}

	menu->AppendMenu(MF_SEPARATOR,0,_T(""));

	menu->AppendMenu(MF_STRING,ID_RESTREE_BROWSEFOLDER,_T("打开当前目录"));
	menu->AppendMenu(MF_SEPARATOR,0,_T(""));
	
}



void CResTree::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	if (!_Tree())
		return;
	// TODO: Add your control notification handler code here

	NodeHandle hNode=GetCurSel();

	if (hNode!=NodeHandle_Null)
	{
		ResType tp=(ResType)CNodeTree::GetType(hNode);
		if (tp!=Res_Folder)
		{
			const char *path=CNodeTree::GetPath(hNode);
			path=_MakeResPath(path,tp);
			NotifyOwner(GLM_ResTree_DblClick,(DWORD_PTR)path,(DWORD_PTR)tp);
		}
	}

}

void CResTree::Update()
{
	if (CNodeTree::IsModified())
	{
		NotifyOwner(GLM_ResTree_ContentChange,0,0);
		CNodeTree::ClearModified();
	}
	CNodeTreeCtrl::IncUpdateSsc();
}

BOOL CResTree::_OnSscOp(HTREEITEM *items,DWORD c,SscOp op,BOOL bTest)
{
	BOOL bRet=CNodeTreeCtrl::_OnSscOp(items,c,op,bTest);

	if (!bTest)
	{
		if ((op==CheckOut)||(op==Get)||(op==GetFolder))
		{
			for (int i=0;i<c;i++)
			if (op==GetFolder)
				_Refresh();

			NotifyOwner(GLM_ResTree_ContentChange,1,0);
		}
	}
	return bRet;
}

void CResTree::_Refresh()
{
	TreeCtrlState state;
	RecordTreeCtrlState(this,state,"\\");

	SetContent(_pathRoot.c_str());

	RestoreTreeCtrlState(this,state,"\\");
}

void CopyToClipboard(CWnd *wnd,const char *str)
{
	if (wnd->OpenClipboard())
	{
		HGLOBAL clipbuffer;
		char * buffer;
		EmptyClipboard();
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(str)+20);
		buffer = (char*)GlobalLock(clipbuffer);
		strcpy(buffer, str);
		GlobalUnlock(clipbuffer);
		SetClipboardData(CF_TEXT,clipbuffer);
		CloseClipboard();
	}
}

BOOL PasteFromClipboard(CWnd *wnd,std::string &str)
{
	// Test to see if we can open the clipboard first.
	if (wnd->OpenClipboard()) 
	{
		HANDLE hClipboardData = GetClipboardData(CF_TEXT);

		if (hClipboardData)
		{
			char *pchData = (char*)GlobalLock(hClipboardData);

			str= pchData;
			GlobalUnlock(hClipboardData);
			CloseClipboard();
			return TRUE;
		}

		CloseClipboard();
	}
	return FALSE;
}

void CResTree::OnCopyPath()
{
	std::string path=_GetResPath();
	if (path=="")
		return;
	CopyToClipboard(this,path.c_str());
}

void CResTree::OnBrowseFolder()
{
	ShellExecute(NULL, _T("open"), _T("explorer.exe"), fromMBCS(_pathRoot.c_str()), NULL, SW_SHOWNORMAL);
}

void CResTree::EnsureVisible(const char *pathAbs)
{
	if (CheckPathContaining(_pathRoot.c_str(),pathAbs))
	{
		std::string path=CutHeadPath(pathAbs,_pathRoot.c_str());
		if (!path.empty())
		{
			HTREEITEM hItem=ItemFromPath(path);
			if ((hItem!=NULL)&&(hItem!=TVI_ROOT))
			{
				CTreeCtrl::EnsureVisible(hItem);
				SelectAll(FALSE);
				SelectItem(hItem);
			}
		}
	}
}

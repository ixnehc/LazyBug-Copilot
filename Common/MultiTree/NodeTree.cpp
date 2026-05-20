/********************************************************************
	created:	2007/6/5   13:36
	filename: 	e:\IxEngine\Common\MultiTree\NodeTree.cpp
	author:		cxi
	
	purpose:	a node tree using string as its key
*********************************************************************/
#include "stdh.h"
#include "NodeTree.h"
#include "../stringparser/stringparser.h"
#include "../datapacket/DataPacket.h"

#pragma warning(disable:4312)
#pragma warning(disable:4311)


CNodeTree::~CNodeTree()
{
	_DiscardRef();
}

NodeTreeRef *CNodeTree::ObtainRef()
{
	if (!_ref)
	{
		_ref=Class_New2(NodeTreeRef);
		_ref->tree=this;
		_ref->AddRef();
	}
	return _ref;
}

void CNodeTree::_DiscardRef()
{
	if (_ref)
		_ref->tree=NULL;
	SAFE_RELEASE(_ref);
}



BOOL CNodeTree::_AddType(NodeType type,const char *nameType,
											 const char *category,const char *showname)
{
	if (NodeType_None==type)
		return FALSE;
	assert(_types.find(type)==_types.end());

	NodeTypeInfo &info=_types[type];
	info.type=type;
	info.category=category;
	info.name=nameType;
	info.showname=showname;

	return TRUE;
}

void CNodeTree::_LoadTypes()
{
	if (_types.size()>0)
		return;
	_AddType(NodeType_Root,"<root>");
	_OnInitType();
}


NodeTypeInfo *CNodeTree::GetTypeInfo(NodeType type)
{
	_LoadTypes();
	std::map<NodeType,NodeTypeInfo>::iterator it;
	it=_types.find(type);
	if (it==_types.end())
		return NULL;
	return &(*it).second;
}

NodeType *CNodeTree::GetChildType(NodeType type,DWORD &count)
{
	_LoadTypes();

	static std::vector<NodeType> buf;
	buf.clear();

	std::map<NodeType,NodeTypeInfo>::iterator it;
	for (it=_types.begin();it!=_types.end();it++)
	{
		if ((*it).first==NodeType_Root)
			continue;
		if (_OnCheckTypeRelation(type,(*it).first))
			buf.push_back((*it).first);
	}

	count=buf.size();
	return buf.data();
}

void CNodeTree::ResetContent()//Will keep the type
{
	_DiscardRef();

	_tree.Clear();
	_types.clear();
	_bModified=TRUE;
}


void CNodeTree::_GetPath(std::list<std::string>&path,NodeHandle hNode)
{
	path.clear();

	std::string s;
	while(MULTITREEBRANCH_ROOT!=hNode)
	{
		s=_tree.GetBranchKey(hNode);
		path.push_front(s);
		hNode=_tree.GetParentBranch(hNode);
	}
}

NodeHandle CNodeTree::AddNode(const char *path,NodeType type,NodePtr ptr)
{
	std::string s=path;
	std::list<std::string>keys;
	SplitStringBy(_GetSep(),s,&keys);

	//first remove existing
	NodeHandle hNode=_tree.GetBranch(keys);
	if (hNode!=MULTITREEBRANCH_NULL)
		_tree.ClearLeaf(hNode);

	_tree.AddLeaf(keys,ptr);
	_tree.AddLeaf(keys,type);

	_bModified=TRUE;

	return _tree.GetBranch(keys);
}

void CNodeTree::RemoveNode(NodeHandle hNode)
{
	_tree.RemoveBranch(hNode);
	_bModified=TRUE;
}


NodeHandle CNodeTree::AddChild(NodeHandle hNode,NodeType type,const char *name,void *param)
{
	_LoadTypes();
	std::map<NodeType,NodeTypeInfo>::iterator it=_types.find(type);
	if (it==_types.end())
		return NodeHandle_Null;

	std::string s=name;
	if (((int)s.find(_GetSep()))!=-1) 
		return NodeHandle_Null;
	std::list<std::string>keys;
	_GetPath(keys,hNode);
	keys.push_back(s);

	if (NodeHandle_Null!=_tree.GetBranch(keys))
		return NodeHandle_Null;//Already exists

	std::string path;
	LinkStringBy(_GetSep(),path,&keys);

	NodePtr pNode=_OnNew(path.c_str(),type,param);

	if (pNode==NodePtr_Invalid)
		return NodeHandle_Null;

	_tree.AddLeaf(keys,pNode);//the ptr
	_tree.AddLeaf(keys,type);//the type

	_bModified=TRUE;



	return _tree.GetBranch(keys);
}

BOOL CNodeTree::_RemoveChildR(NodeHandle hNode)
{
	DWORD c=_tree.GetSubBranchCount(hNode);
	for (int i=0;i<c;i++)
	{
		if (FALSE==_RemoveChildR(_tree.GetSubBranch(hNode,i)))
			return FALSE;
	}

	if (FALSE==_OnDelete(hNode))
		return FALSE;

	_tree.RemoveBranch(hNode);

	_bModified=TRUE;
	return TRUE;
}

//return FALSE if hNode has any child
BOOL CNodeTree::RemoveEndChild(NodeHandle hNode)
{
	if (GetChildCount(hNode)>0)
		return FALSE;
	return RemoveChild(hNode);
}



BOOL CNodeTree::RemoveChild(NodeHandle hNode)
{
	if (hNode==MULTITREEBRANCH_ROOT)
		return FALSE;
	if (FALSE==_RemoveChildR(hNode))
		return FALSE;

	return TRUE;
}

BOOL CNodeTree::RemoveAllChilds(NodeHandle hNode)
{
	std::vector<NodeHandle>subs;
	DWORD c=_tree.GetSubBranchCount(hNode);
	subs.resize(c);
	for (int i=0;i<c;i++)
		subs[i]=_tree.GetSubBranch(hNode,i);	

	for (int i=0;i<c;i++)
		RemoveChild(subs[i]);

	_bModified=TRUE;

	return TRUE;
}


NodePtr CNodeTree::GetPtr(NodeHandle hNode)
{
	if (hNode==MULTITREEBRANCH_ROOT)
		return NULL;
	NodePtr p;
	if (FALSE==_tree.GetLeaf(hNode,0,(DWORD&)p))
		return NULL;
	return p;
}

NodeType CNodeTree::GetType(NodeHandle hNode)
{
	if (hNode==MULTITREEBRANCH_ROOT)
		return NodeType_Root;
	NodeType type;
	if (FALSE==_tree.GetLeaf(hNode,1,(DWORD&)type))
		return NodeType_None;
	return type;
}

const char *CNodeTree::GetName(NodeHandle hNode)
{
	return _tree.GetBranchKey(hNode).c_str();
}

const char *CNodeTree::GetShowName(NodeHandle hNode)
{
	return _GetShowName(hNode,GetName(hNode));
}


BOOL CNodeTree::Rename(NodeHandle hNode,const char *nameNew)
{
	assert(strlen(GetSep())==1);
	if (strcmp(nameNew,GetName(hNode))==0)
		return TRUE;//no change
	if (StringFind(nameNew,GetSep()[0])!=-1)
		return FALSE;//could not contain seperator

	if (FALSE==_tree.ChangeKey(hNode,std::string(nameNew),TRUE))//test whether could change
		return FALSE;

	NodePtr pNode=GetPtr(hNode);
	if (pNode!=NodePtr_Invalid)
	{
		if (FALSE==_OnRename(hNode,nameNew))
			return FALSE;
	}

	BOOL bRet=_tree.ChangeKey(hNode,std::string(nameNew));
	if (bRet)
		_bModified=TRUE;
	return bRet;
}

BOOL CNodeTree::Move(NodeHandle hNode,BOOL bUp)
{
	BOOL bRet=_tree.Move(hNode,bUp);
	if (bRet)
		_bModified=TRUE;
	return bRet;
}

DWORD CNodeTree::GetChildCount(NodeHandle hNode)
{
	return _tree.GetSubBranchCount(hNode);
}

NodeHandle CNodeTree::GetChild(NodeHandle hNode,DWORD idx)
{
	return _tree.GetSubBranch(hNode,idx);
}

NodeHandle CNodeTree::FindChild(NodeHandle hNode,const char *nameChild)
{
	std::list<std::string>keys;
	_GetPath(keys,hNode);
	keys.push_back(std::string(nameChild));

	return (NodeHandle)_tree.GetBranch(keys);
}

NodeHandle CNodeTree::FindChild_NoCase(NodeHandle hNode,const char *nameChild)
{
	DWORD count=_tree.GetSubBranchCount(hNode);
	for (int i=0;i<count;i++)
	{
		NodeHandle hChild=GetChild(hNode,i);
		if (StringEqualNoCase(GetName(hChild),nameChild))
			return hChild;
	}
	return NodeHandle_Null;
}



NodeHandle CNodeTree::GetParent(NodeHandle hNode)
{
	return _tree.GetParentBranch(hNode);
}

BOOL CNodeTree::CheckDescendant(NodeHandle hAncestor,NodeHandle hNode)
{
	while(1)
	{
		hNode=GetParent(hNode);
		if (hNode==MULTITREEBRANCH_ROOT)
			return FALSE;
		if (hNode==hAncestor)
			return TRUE;
	}

	return FALSE;
}



void CNodeTree::_Enum(NodeHandle hNode,NodeType type,std::vector<NodeHandle>&buf)
{
	if (hNode!=NodeHandle_Root)
	{
		if ((type==NodeType_None)||(GetType(hNode)==type))
			buf.push_back(hNode);
	}

	DWORD c=_tree.GetSubBranchCount(hNode);
	for (int i=0;i<c;i++)
		_Enum(_tree.GetSubBranch(hNode,i),type,buf);
}


NodeHandle *CNodeTree::Enum(NodeHandle hNode,NodeType type,DWORD &count)
{
	static std::vector<NodeHandle> buf;
	buf.clear();
	_Enum(hNode,type,buf);

	count=buf.size();
	return buf.data();
}

const char *CNodeTree::GetPath(NodeHandle hNode)
{
	static std::string s;
	s="";
	std::string ss;

	while(hNode!=MULTITREEBRANCH_ROOT)
	{
		ss=GetName(hNode);
		if (s=="")
			s=ss;
		else
			s=ss+_GetSep()+s;
		hNode=GetParent(hNode);
	}

	return s.c_str();
}

NodeHandle CNodeTree::Find(const char *path)
{
	std::list<std::string> keys;
	SplitStringBy(_GetSep(),std::string(path),&keys);
	return _tree.GetBranch(keys);
}

void CNodeTree::_Save(NodeHandle hNode,CDataPacket &dp)
{
	if (hNode!=MULTITREEBRANCH_ROOT)
	{
		dp.Data_NextDword()=1;
		std::list<std::string>keys;
		_tree.FindBranch(keys,hNode);
		dp.Data_NextDword()=keys.size();
		std::list<std::string>::iterator it;
		for (it=keys.begin();it!=keys.end();it++)
			dp.Data_WriteString((*it));

		NodeType type=GetType(hNode);
		dp.Data_NextDword()=type;
		NodePtr p=GetPtr(hNode);
		dp.Data_NextDword()=p;
		_OnSave(dp,hNode);
	}
	else
		dp.Data_NextDword()=0;

	//Save the childs
	DWORD c=GetChildCount(hNode);
	dp.Data_NextDword()=c;
	for (int i=0;i<c;i++)
		_Save(GetChild(hNode,i),dp);
}

void CNodeTree::_Load(CDataPacket &dp)
{
	if (dp.Data_NextDword()==1)
	{
		DWORD c=dp.Data_NextDword();
		std::list<std::string>keys;
		std::string s;
		for (int i=0;i<c;i++)
		{
			dp.Data_ReadString(s);
			keys.push_back(s);
		}

		NodeType type=dp.Data_NextDword();
		NodePtr p=dp.Data_NextDword();
		_tree.AddLeaf(keys,(DWORD)p);
		_tree.AddLeaf(keys,type);

		NodeHandle hNode=_tree.GetBranch(keys);
		_OnLoad(dp,hNode);
	}

	DWORD c=dp.Data_NextDword();
	for (int i=0;i<c;i++)
		_Load(dp);
}


void CNodeTree::Save(CDataPacket &dp)
{
	_Save(MULTITREEBRANCH_ROOT,dp);
}

void CNodeTree::Load(CDataPacket &dp)
{
	_Load(dp);

	ClearModified();
}

//modify by star. 2007-11-6{
BOOL CNodeTree::SetNodeCheck(NodeHandle hNode,BOOL bCheck)
{	
	return _SetNodeCheck(hNode,bCheck);
}
BOOL CNodeTree::TestNodeCheck(NodeHandle hNode)
{
	return _TestNodeCheck(hNode);
}
BOOL CNodeTree::SupportCheck()
{
	 return _SupportCheck();
}
//}

const char *CNodeTree::GetSscPath(NodeHandle hNode,BOOL &bFolder)
{
	bFolder=FALSE;
	return _GetSscPath(hNode,bFolder);
}

BOOL CNodeTree::_CheckChildName(NodeHandle hParentNode,const char *name)
{
	return !FindChild_NoCase(hParentNode,name);
}

BOOL CNodeTree::CheckChildName(NodeHandle hNode,const char *name)
{
	return _CheckChildName(hNode,name);
}

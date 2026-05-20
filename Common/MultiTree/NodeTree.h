 #pragma once

#include "MultiTree.h"

#include "../class/class.h"

#include <string>

typedef MultiTreeBanch NodeHandle;
typedef DWORD NodeType;
typedef DWORD NodePtr;
typedef void *NodeRecover;

#define NodeHandle_Null (MULTITREEBRANCH_NULL)
#define NodeHandle_Root (MULTITREEBRANCH_ROOT)

#define NodeType_None (0)
#define NodeType_Root (0xffffffff)

#define NodePtr_Null (0)
#define NodePtr_Invalid (0xffffffff)

class CDataPacket;

struct NodeTypeInfo
{
	NodeType type;
	std::string category;//"" indicates no category
	std::string name;
	std::string showname;
};
//modify by star 2007-11-6{
struct TreeNodeState
{
	BOOL  state;
	NodeHandle  handle;
};
//}

class CNodeTree;
struct NodeTreeRef
{
	DEFINE_CLASS(NodeTreeRef);
	NodeTreeRef()
	{
		refcount=0;
		tree=NULL;
	}
	int AddRef()	{		refcount++;return refcount;	}
	int Release()
	{
		refcount--;
		int t=refcount;
		if (refcount<=0)
			Class_Delete(this);
		return t;
	}

	CNodeTree *GetTree()	{		return tree;	}

	int refcount;
	CNodeTree *tree;
};


class CModBase;

class CNodeTree
{
public:
	CNodeTree()
	{
		_bModified=FALSE;
		_hNodeMod=NodeHandle_Null;
		_ref=NULL;
	}

	~CNodeTree();


	void ResetContent();//Will keep the type
	NodeTreeRef *ObtainRef();
	NodeTypeInfo *GetTypeInfo(NodeType type);
	NodeType *GetChildType(NodeType type,DWORD &count);//A temply ptr
	const char *GetSep()	{		return _GetSep();	}
	BOOL IsReadOnly()	{		return _IsReadOnly();	}
	NodeHandle AddChild(NodeHandle hNode,NodeType type,const char *name,void *param=NULL);
	BOOL RemoveEndChild(NodeHandle hNode);//return FALSE if hNode has any child
	BOOL RemoveChild(NodeHandle hNode);
	BOOL RemoveAllChilds(NodeHandle hNode);
	BOOL CheckChildName(NodeHandle hNode,const char *name);

	//Note:AddNode()/RemoveNode() will add/remove the node without calling the internal
	//_OnNew()/_OnDelete(),
	NodeHandle AddNode(const char *path,NodeType type,NodePtr ptr);//will update the node if already existing
	void RemoveNode(NodeHandle hNode);

	NodePtr GetPtr(NodeHandle hNode);
	NodeType GetType(NodeHandle hNode);//if is root,will return an undefined value
	const char *GetName(NodeHandle hNode);
	const char *GetShowName(NodeHandle hNode);
	BOOL Rename(NodeHandle hNode,const char *nameNew);
	BOOL Move(NodeHandle hNode,BOOL bUp);

	DWORD GetChildCount(NodeHandle hNode);
	NodeHandle GetChild(NodeHandle hNode,DWORD idx);
	NodeHandle FindChild(NodeHandle hNode,const char *nameChild);
	NodeHandle FindChild_NoCase(NodeHandle hNode,const char *nameChild);//²»¿¼ÂÇ´óÐ¡Ð´
	NodeHandle GetParent(NodeHandle hNode);
	BOOL CheckDescendant(NodeHandle hAncestor,NodeHandle hNode);

	//if type is NodeType_None,enum the nodes of all the types
	NodeHandle *Enum(NodeHandle hNode,NodeType type,DWORD &count);

	const char *GetPath(NodeHandle hNode);
	NodeHandle Find(const char *path);

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

	//check related
	BOOL SetNodeCheck(NodeHandle hNode,BOOL bCheck);
	BOOL TestNodeCheck(NodeHandle hNode);
	BOOL SupportCheck();

	//source safe control related
	const char *GetSscPath(NodeHandle hNode,BOOL &bFolder);
	BOOL SupportSsc()	{		return _SupportSsc();	}

	void ClearModified()	{		_bModified=FALSE;	}
	BOOL IsModified()	{		return _bModified;	}

protected:
	void _DiscardRef();
//overridable
	//base
	virtual BOOL _OnCheckTypeRelation(NodeType typeParent,NodeType typeChild)=0;//return whether typeChild could be under typeParent
	virtual const char *_GetSep()=0;
	virtual void _OnInitType()=0;
	virtual BOOL _IsReadOnly()	{		return FALSE;	}
	virtual NodePtr _OnNew(const char *path,NodeType type,void *param)=0;
	virtual BOOL _OnDelete(NodeHandle hNode)=0;
	virtual BOOL _OnRename(NodeHandle hNode,const char *nameNew){return TRUE;	}
	virtual void _OnStartModify(NodeHandle hNode)	{	}
	virtual void _OnEndModify(NodeHandle hNode){	}
	virtual void _OnSave(CDataPacket &dp,NodeHandle hNode){}
	virtual void _OnLoad(CDataPacket &dp,NodeHandle hNode){}
	virtual const char*_GetShowName(NodeHandle hNode,const char *nameOrg)	{		return nameOrg;	}
	virtual BOOL _CheckChildName(NodeHandle hParentNode,const char *name);

	//for check
	virtual BOOL _SetNodeCheck(NodeHandle pNode,BOOL bCheck)	{		return TRUE;	}
	virtual BOOL _TestNodeCheck(NodeHandle pNode)	{		return TRUE;	}
	virtual BOOL _SupportCheck()	{		return FALSE;	}
	//for ssc
	virtual BOOL _SupportSsc()	{		return FALSE;	}
	virtual const char *_GetSscPath(NodeHandle hNode,BOOL &bFolder)	{		bFolder=FALSE;return "";	}

//internal operations
	//type should be a none-0 value
	//param is a value that will be passed to _OnNewPtr(...,void *param)
	BOOL _AddType(NodeType type,const char *nameType,
						const char *category="",const char *showname="");
																							

	void _GetPath(std::list<std::string>&path,NodeHandle hNode);
	BOOL _RemoveChildR(NodeHandle hNode);
	void _Enum(NodeHandle hNode,NodeType type,std::vector<NodeHandle>&buf);
	void _Save(NodeHandle hNode,CDataPacket &dp);
	void _Load(CDataPacket &dp);

	NodeHandle _hNodeMod;//the node current being modified

	CMultiTree<DWORD,std::string,1> _tree;

	NodeTreeRef *_ref;

	void _LoadTypes();
	std::map<NodeType,NodeTypeInfo> _types;
	BOOL _bModified;
};



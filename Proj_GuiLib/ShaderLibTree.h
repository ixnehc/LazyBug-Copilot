#pragma once

#include "GuiLib.h"

#include "NodeTreeCtrl.h"

#include <map>
#include <string>

#define SHADERLIB_NODE_NONE 0
#define SHADERLIB_NODE_LIB 1
#define SHADERLIB_NODE_LIBFEATURE 2
#define SHADERLIB_NODE_FEATUREFOLDER 3
#define SHADERLIB_NODE_FEATURE 4



class CShaderLibGlobal;
class GuiLib_Api CShaderLibTree:public CNodeTreeCtrl,public CNodeTree
{
public:
	CShaderLibTree();
	~CShaderLibTree();

	void Clear();

	void SetContent(CShaderLibGlobal *global);

	void Update();
	void Refresh();

protected:


	//CNodeTree overidding
	virtual void _OnInitType();
	virtual BOOL _OnCheckTypeRelation(NodeType typeParent,NodeType typeChild);//return whether typeChild could be under typeParent
	virtual const char *_GetSep();
	virtual NodePtr _OnNew(const char *path,NodeType type,void *param);
	virtual BOOL _OnDelete(NodeHandle hNode);
	virtual BOOL _OnRename(NodeHandle hNode,const char *nameNew);
	virtual BOOL _SupportSsc()	{		return FALSE;	}  
	virtual const char *_GetSscPath(NodeHandle hNode,BOOL &bFolder);
	virtual const char*_GetShowName(NodeHandle hNode,const char *nameOrg);

	//CNodeTreeCtrl overriding
	virtual UINT _GetImageID();
	virtual DWORD _GetImageIdx(NodeHandle hNode,SscState state);

	virtual BOOL _IsEditable();
	virtual BOOL _IsExchangable()	{		return FALSE;	}
	virtual BOOL _CanRename(NodeType type);
	virtual BOOL _CanNew(NodeType type);
	virtual BOOL _CanAutoGenUniqueName(NodeType type);
	virtual std::string _GenNewName(NodeType type,const char *nameType);
	virtual BOOL _GenUniqueName(NodeType type,std::string &name);
	virtual void _ModifyEdit(NodeHandle hNode,std::string &str);

	virtual void _OnCustomMenu(CMenu *menu);

	virtual BOOL _OnSscOp(HTREEITEM *items,DWORD c,SscOp op,BOOL bTest);

	IFileSystem *_pFS;
	CShaderLibGlobal *_global;


public:

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);


};



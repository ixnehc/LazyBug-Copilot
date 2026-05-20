#pragma once

#include "GuiLib.h"

#include "resdata/ResDataDefines.h"

#include "NodeTreeCtrl.h"

#include <map>
#include <string>


struct ResData;
class CResBrowseBtn;
class CRdgHistory;
class IUtilRS;
class GuiLib_Api CResTree:public CNodeTreeCtrl,public CNodeTree
{
public:
	CResTree();
	~CResTree();

	void Clear();

	void SetContent(const char *path,ResType filter=Res_None,BOOL bRecursive=TRUE);
	void SetContent(const char *path,std::vector<std::string> &filelist,std::vector<std::string> &folderlist,ResType filter=Res_None);
	const char *GetRootPath()	{		return _pathRoot.c_str();	}

	void EnsureVisible(const char *pathAbs);

	void Update();

protected:

	void _AddType(ResType tp);
	const char *_MakeFolderPath(const char *path);
	const char *_MakeResPath(const char *path,ResType tp);
	const char *_MakeShowName(const char *name,ResType tp);
	void _Refresh();

	//CNodeTree overidding
	virtual void _OnInitType();
	virtual BOOL _OnCheckTypeRelation(NodeType typeParent,NodeType typeChild);//return whether typeChild could be under typeParent
	virtual const char *_GetSep();
	virtual NodePtr _OnNew(const char *path,NodeType type,void *param);
	virtual BOOL _OnDelete(NodeHandle hNode);
	virtual BOOL _OnRename(NodeHandle hNode,const char *nameNew);
	virtual BOOL _SupportSsc()	{		return TRUE;	}
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

	std::string _GetResPath();

	std::string _pathRoot;

	std::string _temp;

	HWND _hOwner;//Owner window,

	UINT _idTimer;

public:

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCopyPath();
	afx_msg void OnBrowseFolder();
};



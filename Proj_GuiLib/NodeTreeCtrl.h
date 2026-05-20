#pragma once

#include "GuiLib.h"

#include "MultiTree/NodeTree.h"

#include "mod/ModBase.h"

#include "sscbase.h"

struct ItemGroupDesc2
{
	std::vector<HTREEITEM>total;
	void Clear()
	{
		total.clear();
	}
	void Copy(ItemGroupDesc2 &src)
	{
		total=src.total;
	}
};


class CNodeTreeCtrl;

class CModManager;
class GuiLib_Api CNodeTreeCtrl:public CXTTreeCtrl
{
public:
	CNodeTreeCtrl();
	BOOL SetNodeTree(NodeTreeRef *treeref);
	NodeTreeRef *GetNodeTree()	{		return _treeref;	}
	BOOL UpdateNodeTree(NodeTreeRef *treeref);
	BOOL UpdateItem(NodeHandle hNode);
	BOOL Create(CWnd *pParent,RECT &rc,UINT id,DWORD StyleEx=0);//Create window
	void SetSsc(CSscSystemWrapper *ssc)	{	_ssc=ssc;	}
	CSscSystemWrapper *GetSsc()	{		return _ssc;	}

	void IncUpdateSsc();//递增更新Item的Ssc状态

	void EnableEdit(BOOL bEnable);
	void EnableRecursiveCheck(BOOL  bEnable){_bRecursiveCheck=bEnable;}

	void SetOwner(HWND hOwner)	{		_hOwner=hOwner;	}
	void NotifyOwner(UINT msg,DWORD_PTR param1,DWORD_PTR param2);
	NodeHandle GetCurSel();
	NodeHandle* GetCurSels(DWORD &c);

	BOOL UpdateShowName(NodeHandle hNode);


	HTREEITEM ItemFromPath(std::vector<std::string>&path);
	HTREEITEM ItemFromPath(std::string &path);
	HTREEITEM ItemFromNodeHandle(NodeHandle hNode);

	BOOL BuildContextMenu(CMenu *);

	BOOL IsChecked(HTREEITEM item);
	BOOL SetChecked(HTREEITEM item,BOOL bChecked); 
protected:

	NodeTreeRef *_treeref;

	virtual CNodeTree *_Tree()	
	{		
		if (_treeref)
			return _treeref->GetTree();
		return NULL;
	}

	void _AddNode(HTREEITEM hParent,NodeHandle hNode);
	void _UpdateNode(HTREEITEM hParent,NodeHandle hNode);
	void _UpdateItem(HTREEITEM hItem,NodeHandle hNode);
	void _CollapseItems(std::vector<HTREEITEM>&collapsed,std::vector<HTREEITEM>&total);



	//Some overidable
	virtual UINT _GetImageID()	{		return 0xffffffff;	}
	virtual DWORD _GetImageIdx(NodeHandle hNode,SscState state)=0;
	virtual BOOL _IsEditable()	
	{		
		if (_Tree())
		{
			if (_Tree()->IsReadOnly())
				return FALSE;
		}
		return _bEditable;	
	}
	virtual BOOL _IsExchangable()	{		return FALSE;	}//是否可以将两个sibling item交换位置,注意,如果_IsEditable()返回FALSE,即便_IsExchangable()返回TRUE,也不能作任何修改
	virtual BOOL _CanRename(NodeType type)	{		return TRUE;	}
	virtual BOOL _CanNew(NodeType type)	{		return TRUE;	}
	virtual BOOL _CanAutoGenUniqueName(NodeType type)	{		return TRUE;	}
	virtual void _ModifyEdit(NodeHandle hNode,std::string &str)	{	}//在编辑后,调用这个函数来对编辑框的字符串做一个修改,如果需要的话.缺省为不修改

	virtual void _OnCustomMenu(CMenu *menu)	{	}
	virtual std::string _GenNewName(NodeType type,const char *nameType);
	virtual BOOL _GenUniqueName(NodeType type,std::string &name);//如果名字与已有名字重复,会调用这个函数产生一个独一无二的名字
	virtual BOOL _OnFilterItem(NodeHandle hNode)	{		return FALSE;	}//返回TRUE使这个node 不被加到tree ctrl 里
	virtual BOOL _SupportCheckBox();

	virtual BOOL _PromptDelSel();

	virtual const char *_GetShowName(NodeHandle hNode);

	virtual BOOL _CanSscOp()	{		return TRUE;	}

	HTREEITEM _GetNextSscUpdate();


	CImageList _imgNode;

	//Selection
	void _RecordSel(ItemGroupDesc2 *sel);
	ItemGroupDesc2 _sel;

	//clipboard
	void _RecordClip();
	void _ClearClip();
	ItemGroupDesc2 _clip;//clipboard


	BOOL _bEditable;
	BOOL _bRecursiveCheck;

	HWND _hOwner;//Owner window,

	//for edit label
	BOOL _bNewItem;
	std::string _sEditItem;

	std::string _pathMod;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	virtual void OnNew(NodeType type);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCopy();
	afx_msg void OnCut();
	afx_msg void OnPaste();
	afx_msg void OnRename();
	afx_msg void OnDelete();
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	//modify by star.  2007-11-6
	//{
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	//}


protected:
	//modify by star .2007-11-6{
	void _TranslateItem(HTREEITEM  item,UINT state);
	BOOL  _CheckLegal(HTREEITEM  item,UINT  state);
	//}

	void _DeleteSels();
	BOOL _DeleteItemR(HTREEITEM hItem);
	void _NewItem(NodeType type,const char *name,void *param,HTREEITEM hParent=NULL);

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	////modify by star .2007-11-6{
	virtual void  _OnCheckStateChange(HTREEITEM  item,UINT state);
	//}

	//Ssc Support
	enum SscOp
	{
		None,
		CheckIn_,
		CheckIn_KeepOut,
		CheckOut,
		Get,
		Add,
		Remove,
		Refresh,
		GetFolder
	};
	NodeHandle _GetItemData(HTREEITEM item);
	SscState _GetItemSscState(HTREEITEM item);//得到cache的source safe状态,不一定精确
	SscState _GetActualItemSscState(HTREEITEM item);//得到真实的source safe状态,直接通过访问ssc得到
	void _ApplySscOp(HTREEITEM *items,DWORD nItems,SscOp op,BOOL bTest);
	void _ApplyItemSscOp(HTREEITEM item,SscOp op,BOOL bTest);
	void _FlushSscOp(SscOp op);
	BOOL _TestSscOp(HTREEITEM *items,DWORD nItems,SscOp op);
	virtual BOOL _OnSscOp(HTREEITEM *items,DWORD c,SscOp op,BOOL bTest);


	CSscSystemWrapper *_ssc;
	BOOL _bInSscOp;
	std::vector<std::string> _pathesSsc;
	std::vector<HTREEITEM>_itemsSsc;
	std::vector<std::string> _pathesSscFolder;

	DWORD _iSscUpdate;

	//Context Menu
	void _ClearMenu();
	std::vector<CMenu *>_menus;//_menus[0] 是主菜单,其它的是子菜单

	friend class CNtcMod_Remove;
	friend class CNtcMod_Change;
	friend class CNtcMod_Rename;
	friend class CXTTreeCtrl;
};


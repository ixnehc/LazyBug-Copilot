
#pragma once
#include "GuiLib.h"


#include "ResTree.h"

#include "NodeTreeCtrl.h"

#include "TreeCtrlBase.h"

class CResAnchorBase;

class IProtoLib;
class CProtoSelectDlg;

class CProtoSelTree:public CNodeTreeCtrl
{
public:
	CProtoSelTree()
	{
		_owner=NULL;
	}
protected:
	virtual UINT _GetImageID();
	virtual DWORD _GetImageIdx(NodeHandle hNode,SscState state);

	virtual BOOL _OnFilterItem(NodeHandle hNode);

	void SetProtoLib(IProtoLib *lib);

protected:

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);

	CProtoSelectDlg *_owner;

	int _flag;//0: default, 1: none-lua proto only , 2: lua proto only

	IProtoLib *_lib;

	friend class CProtoSelectDlg;

};
// CProtoSelectDlg 对话框
class GuiLib_Api CProtoSelectDlg : public CXTPDialog
{
// 构造
public:
	CProtoSelectDlg(CWnd* pParent = NULL);	// 标准构造函数

	BOOL Create(CWnd *pParent);

	void SetProtoLib(IProtoLib *lib)	{	_lib=lib;	}
	const char *GetSelPath()	{		return _pathSel.c_str();	}
	void SetSelPath(const char *path)	{		_pathSel=path;	}

	void ShowSelNone()	{		_bShowSelNone=TRUE;	}
	void SetLuaProtoOnly()	{		_tree._flag=2;	}
	void SetNoneLuaProtoOnly()	{		_tree._flag=1;	}

	static void SetForceReloadLib()	{		_bForceReload=TRUE;	}


	virtual void OnOK();
	virtual void OnCancel();

// 对话框数据
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持




// 实现
protected:
	CProtoSelTree _tree;
	IProtoLib *_lib;

	std::string _pathSel;

	BOOL _bShowSelNone;

	static TreeCtrlState _state;
	static BOOL _bForceReload;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSelnone();
};



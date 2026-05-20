#pragma once

#include "GuiLib.h"


#include "GuiEditor.h"

#include "GuiAgent_general.h"

#include "NodeTreeCtrl.h"



class IAssetSystem;
class GuiLib_Api CAssetLibTree:public CNodeTreeCtrl,public CNodeTree
{
public:
	CAssetLibTree()
	{
		_pAS=NULL;
		_bDrag=FALSE;
	}


	void SetContent(IAssetSystem *pAS);

	const char *GetCurSelHelp();


protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL  OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual UINT _GetImageID();
	virtual DWORD _GetImageIdx(NodeHandle hNode,SscState state);

	virtual BOOL _IsEditable()	{		return FALSE;	}

	//CNodeTree重载函数
	virtual void _OnInitType();
	virtual BOOL _OnCheckTypeRelation(NodeType typeParent,NodeType typeChild);//return whether typeChild could be under typeParent
	virtual NodePtr _OnNew(const char *path,NodeType type,void *param);
	virtual BOOL _OnDelete(NodeHandle hNode);

	virtual const char *_GetSep()	{		return ".";	}
	virtual const char*_GetShowName(NodeHandle hNode,const char *nameOrg);


	IAssetSystem *_pAS;
	std::unordered_map<std::string,CClass *>_clsses;

	BOOL _bDrag;
	std::string _sDrag;//当_bDrag为TRUE时有效,存储了选中的asset名称

};


class CSscSystemWrapper;
class IProtoLib;
class CGuiPanel_Proto;
class GuiLib_Api CGuiPanel_AssetLib:public CGuiPanel
{
public:
	CGuiPanel_AssetLib(CWnd* pParent = NULL);
	virtual const char *GetName()	{		return "assetlib";	}

	BOOL Create(CWnd *pParent);

	virtual void Reset();

	virtual void UpdateUI();


protected:

	void _RecalcLayout();

	CAssetLibTree _tree;


	GStubBegin(CGuiPanel_AssetLib);

	GStubEnd();


public:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

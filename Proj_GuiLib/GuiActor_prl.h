#pragma once

#include "GuiLib.h"

#include "WorldSystem/IEntitySystemDefines.h"


#include "GuiEditor.h"

#include "GuiAgent_general.h"

#include "NodeTreeCtrl.h"
#include "SscBtn.h"

#include "GObjGrid.h"

#include "gds/GStub.h"
#include "gds/GProp.h"

#include "filewatcher/FileWatcher.h"

class IProto;
GuiLib_Api void EnumProtoRes(IProto *proto,std::vector<std::string>&buf);

class IWorldSystem;
class IProtoLib;
class GuiLib_Api CPrlTree:public CNodeTreeCtrl
{
public:
	CPrlTree()
	{
		_lib=NULL;
		_pWS=NULL;
		_pES=NULL;
		_bDrag=FALSE;
	}


	void SetES(IEntitySystem*pES);
	void SetLib(IProtoLib *lib);

	void UpdateStartup(const char *path);

	virtual BOOL _OnSscOp(HTREEITEM *items,DWORD c,SscOp op,BOOL bTest);


protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSetStartup();
	afx_msg void OnClearStartup();
	afx_msg void OnRefresh();
	afx_msg void OnCopyPath();
	afx_msg void OnCopyIDPath();
	afx_msg void OnRefRes();
	afx_msg void OnBrowseFolder();
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL  OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual UINT _GetImageID();
	virtual DWORD _GetImageIdx(NodeHandle hNode,SscState state);

	virtual BOOL _IsEditable();
	virtual BOOL _IsExchangable()	{		return TRUE;	}
	virtual BOOL _CanRename(NodeType type);
	virtual BOOL _CanNew(NodeType type);
	virtual BOOL _CanAutoGenUniqueName(NodeType type);
	virtual std::string _GenNewName(NodeType type,const char *nameType);
	virtual BOOL _GenUniqueName(NodeType type,std::string &name);
	virtual void _ModifyEdit(NodeHandle hNode,std::string &str);

	virtual void _OnCustomMenu(CMenu *menu);
	virtual BOOL _CanSscOp()	{		return _IsEditable();	}
	void _Add(BOOL bNew,BOOL bLua);

	void _ClearBold(HTREEITEM hParent,std::vector<HTREEITEM>&bolds);

	void _UpdateCursor();

	std::string _GetProtoPath();
	std::string _GetProtoIDPath();

	IProtoLib *_lib;
	IEntitySystem *_pES;

	IWorldSystem *_pWS;

	BOOL _bDrag;
	std::string _sDrag;//当_bDrag为TRUE时有效,存储了选中的proto的路径,以","分隔


};


class CSscSystemWrapper;
class IProtoLib;
class CGuiPanel_Proto;
class GuiLib_Api CGuiPanel_Prl:public CGuiPanel
{
public:
	CGuiPanel_Prl(CWnd* pParent = NULL);
	virtual const char *GetName()	{		return "protolib";	}

	BOOL Create(CWnd *pParent);

	virtual void Reset();

	virtual void UpdateUI();

	void SetStartupProto(const char *path);

	BOOL CanMakeThumbnail(const char *path);

	void EnsureVisible(ProtoID idProto);


protected:

	void _RecalcLayout();

	CPrlTree _tree;
	CXTButton _btnSync;


	CFileWatcher _watcher;


	GStubBegin(CGuiPanel_Prl);
		
		GStubString(DblClickProto,"",GSem_Unknown,"");
			GStubSetType(GStub_Signal);
		GStubVoid(LibModified,"");
			GStubSetType(GStub_Signal);

	GStubEnd();

	BOOL prop_DblClickProto(BOOL bSet,const char *&path)	{		return FALSE;	}
	BOOL prop_LibModified(BOOL bSet)	{		return FALSE;	}

public:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnPrlTreeDblClk(WPARAM wParam,LPARAM lParam);
	afx_msg void OnSync();
};

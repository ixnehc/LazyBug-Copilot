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

#include "MultiTree/NodeTree.h"

#include "database/VcxprojDatabaseCore.h"

class GuiLib_Api CDbTreeCtrl :  public CNodeTreeCtrl,public CNodeTree
{
public:
	CDbTreeCtrl();
	virtual ~CDbTreeCtrl();

	void SetDatabase(CVcxprojDatabaseCore* pDB);

	AbsTick FetchOpenSelRequest()
	{
		AbsTick ret = _requestOpenSelTime;
		_requestOpenSelTime = 0;
		return ret;
	}

	bool EnsureVisible(const char* fullPath);

protected:
	DECLARE_MESSAGE_MAP()

	UINT _GetImageID() override;
	DWORD _GetImageIdx(NodeHandle hNode, SscState state) override;
	BOOL _IsEditable() override;
	BOOL _CanRename(NodeType type)override;
	BOOL _CanNew(NodeType type)override;

	// CNodeTree 虚函数实现
	virtual BOOL _OnCheckTypeRelation(NodeType typeParent, NodeType typeChild) override;
	virtual const char* _GetSep() override;
	virtual void _OnInitType() override;
	virtual NodePtr _OnNew(const char* path, NodeType type, void* param) override;
	virtual BOOL _OnDelete(NodeHandle hNode) override;

	//
	afx_msg void OnNMClick(NMHDR* pNMHDR, LRESULT* pResult);

private:
	CVcxprojDatabaseCore* _pDB;

	AbsTick _requestOpenSelTime;
};

class CSscSystemWrapper;
class IProtoLib;
class CGuiPanel_Proto;
class GuiLib_Api CGuiPanel_Db:public CGuiPanel
{
public:
	CGuiPanel_Db(CWnd* pParent = NULL);
	virtual const char *GetName()	{		return "protolib";	}

	BOOL Create(CWnd *pParent);

	virtual void Reset();

	virtual void UpdateUI();

	void EnsureVisible(ProtoID idProto);

	const char *GetCurSelPath();

	CDbTreeCtrl& GetTree()	{		return _tree;	}

protected:

	void _RecalcLayout();

	CDbTreeCtrl _tree;

	CFileWatcher _watcher;

	GStubBegin(CGuiPanel_Db);
		
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
	afx_msg LRESULT OnDbTreeDblClk(WPARAM wParam,LPARAM lParam);
};

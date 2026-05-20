#pragma once

#include "GuiLib.h"


#include "GuiEditor.h"

#include "GuiAgent_general.h"
#include "GuiAgent_proto.h"

#include "NodeTreeCtrl.h"
#include "SscBtn.h"

#include "GObjGrid.h"
#include "GPropGrid.h"

#include "BehaviorGraphGrid.h"

#include "GuiAgent_MatrixEdit.h"
#include "behaviorgraph/BehaviorValue.h"
#include "behaviorgraph/BehaviorGraphsUtil.h"

struct ProtoNodeData
{
	ProtoNodeID id;
	i_math::pos2di pos;
	std::string path;
	NodeType type;
	std::vector<BYTE> data;
};
struct PNConnData
{
	ProtoNodeID id[2];
	std::string name[2];
};

struct PNStubData
{
	std::string name;
	ProtoNodeID idInner;
	std::string nameInner;
	i_math::pos2di pos;//graph pos
};

class CPNClipboard
{
public:
	~CPNClipboard();
	void Clear();
	void Add(ProtoNodeID id,const char *path,NodeType type,BYTE *data,DWORD szData,i_math::pos2di &pos);
	void SafeAddConn(PNConnect &conn);//加入的connect的src和target node必须都存在于_dataes里
	void SafeAddStub(ProtoStubInfo&stub);//加入的stub的proto node必须存在于_dataes里
	DWORD GetCount()	{		return _dataes.size();	}
	ProtoNodeData*Get(DWORD idx);
	DWORD GetConnCount()	{		return _connes.size();	}
	PNConnData *GetConn(DWORD idx)	{		return _connes[idx];	}
	DWORD GetStubCount()	{		return _stubs.size();	}
	PNStubData *GetStub(DWORD idx)	{		return _stubs[idx];	}

protected:
	std::vector<ProtoNodeData*>_dataes;
	CMemPool<ProtoNodeData> _poolNode;
	std::vector<PNConnData*>_connes;
	CMemPool<PNConnData> _poolConn;
	std::vector<PNStubData*>_stubs;
	CMemPool<PNStubData> _poolStub;
};


class IProtoLib;
class GuiLib_Api CProtoTree:public CNodeTreeCtrl
{
public:
	CProtoTree()
	{
		_lib=NULL;
		_protoid=ProtoID_Null;

		_ver=0;
	}
	~CProtoTree()
	{
		_ClearSubMenus();
	}

	virtual void OnNew(NodeType type);

	void SetLib(IProtoLib *lib)	{		_lib=lib;	}
	void Bind(ProtoID id);

	void NewProtoes(i_math::pos2di &ptStart,const char *pathes,BOOL bAssetOrProto);

    static std::string GenNewAutoName();
    static std::string GenUniqueAutoName();

protected:
	virtual UINT _GetImageID();
	virtual DWORD _GetImageIdx(NodeHandle hNode,SscState state);
	virtual void _OnCustomMenu(CMenu *menu);
	virtual std::string _GenNewName(NodeType type,const char *nameType);
	virtual BOOL _GenUniqueName(NodeType type,std::string &name);//如果名字与已有名字重复,会调用这个函数产生一个独一无二的名字
	virtual BOOL _IsExchangable()	{		return TRUE;	}
	virtual void _ModifyEdit(NodeHandle hNode,std::string &str);

	virtual BOOL _PromptDelSel();


	IProtoNode *_GetSelProtoNode();
	IProtoNode **_GetSelProtoNodes(DWORD &count);

	void _ClearSubMenus();

	IProtoLib *_lib;

	ProtoID _protoid;
	DWORD _ver;

	static CPNClipboard _cbPN;
	std::vector<IProtoNode*> _temp;//for _GetSelProtoNodes(..)
	std::vector<CMenu*>_menuAllocs;
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDynamic();
	afx_msg void OnInvDynamic();
	afx_msg void OnVirtual();
	afx_msg void OnInvVirtual();
	afx_msg void OnLab();
	afx_msg void OnInvLab();
	afx_msg void OnDeferredGrp(UINT idCmd);
	afx_msg void OnInvDeferred();
	afx_msg void OnEditHelper();
	afx_msg void OnInvEditHelper();
	afx_msg void OnCopy();
	afx_msg void OnCut();
	afx_msg void OnPaste();



};

struct NodeRGState
{
	DEFINE_CLASS(NodeRGState);
	NodeRGState()
	{
		nodeid=ProtoNodeID_Null;
		t=0;
	}
	ProtoNodeID nodeid;
	RGState state;
	DWORD t;
};

struct ProtoRGState
{
	DEFINE_CLASS(ProtoRGState);

	~ProtoRGState()
	{
		Clear();
	}


	void Clear()
	{
		for (int i=0;i<entries.size();i++)
			Safe_Class_Delete(entries[i]);
		entries.clear();
	}
	NodeRGState *ObtainRGState(ProtoNodeID nodeid)
	{
		for (int i=0;i<entries.size();i++)
		{
			if (entries[i]->nodeid==nodeid)
				return entries[i];
		}
		NodeRGState *p=Class_New2(NodeRGState);
		p->nodeid=nodeid;
		entries.push_back(p);
		return p;
	}

	std::vector<NodeRGState*>entries;
};


class GuiLib_Api CProtoNodePage:public CGPropGrid
{
public:
	CProtoNodePage()
	{
		Zero();
	}
	~CProtoNodePage()
	{
		_ClearNodeRGStates();
	}
	void Zero()
	{
		_id=ProtoID_Null;
		_nodeid=ProtoNodeID_Null;
		_ver=0;
		_lib=NULL;
		_mgr=NULL;
	}
	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle = LBS_OWNERDRAWFIXED| LBS_NOINTEGRALHEIGHT);

	void Reset();
	void SetLib(IProtoLib *lib)	;
	void SetMgr(CGuiMgr *mgr);
	void Bind(ProtoID id,ProtoNodeID nodeid);

	virtual void OnBeginItemChange(CXTPPropertyGridItem *item);
	virtual void OnItemChange(CXTPPropertyGridItem *item);
	virtual void OnEndItemChange(CXTPPropertyGridItem *item);

	virtual void RedrawDueToItemChange();

protected:
	void _Bind(BOOL bUpdate);

	NodeRGState*_ObtainNodeRGState(ProtoID id,ProtoNodeID nodeid);
	void _ClearNodeRGStates();
	void _RecordNodeRGState();
	BOOL _RestoreNodeRGState();
	virtual BOOL _InsertElem(GObjBase *obj,GElemBase *elem);

	ProtoNodeID _nodeid;
	ProtoID _id;
	DWORD _ver;

	IProtoLib *_lib;

	CGuiMgr *_mgr;

	std::unordered_map<ProtoID,ProtoRGState*>_rgstates;

	BOOL _bChanging;

	CBehaviorGraphUtil _util;
	std::deque<BhvValuesCache> _cachesBhvValues;

public:
	DECLARE_MESSAGE_MAP()
};

typedef fastdelegate::FastDelegate2<ProtoNodeID,DWORD> OpenLuaSrcCallBack;

class CGuiPanel_Proto;
class GuiLib_Api CGuiActor_Proto:public CGuiActor
{
public:
	CGuiActor_Proto();
	~CGuiActor_Proto();
	virtual CWnd *GetWnd();
	virtual const char *GetName()	{		return "proto";	}

	void SetPanel(CGuiPanel_Proto *panel)	{		_panel=panel;	}

	virtual void Reset();
	
	virtual void UpdateUI();

	virtual void DoCommand(DWORD idCmd);

	void OnLostPanel();
	void OnOccupyPanel();

	BOOL BuildProtoTreeMenu(CMenu *menu);
	void SendProtoTreeCmd(DWORD idCmd);

	BOOL ReplaceProto(std::vector<BYTE>&data,BOOL bTest);


protected:
	CGuiPanel_Proto *_panel;

	virtual const char *_GetModMgrName()	{		return "proto";	}

	CGuiAgent_PNMatrixEdit _matedit;
	CGuiAgent_TestProto _tester;
	CGuiAgent_ThumbnailMake _thumbnailmaker;
	CGuiAgent_CameraController<DRAG_BUTTON_MIDDLE,0,DRAG_BUTTON_RIGHT,0>*_controller;
	i_math::matrix43f _matWork;
	ProtoNodeID _nodeidSel;

	i_math::vector3df _posFocus;


};



class IProtoLib;
struct GuiData_Proto;
class GuiLib_Api CGuiPanel_Proto:public CGuiPanel
{
public:
	CGuiPanel_Proto(CWnd* pParent = NULL);
	virtual const char *GetName()	{		return "proto";	}

	BOOL Create(CWnd *pParent);


	virtual void UpdateUI();

	void BindActor(CGuiActor_Proto *actor);

	void InvalidateView();

	CProtoTree *GetTree()	{		return &_tree;	}
	CProtoNodePage *GetPage()	{		return &_page;	}

	BOOL IsChangingProto();


protected:
	virtual const char *_GetModMgrName()	{		return "proto";	}

	void _RecalcLayout();
	CGuiMgr *_CurMgr();

	CProtoTree _tree;
	CProtoNodePage _page;
	CGuiActor_Proto *_actor;



	BOOL _bInUpdateTreeSel;
	BOOL _UpdateTreeSel(HTREEITEM hItem,GuiData_Proto*data);//返回selection有没有任何变化


public:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnNMDblclkTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);
};


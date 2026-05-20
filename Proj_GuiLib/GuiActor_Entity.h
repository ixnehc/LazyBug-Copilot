#pragma once

#include "GuiLib.h"

#include "FileSystem/IMapFileDefines.h"
#include "WorldSystem/IEntitySystemDefines.h"


#include "GuiData.h"
#include "GuiEditor.h"

#include "GuiActor_Prl.h"
#include "GuiActor_proto.h"

#include "GuiAgent_general.h"

#include "GuiAgent_3dnodeedit.h"

#include "NodeTreeCtrl.h"

#include "GPropGrid.h"

#include "ModBlockBack.h"
	
#include "FileWatcher/FileWatcher.h"

#include "behaviorgraph/BehaviorValue.h"
#include "behaviorgraph/BehaviorGraphsUtil.h"


#include <set>

class CMod_InvalidateView:public CModBase
{
public:
	CMod_InvalidateView(CGuiView *view)
	{
		_view=view;
	}

	virtual void DeleteThis()	{		delete this;	}

	virtual BOOL IsEmpty()	{		return FALSE;	}
	virtual void Clear()	{		_view=NULL;	}

	virtual BOOL Undo()
	{
		_view->Invalidate();
		return TRUE;
	}
	virtual BOOL Redo()
	{
		return Undo();
	}

protected:
	CGuiView *_view;

};

struct GuiData_EntityMap;
class CMod_ChangeEntityMap:public CModBlockBack
{
public:
	CMod_ChangeEntityMap(CGeView * view);
	void BackupSelection();
protected:

	virtual void OnBackup();
	virtual void OnRestore();

};


class CGuiAgent_ResideEntity:public CGuiAgent
{
public:
	CGuiAgent_ResideEntity()
	{
		_entity=NULL;
		_protoid=ProtoID_Null;
		_bHold=FALSE;
		_angle=0.0f;
		_scale=1.0f;
		_offVer=0.0f;
		_nIgnore=0;
		_bRandomRotate=FALSE;
		_angleRandom=0.0f;

		_bMoingVer=FALSE;

		_bResideOnGround=FALSE;
		_bAutoAlign=FALSE;
	}
	void SetProtoID(ProtoID protoid)	{		_protoid=protoid;	}
	virtual BOOL OnTimer(int dt,DWORD flag);
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnMouseWheel(int delta,DWORD flag);
	virtual BOOL OnKeyDown(char c,DWORD flag);
	virtual BOOL OnDraw();

	virtual BOOL OnSetCursor(int x,int y,DWORD flag);
	virtual void OnDetachView(CGeView *view,DWORD iLevel);


	void SetRandomRotate(BOOL bRandom)	{		_bRandomRotate=bRandom;	}
	void SetResideOnGround(BOOL bResideOnGround)	{		_bResideOnGround=bResideOnGround;	}
	void SetAutoAlign(BOOL bAutoAlign)	{		_bAutoAlign=bAutoAlign;	}

protected:
	BOOL _FindResidePos(i_math::pos2di &ptCursor,i_math::matrix43f &matReside);
	ProtoID _protoid;
	IEntity *_entity;

	float _angle;
	i_math::vector3df _rotAxis;

	float _scale;
	float _offVer;
	i_math::quatf _rotCache;

	BOOL _bHold;
	BOOL _bCtrlOrShift;//在_bHold时有效
	void _ModResideMat(i_math::matrix43f &mat);
	IEntity * _TryAlign(i_math::matrix43f &mat,IEntity *en);
	i_math::pos2di _ptHoldStart;
	i_math::pos2di _ptCursorPosStart;
	float _angleStart;
	float _scaleStart;

	BOOL _bMoingVer;

	BOOL _bRandomRotate;
	float _angleRandom;

	BOOL _bResideOnGround;
	BOOL _bAutoAlign;

	int _nIgnore;
};

class CGuiAgent_OperateEntity:public CGuiAgent_3DNodeOperate
{
public:
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnLButtonDblClk(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

protected:
	virtual  void*_GetSelBuf();
	virtual i_math::pos2di *_GetBlock(H3DNode node);
	virtual H3DNode _HitTest(i_math::line3df &ray);
	virtual BOOL _Remove(H3DNode node);
	virtual void _CollectEnvelope(H3DNode *nodes,DWORD nNodes,Envelope &evlp)	;

	virtual BOOL _NeedClone()	{		return TRUE;	}

	virtual H3DNode _Clone(H3DNode node);

	BOOL _GetAgentInfo(RecordID &idMap,std::string &nmMap,RecordID &idAgent,std::string &nmAgent,DWORD &guid);


	i_math::pos2di _ptTemp;
	
};

class CGuiAgent_EntityRectSel:public CGuiAgent_Dragger<TRUE,0>
{
public:
	CGuiAgent_EntityRectSel()
	{
		_bAccum=FALSE;
	}
	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);
	virtual BOOL OnDraw();

protected:
	void _Sel(ProtoNodeID *inrects,DWORD c);
	std::vector<EntityAddress>_initials;
	i_math::pos2di _start;
	i_math::recti _rcDraw;
	BOOL _bAccum;


};


class CModBlockBack;
class CGuiAgent_EntityMatrixEdit:public CGuiAgent_3DNodeMatEdit
{
public:
protected:
	virtual  void*_GetSelBuf();
	virtual i_math::matrix43f *_GetMat(H3DNode node);
	virtual i_math::pos2di *_GetBlock(H3DNode node);//返回这个node位于那个block内

	virtual void _Move(H3DNode &node,i_math::matrix43f &mat);

	i_math::pos2di _ptTemp;

};

class CGuiAgent_EntityTrrnImprint:public CGuiAgent
{
public:
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

protected:
	void _DoImprint(ITrrnMapEditor *editor,IMeshSnapshot *ms,i_math::vector3df *vtxs,DWORD nVtx,WORD *idxs,DWORD nIdx,const char *mode);

	std::set<i_math::pos2di> _mods;

};

class GuiLib_Api CEntityPage:public CGPropGrid
{
public:
	CEntityPage()
	{
		Zero();
	}
	void Zero()
	{
		_modmgr=NULL;
		_mp=NULL;
		_addr=EntityAddress_Null;
	}
	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle = LBS_OWNERDRAWFIXED| LBS_NOINTEGRALHEIGHT);

	void Reset();
	void SetModMgr(CModManager *modmgr)	{		_modmgr=modmgr;	}
	void SetEntityMap(IEntityMap *mp)	{		_mp=mp;	}
	void Bind(EntityAddress addr,BOOL bForceRebind);

	virtual void OnBeginItemChange(CXTPPropertyGridItem *item);
	virtual void OnItemChange(CXTPPropertyGridItem *item);
	virtual void OnEndItemChange(CXTPPropertyGridItem *item);


protected:
	virtual BOOL _InsertElem(GObjBase *obj,GElemBase *elem);


	void _ApplyMod();


	EntityAddress _addr;
	CModManager *_modmgr;
	IEntityMap *_mp;

	void _ClearCache();

	CBehaviorGraphUtil _util;
	std::vector<GProperty *>_cache;
	std::deque<BhvValuesCache> _cachesBhvValues;

public:
	DECLARE_MESSAGE_MAP()

};

class CProtoThumbnailsList:public CXTListCtrl
{
public:
	CProtoThumbnailsList()
	{
		_hSel=NodeHandle_Null;
	}
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);

	void ResetContent()	{		_ClearContent();	_hSel=NodeHandle_Null;}

	void Update();

	NodeHandle GetCurSel()
	{
		return _GetCurSel();
	}


public:
	DECLARE_MESSAGE_MAP()

	void _ClearContent();

	NodeHandle _GetCurSel();
	void _SetCurSel(NodeHandle h);

	NodeHandle _hSel;
	std::vector<NodeHandle> _handlesItem;

	CFont _font;
	CImageList _il;


};



class IAssetClassLib;
class GuiLib_Api CGuiPanel_Entity:public CGuiPanel
{
public:
	CGuiPanel_Entity(CWnd* pParent = NULL);
	virtual const char *GetName()	{		return "entity";	}

	BOOL Create(CWnd *pParent);

	virtual void Reset();
	virtual void UpdateUI();

	virtual void OnDetachView(CGeView *view,DWORD iLevel);

	virtual void OnEnterActivity();

	void SetForceBind()	{		_bForceBind=TRUE;	}

	CPrlTree &GetPrlTree()	{		return _tree;	}

protected:
	virtual const char *_GetModMgrName()	{		return "world";	}

	void _OccupyActor();

	void _AddAffectBlock(i_math::matrix43f &mat);
	void _ClearAffectBlock();
	std::vector<i_math::pos2di> _affected;//修改时影响到的block

	CFileWatcher _watcher;

	CPrlTree _tree;
	CEntityPage _page;
	CProtoThumbnailsList _tbns;

	CGuiAgent_ResideEntity _resider;

	CGuiAgent_EntityMatrixEdit *_mateditor;

	CXTButton _btnSync;

	BOOL _bLibDirty;


	BOOL _bForceBind;

public:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnReside();
	afx_msg void OnReloadProtoLib();
	afx_msg void OnRepairGUID();
	afx_msg LRESULT OnPrlTreeDblClk(WPARAM wParam,LPARAM lParam);
	afx_msg void OnSync();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	ProtoID _GetCurSelProto();

	void _AdjustCtrlWidth(CWnd &wnd,int wDlg);


	BOOL _bResiding;


};

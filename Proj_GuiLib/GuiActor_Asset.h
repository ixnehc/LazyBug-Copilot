#pragma once

#include "GuiLib.h"


#include "GuiData.h"
#include "GuiEditor.h"

#include "GuiAgent_general.h"

#include "NodeTreeCtrl.h"

#include "GObjGrid.h"

#include "GuiActor_Acl.h"

#include "gds/GStub.h"
#include "gds/GProp.h"

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

class CMod_ChangeAssetMap:public CModBase
{
public:
	CMod_ChangeAssetMap(GuiData_AssetMap *data)
	{
		_data=data;
	}
	virtual void DeleteThis()	{		delete this;	}

	virtual BOOL IsEmpty()	{		return FALSE;	}
	virtual void Clear();

	virtual BOOL Undo();
	virtual BOOL Redo();

	BOOL BackupBlock(i_math::pos2di &ptBlk);
	void BackupSelection();

protected:
	struct _BlockData
	{
		std::vector<BYTE> data;
		i_math::pos2di ptBlk;

		_BlockData &operator=(const _BlockData &src)
		{
			data=src.data;
			ptBlk=src.ptBlk;
			return *this;
		}
	};
	

	std::vector<_BlockData> _blocks;
	std::vector<AssetAddress> _selections;

	GuiData_AssetMap *_data;

};


class CGuiAgent_ResideAsset:public CGuiAgent
{
public:
	CGuiAgent_ResideAsset()
	{
		_ast=NULL;
		_clssid=AssetClassID_Null;
	}
	void SetClassID(AssetClassID clssid)	{		_clssid=clssid;	}
	virtual BOOL OnTimer(int dt,DWORD flag);
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnDraw();

	virtual BOOL OnSetCursor(int x,int y,DWORD flag);
	virtual void OnDetachView(CGeView *view,DWORD iLevel);


protected:
	AssetClassID _clssid;
	IAsset *_ast;
};

class CGuiAgent_DrawSelAsset:public CGuiAgent
{
public:
	CGuiAgent_DrawSelAsset()
	{
	}
	virtual BOOL OnDraw();


protected:
};

class CGuiAgent_SelectAsset:public CGuiAgent
{
public:
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnRButtonDown(int x,int y,DWORD flag);

	virtual BOOL OnSetCursor(int x,int y,DWORD flag);

protected:
	BOOL _Select(int x,int y,DWORD flag);
	
};

class CGuiAgent_RemoveAsset:public CGuiAgent
{
public:
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

protected:
	void _Remove();
};


class CAssetDataCategory:public CXTPPropertyGridItem
{
public:
	CAssetDataCategory(const char *cap):CXTPPropertyGridItem(cap)
	{
		_ast=NULL;
	}
	virtual BOOL IsInplaceButtonsVisible() const
	{
		if (!m_pGrid)
			return FALSE;
		return TRUE;
	}

	void Bind(IAsset *ast,const char *name)
	{
		_ast=ast;
		_name=name;
	}

	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);


protected:
	IAsset *_ast;
	std::string _name;

};

class GuiLib_Api CAssetPage:public CGObjGrid
{
public:
	CAssetPage()
	{
		Zero();
	}
	void Zero()
	{
		_modmgr=NULL;
		_amap=NULL;
		_addr=AssetAddress_Null;
	}
	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle = LBS_OWNERDRAWFIXED| LBS_NOINTEGRALHEIGHT);

	void Reset();
	void SetModMgr(CModManager *modmgr)	{		_modmgr=modmgr;	}
	void SetAssetMap(IAssetMap *amap)	{		_amap=amap;	}
	void Bind(AssetAddress addr,BOOL bForceRebind);

	virtual void OnBeginItemChange(CXTPPropertyGridItem *item);
	virtual void OnItemChange(CXTPPropertyGridItem *item);
	virtual void OnEndItemChange(CXTPPropertyGridItem *item);

protected:

	void _ApplyMod();

	AssetAddress _addr;
	CModManager *_modmgr;
	IAssetMap *_amap;

public:
	DECLARE_MESSAGE_MAP()

	friend class CAssetDataCategory;
};



class IAssetClassLib;
class GuiLib_Api CGuiPanel_Asset:public CGuiPanel
{
public:
	CGuiPanel_Asset(CWnd* pParent = NULL);
	virtual const char *GetName()	{		return "assetclasslib";	}

	BOOL Create(CWnd *pParent);

	virtual void Reset();
	virtual void UpdateUI();

	virtual void OnDetachView(CGeView *view,DWORD iLevel);

	void SetForceBind()	{		_bForceBind=TRUE;	}



protected:
	virtual const char *_GetModMgrName()	{		return "world";	}

	void _RecalcLayout();

	CAclTree _tree;
	CAssetPage _page;

	CGuiAgent_ResideAsset _resider;

	BOOL _bForceBind;

	std::set<AssetClassID>_acsRemoved;


public:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnReside();


	BOOL _bResiding;

	GStubBegin(CGuiPanel_Asset,1)
		GStubDefine("inAclMod",Prop_Void,Prop_SetAclMod,NULL);
	GStubEnd();

	BOOL Prop_SetAclMod(Prop_Void &prop);

	friend class CAssetClassPage;
};

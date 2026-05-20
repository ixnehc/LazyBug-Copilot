

#pragma once
#include "GuiLib.h"

#include <vector>


#include "RichGrid.h"
#include "MtrlGrid.h"

#include "gds/GDefines.h"

class IRenderSystem;
class IWorldSystem;
class IEntitySystem;
class ISscSystem;

struct GObjBase;
struct GElemBase;

struct ElemEntry
{
	ElemEntry()
	{
		item=NULL;
		owner=NULL;
		elem=NULL;
		iSub=-1;
		mask=0;
	}
	CXTPPropertyGridItem *item;
	void *owner;
	GElemBase *elem;
	int iSub;
	DWORD mask;//ID_RGIB_XXXX
};

struct MtrlDataEntry
{
	CXTPPropertyGridItem *item;
	MtrlData *md;
};

class CGObjGrid;
class CRemoteAgent_MatSet
{
public:
	CRemoteAgent_MatSet()
	{
		_owner=NULL;
		_ver=0;
		_uid=0;
	}
	void Init(CGObjGrid *owner)
	{
		_owner=owner;
	}
	void Start(const char *pathItem);
	void Update();

protected:

	CGObjGrid *_owner;
	DWORD _uid;
	std::string _pathItem;

	DWORD _ver;


	std::vector<BYTE>_bufTemp;

};

class GuiLib_Api CGObjGrid:public CRichGrid
{
public:
	CGObjGrid()
	{
		_iCurElemEntry=-1;
		_itemCur=NULL;
		_idTimer=0;
	}

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle = LBS_OWNERDRAWFIXED| LBS_NOINTEGRALHEIGHT);
	virtual void Bind(GObjBase **objs,DWORD count);
	virtual void Bind(GObjBase *obj);
	void RenderDelta(std::vector<void*> ptrs);

	void ResetContent();

	CXTPPropertyGridItem *InsertGObjVectorItem(const char *cap,const char *desc,
													GElemBase *elem,void *owner,DWORD IDs,DWORD IDsOfSub);
	CXTPPropertyGridItem *InsertGObjSubItem(const char *cap,const char *desc,
												GElemBase *elem,void *owner,DWORD iSub,DWORD IDs,const char *brief="");

	void InsertMtrlDataLod(MtrlData::Lod&lod,DWORD iLod);
	CXTPPropertyGridItem *InsertMtrlDataItem( const char* cap, const char* desc, MtrlData*p);

	virtual void OnBeginItemChange(CXTPPropertyGridItem *item);
	virtual void OnItemChange(CXTPPropertyGridItem *item);
	virtual void OnEndItemChange(CXTPPropertyGridItem *item);

	//重载这个函数用来截获XTP_PGN_DBLCLICK
	virtual LRESULT SendNotifyMessage(WPARAM wParam = 0, LPARAM lParam = 0);

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnGridNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClone();
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnRemove();
	afx_msg void OnCopyElem();
	afx_msg void OnPasteElem();
	afx_msg void OnPasteAgent();
	afx_msg void OnCollapseAll();
	afx_msg void OnExpandAll();
	afx_msg void OnLocateFile();
    afx_msg void OnOpenRes();
	afx_msg void OnOpenProto();
	afx_msg void OnOpenRecord();
	afx_msg void OnAnimNone();
	afx_msg void OnAnimValueSet();
	afx_msg void OnAnimRes();
	afx_msg void OnTimer(UINT_PTR idEvent);
	afx_msg void OnDestroy();

public://take as protected

	void _Bind(GObjBase *obj,GSem &sem,const char *capOverride="");
	void _BindElem(GObjBase *obj,GElemBase *elem,const char *capOverride,std::unordered_map<std::string,std::string>&overrides);
	void _AddElemSub(CXTPPropertyGridItem *item,void *owner,GElemBase *elem,int iSub,DWORD mask);
	virtual BOOL _InsertElem(GObjBase *obj,GElemBase *elem)	{		return FALSE;	}
	virtual void _OnContextMenu(CXTPPropertyGridItem *item,CMenu &menu){}
	virtual BOOL _OnDblClickOnItem(CXTPPropertyGridItem *item){ return FALSE;}//return whether handled the dbl click

	void _RecordElemEntry(GElemBase *elem,void *owner,int iSub=-1);

	MtrlData *_MtrlDataFromItem(CXTPPropertyGridItem *item);
	void _RepairMtrlData(CXTPPropertyGridItem *item);
	void _SwitchAnimType(int at);

	virtual BOOL _NeedRemoteMatSetEdit()	{		return FALSE;	}

	void _RenderDelta(CXTPPropertyGridItem *item);

	std::vector<GObjBase *> _objs;
	RGState _state;

	CXTPPropertyGridItem * _itemCur;

	std::vector<ElemEntry> _elems;
	int _iCurElemEntry;

	std::vector<MtrlDataEntry> _mds;
	std::map<CXTPPropertyGridItem*,MtrlDataItemInfo> _mditems;

	ElemEntry _cursub;

	CRemoteAgent_MatSet _agentRemote;
	UINT _idTimer;
};

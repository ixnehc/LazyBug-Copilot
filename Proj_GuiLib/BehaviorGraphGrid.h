
#pragma once
#include "GuiLib.h"

#include "resource.h"
#include <vector>

#include "resdata/BehaviorGraphData.h"
#include "behaviorgraph/behaviordefines.h"
#include "behaviorgraph/BehaviorValue.h"

#include "ResEditCtrl.h"

#include "GObjGrid.h"



class CResEditPanel;
struct ResEditPanelState;

struct Reps_BehaviorGraph;
struct BehaviorGraphData;
class CBehaviorGraphEditPanel;
struct LoAgentRef;

struct BhvValuesCache
{
	BhvValuesCache()
	{
		values=NULL;
	}

	struct Bind
	{
		Bind()
		{
			pad=NULL;
			elem=NULL;
			bDef=FALSE;
			nm=StringID_Invalid;
			nmRef=StringID_BhvValInvalidRef;
			value=NULL;
		}
		BOOL bDef;
		BhvVal *value;
		StringID nm;
		BhvValType tp;
		StringID nmRef;
		CBehaviorGraphPad *pad;
		GElemBase *elem;
	};

	void Zero()
	{
		values=NULL;
	}

	void Clear();

	void LoadBind(CBehaviorGraphPads &pads);
	void SaveBind();

	//原始信息
	BhvValues *values;
	std::vector<BhvValDeclare*> declares;

	//Binds
	std::vector<BhvVal> valuesDef;
	std::vector<Bind> binds;
	std::vector<CXTPPropertyGridItem *> items;

	Bind*BindFromItem(CXTPPropertyGridItem *item)
	{
		for (int i=0;i<items.size();i++)
		{
			if (items[i]==item)
			{
				if (i<binds.size())
					return &binds[i];
			}
		}
		return NULL;
	}
};

struct BhvValDeclareCache
{
	BhvValDeclareCache()
	{
		Zero();
	}
	~BhvValDeclareCache()
	{
		Clear();
	}
	void Zero()
	{
		pad=NULL;
		elem=NULL;
		declare=NULL;
		item=NULL;
	}
	void Clear()
	{
		Safe_Class_Delete(pad);
		Zero();
	}
	BhvValDeclare* declare;

	CBehaviorGraphPad* pad;
	GElemBase* elem;

	CXTPPropertyGridItem * item;
};

struct BhvValDeclareCaches
{
	BhvValDeclareCaches()
	{
	}

	void Clear()
	{
		entries.clear();
	}

	BhvValDeclareCache*CacheFromItem(CXTPPropertyGridItem *item)
	{
		for (int i=0;i<entries.size();i++)
		{
			if (entries[i].item==item)
				return &entries[i];
		}
		return NULL;
	}

	std::deque<BhvValDeclareCache> entries;

};

struct BhvVarDeclare;
class CBehaviorGraphGrid:public CGObjGrid,public CResEditCtrl
{
public:
	CBehaviorGraphGrid()
	{
		_owner=NULL;
		_padBind=NULL;
		_itemBehaviorValue=NULL;
	}
	void SetOwner(CBehaviorGraphEditPanel *owner)	{		_owner=owner;	}

	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
	Reps_BehaviorGraph*GetState()	{		return (Reps_BehaviorGraph*)CResEditCtrl::_state;	}
	BehaviorGraphData *GetResData()	{		return (BehaviorGraphData*)_GetResData();	}

	virtual void OnBeginItemChange(CXTPPropertyGridItem *item);
	virtual void OnEndItemChange(CXTPPropertyGridItem *item);
	
	virtual void EnableCtrl(BOOL bActive=TRUE);

	virtual CXTPPropertyGridItem *InsertVar(void *var,const char *cap,const char *desc,GVarType vt,GSem &sem);

protected:

	virtual BOOL _NeedRemoteMatSetEdit()	{		return TRUE;	}
	virtual BOOL _InsertElem(GObjBase *obj,GElemBase *elem); 
	virtual void _OnContextMenu(CXTPPropertyGridItem *item,CMenu &menu);
	virtual BOOL _OnDblClickOnItem(CXTPPropertyGridItem *item);

	BOOL _IsDeclarePad();

	LoAgentRef *_FindAgentRef(GObjBase *obj);

	void _CollectRefConstraints(const char *prefix,BhvValDeclare *declare,BhvValType &tp,std::vector<std::string> &constraints);
	void _CollectRefConstraints(const char *prefix,BhvValDeclare *declare,BehaviorMemType tpMem,std::vector<std::string> &constraints);
	void _CollectRefConstraints(const char *prefix,BhvVarDeclare *declare,BehaviorMemType tpMem,std::vector<std::string> &constraints);
	void _CollectRefConstraints(BhvValType &tp,std::vector<std::string> &constraints);
	void _CollectRefConstraints(BehaviorMemType tpMem,std::vector<std::string> &constraints);
	void _CollectStateConstraints(BehaviorGraphData *data,std::vector<std::string> &constraints);
	void _CollectStateConstraints(std::vector<std::string> &constraints);
	void _CollectAgentStateConstraints(std::vector<std::string> &constraints);
	void _CollectTroopConstraints(std::vector<std::string> &constraints);
	void _CollectFuncConstraints(std::vector<std::string> &constraints);

	void _BhvValueCache_PreLoad(BhvValuesCache *cache,BhvValues *values,GObjBase *obj);
	void _BhvValueCache_InsertItems(BhvValuesCache *cache);
	void _BhvValueCache_PreSave(BhvValuesCache *cache);
	BhvValuesCache::Bind*_BhvValueCache_BindFromItem(CXTPPropertyGridItem *item);

	BhvValDeclareCache *_BhvValDeclareCache_Preload(BhvValDeclare *declare);
	void _BhvValDeclareCache_InsertItem(BhvValDeclareCache *cache);
	void _BhvValDeclareCache_PreSave(CXTPPropertyGridItem *item);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnElemRef();
	afx_msg void OnBhvValRef();
	afx_msg void OnElemAddConst();
	afx_msg void OnElemAddParam();

	CLinkPad *_padBind;//只在Bind(..)函数执行内部有效

	CBehaviorGraphEditPanel *_owner;

	std::deque<BhvValuesCache> _cachesBhvValues;
	CXTPPropertyGridItem *_itemBehaviorValue;

	BhvValDeclareCaches _cachesBhvValDeclare;

};

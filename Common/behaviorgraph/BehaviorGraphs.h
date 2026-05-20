#pragma once

#include "../class/class.h"
#include "../gds/GObj.h"

#include "../strlib/strlibdefines.h"

#include "../linkpad/LinkPadDefines.h"

#include "BehaviorMem.h"
#include "BehaviorValue.h"

#include <set>


class BgnClassRegister
{
public:
	BgnClassRegister(CClass *clssPad,CClass *clssNode);
};


#define BIND_BGN_CLASS(clss,clssPad)															\
	BgnClassRegister __bgnregister##clss(Class_Ptr2(clssPad),Class_Ptr2(clss));


struct BgpClasses:public LinkPadClasses
{
	struct BgpClass
	{
		CClass *clssPad;
		CClass *clssNode;
	};

	virtual CLinkPad *New(const char *nmClass)
	{
		std::unordered_map<std::string,BgpClass>::iterator it=clsses.find(std::string(nmClass));
		if (it==clsses.end())
			return NULL;
		CClass *clss=((*it).second).clssPad;
		return (CLinkPad*)clss->New();
	}
	virtual CLinkPad *New(WORD uid)
	{
		std::unordered_map<WORD,BgpClass>::iterator it=clsses2.find(uid);
		if (it==clsses2.end())
			return NULL;
		CClass *clss=((*it).second).clssPad;
		return (CLinkPad*)clss->New();
	}

	virtual void CollectNames(std::vector<std::string>&buf)
	{
		buf.clear();
		std::unordered_map<std::string,BgpClass>::iterator it;
		for (it=clsses.begin();it!=clsses.end();it++)
			buf.push_back((*it).first);
	}

	virtual void CollectPadClasses(std::vector<CClass *>&buf)
	{
		buf.clear();
		std::unordered_map<std::string,BgpClass>::iterator it;
		for (it=clsses.begin();it!=clsses.end();it++)
			buf.push_back((*it).second.clssPad);
	}

	virtual WORD UIDFromClassName(const char *nmClass);

	CClass *FindNodeClass(const char *nmClass)
	{
		std::unordered_map<std::string,BgpClass>::iterator it=clsses.find(std::string(nmClass));
		if (it==clsses.end())
			return NULL;
		return ((*it).second).clssNode;
	}


	void Add(const char *nm,CClass *clssPad,CClass *clssNode)
	{
		BgpClass t;
		t.clssPad=clssPad;
		t.clssNode=clssNode;
		clsses[std::string(nm)]=t;
	}
	void Add(WORD uid,CClass *clssPad,CClass *clssNode)
	{
		BgpClass t;
		t.clssPad=clssPad;
		t.clssNode=clssNode;
		clsses2[uid]=t;
	}

	std::unordered_map<std::string,BgpClass> clsses;
	std::unordered_map<WORD,BgpClass> clsses2;

};

struct BGPad;
struct StbOther
{
	StbOther()
	{
		Zero();
	}
	StbOther(BGPad* pad_,DWORD iStb_)
	{
		pad=pad_;
		iStb=iStb_;
	}
	void Zero()
	{
		pad=NULL;
		iStb=0;
	}
	BGPad* pad;
	DWORD iStb;
};



class CBehaviorGraphPad;
class CLinkPad;
class CBgp_Counter;
class CBgp_Timer;
class CBgp_Register;
struct BGPad
{
	DEFINE_CLASS(BGPad);

	BGPad()
	{
		Zero();
	}
	void Zero()
	{
		pad=NULL;
		clssNode=NULL;
		idxMem=-1;
		lpadOwnerState=NULL;
		idxInStateHeap=-1;
	}

	struct RefInfo
	{
		RefInfo()
		{
			Zero();
		}
		void Zero()
		{
			elem=NULL;
			idRef=0;
			tp=None;
			value.Zero();
		}
		enum Type
		{
			None=0,
			Const,
			Mem,
			SimpleMem,
			Param,
		};
		GElemBase *elem;
		BhvVal value;
		StringID idRef;
		Type tp;
	};

	void Clear();

	void AddOther(DWORD iStub,BGPad *padOther,DWORD iStubOther);

	CBehaviorGraphPad *pad;
	CClass *clssNode;
	std::vector<RefInfo> refs;

	std::vector<StbOther> stbOthers;
	int idxMem;

	int idxInStateHeap;
	BGPad *lpadOwnerState;//owner state
};

class CBehaviorGraphPads;
class CBehaviorGraphs;
class CBehaviorGraph
{
public:
	DEFINE_CLASS(CBehaviorGraph);
	CBehaviorGraph()
	{
		Zero();
	}
	void Zero()
	{
		_owner=NULL;
		_pads=NULL;
		_nm=StringID_Invalid;
		_def=NULL;
	}

	void Clear();

	CBehaviorGraphs *GetOwner()	{		return _owner;	}

	StringID GetName()	{		return _nm;	}

	CBehaviorGraphPads *GetPads()	{		return _pads;	}

	BGPad *LPadFromPad(CLinkPad*pad);
	BGPad *LPadFromStateName(StringID nmState);
	PadID PadIDFromStateName(StringID nmState);
	PadID PadIDFromRelayName(StringID nmState);
	BGPad *LPadFromPadID(PadID id);
	BGPad *LPadFromRelayName(StringID nmRelay);

	BGPad *GetDefLPad()	{		return _def;	}

	CBgp_Timer*FindTimer(StringID nm);

	CBehaviorMemDesc *GetMemDesc(DWORD idx)	{		return idx<_mems.size()?_mems[idx]:NULL;	}

	//注意breaks/starts中state的顺序是sub state排在前面
	//返回的breaks/starts是临时指针,不能保存
	BOOL FindStateSwitch(PadID idSrcState,PadID idDestState,std::vector<PadID>*&breaks,std::vector<PadID>*&starts);
	BOOL FindStateActivate(PadID idDestState,std::vector<PadID>*&starts);

	PadID GetOwnerState(PadID id);

	void ResolveInclude(PadID id,CBehaviorGraph *bg);

protected:

	BOOL _CheckAncestorState(BGPad *lpad,BGPad *lpadAncestor);


	CBehaviorGraphs *_owner;
	CBehaviorGraphPads *_pads;
	std::vector<BGPad*> _lpads;
	std::unordered_map<PadID,BGPad *> _lookupLPad;//根据PadID找lpad
	BGPad *_def;

	std::unordered_map<StringID,BGPad *> _states;//根据名字找states
	std::unordered_map<StringID,BGPad *> _relays;//根据名字找relays
	std::vector<BGPad*> _heapStates;//所有的State按照Level排列,确保一个State永远排在它的后代State的前面

	StringID _nm;
	std::unordered_map<StringID,CBgp_Counter*> _counters;
	std::unordered_map<StringID,CBgp_Timer*> _timers;
	std::unordered_map<StringID,CBgp_Register*> _regs;
	std::vector<CBehaviorMemDesc*> _mems;

	//FindStateSwitch(..)的临时Buff
	std::vector<PadID>_breaksSwitchState;
	std::vector<PadID>_startsSwitchState;

	friend class CBehaviorGraphs;
	friend class CBehavior;
	friend class CBehaviorGraph;

};

class CBehaviorGraphPads;
struct LevelBehaviorContext;
class CLinkPad;
class CBehaviorDebug;
class CBehaviorGraphs
{
public:

	CBehaviorGraphs()
	{
		Zero();
	}

	~CBehaviorGraphs()
	{
		Clear();
	}

	void Zero()
	{
		_debug=NULL;
	}
	virtual void Clear();

	CBehaviorGraph *FindBG(StringID nm);
	CBehaviorDebug *GetDebug()	{		return _debug;	}
	void SetDebug(CBehaviorDebug * dbg)	{		_debug=dbg;	}

protected:
	BOOL _CompileBG(CBehaviorGraphPads *pads);
	BOOL _LoadBGPadsFromData(BYTE *data,CBehaviorGraphPads &pads,LinkPadClasses *clsses);

	std::vector<CBehaviorGraph *>_bgs;
	std::unordered_map<StringID,CBehaviorGraph *>_lookup;

	std::unordered_map<std::string,CClass *>_classesNode;//根据BGP的Class名称查找BGN的Class

	CBehaviorDebug *_debug;

	friend class CBehaviorGraph;


};
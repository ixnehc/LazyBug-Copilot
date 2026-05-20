#pragma once

#include "../class/class.h"
#include "../gds/GObj.h"
#include "../strlib/strlibdefines.h"
#include "../linkpad/LinkPadDefines.h"
#include "../anim/animdefines.h"

#include "BehaviorDefines.h"

#include "BehaviorDebug.h"

class CBgp_Func;
struct BhvVal;
struct BehaviorCall
{
	BehaviorCall()
	{
		_padFunc=NULL;
	}
	CBgp_Func *_padFunc;
	std::unordered_map<StringID,BhvVal*> _paramsRT;
};


#define BGNTHREAD_INVALID_REWINDKEY (0xff)
struct BgnThread
{
	BgnThread()
	{
		padState=PadID_Null;
		keyRewind=BGNTHREAD_INVALID_REWINDKEY;
		bFinalizing=0;
		idNode=BgnID_Invalid;
		call=NULL;
	}

	BOOL IsValid()
	{
		return idNode!=BgnID_Invalid;
	}

	BOOL Equals(BgnThread &other)
	{
		if ((padState==other.padState)&&
			(keyRewind==other.keyRewind)&&
			(bFinalizing==other.bFinalizing)&&
			(idNode==other.idNode))
			return TRUE;
		return FALSE;
	}
	PadID padState;
	BYTE keyRewind;
	BYTE bFinalizing;
	BgnID idNode;
	BehaviorCall *call;
};


class CBehaviorGraph;
class CBehavior;

struct BGPad;
struct BGNOutputs;
class CBehaviorMem;
class CBehaviorMemDesc;
class CBehaviorGraphPad;
struct BhvValType;
class CBehaviorGraphNode
{
public:
	CBehaviorGraphNode()
	{
		_result=A_Pending;
		_bInPending=FALSE;
		_lpad=NULL;
		_bhv=NULL;
		_id=BgnID_Invalid;
		_padCache=NULL;
	}
	~CBehaviorGraphNode();
	BOOL IsValid()	{		return _bhv!=NULL;	}

	virtual CClass*GetClass()=0;

	virtual void Create(){}
	virtual void Destroy(){}

	virtual void Start(DWORD iStb,BGNOutputs &outputs){}
	virtual void StartPending(DWORD iStb){}
	virtual void Break(BGNOutputs &outputs){}
	virtual void Update(BGNOutputs &outputs){}
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs){}//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs){}//因为执行失败导致的Rewind


	virtual BOOL GetCIn()	{		return FALSE;	}

	CBehaviorGraphPad *GetPad()	{		return _GetPad();	}

protected:
	CBehaviorGraphPad *_GetPad();
	CBehaviorMem*_GetMem();
	CBehaviorMemDesc*_GetMemDesc();
	CBehaviorGraph *_GetBg();
	AnimTick _GetT();
	
	template <typename T>
	T *_GetPad()	{		return (T*)_GetPad();	}

	BOOL _GetCOut(DWORD iStb);
	BOOL _TestStbLink(DWORD iStb);//返回iStb上有没有link其它stb

	void _OutputOk(BGNOutputs &outputs,DWORD iStb,const char *nm);
	void _OutputFail(BGNOutputs &outputs,DWORD iStb,const char *nm);

	void _VerifyStbName(DWORD iStb,const char *nm);

	BOOL _IsFinalizing()	{		return _thrd.bFinalizing;	}

	//Var Access
	virtual BOOL _SetBit(StringID nmVar,BOOL b);
	virtual BOOL _SetNumber(StringID nmVar,short n);
	virtual BOOL _SetID(StringID nmVar,BehaviorMemType tpID,DWORD id);
	virtual BOOL _SetPos(StringID nmVar,i_math::vector2df &pos);
	virtual BOOL _SetFloat(StringID nmVar,float f);
	virtual BOOL _GetBit(StringID nmVar,BOOL &b);
	virtual BOOL _GetNumber(StringID nmVar,short &n);
	virtual BOOL _GetID(StringID nmVar,BehaviorMemType tpID,DWORD &id);
	virtual BOOL _GetPos(StringID nmVar,i_math::vector2df &pos);
	virtual BOOL _GetFloat(StringID nmVar,float&f);

	virtual void _ResolvePad(CBehaviorGraphPad *pad)	{	}

	void _SetResult(AResult result);
	BYTE _result;
	BYTE _bInPending:1;//这个Node是否在CBehavior::_pending中

	BgnID _id;

	BgnThread _thrd;

	CBehavior *_bhv;

	BGPad *_lpad;
	CBehaviorGraphPad *_padCache;

	friend class CBehavior;
};


#define MAX_BGN_OUTPUT 16
struct BGNOutputs
{
	BGNOutputs()
	{
		Zero();
	}
	~BGNOutputs()
	{
		Clear();
	}
	void Zero()
	{
		nOutputs=0;
		thrdsBreak.clear();
		idsNewState.clear();
		idRelay=StringID_Invalid;
	}
	void Clear()
	{
		Zero();
	}

	void Add(int iStb,BgnThread thrd)
	{
		if (nOutputs>=ARRAY_SIZE(stbs))
			return;
		stbs[nOutputs]=(BYTE)iStb;
		thrds[nOutputs]=thrd;
		nOutputs++;
	}

	std::vector<BgnThread> thrdsBreak;

	std::vector<PadID> idsNewState;//要新开始那些State

	PadID idRelay;
	BgnThread thrdRelay;

	DWORD nOutputs;
	BYTE stbs[MAX_BGN_OUTPUT];
	BgnThread thrds[MAX_BGN_OUTPUT];
};

struct BGNop
{
	enum Op
	{
		None,
		Start,
		Break,
		Update,
		RewindOk,
		RewindFail,
		StartPending,
	};

	BGNop()
	{
		node=NULL;
		op=None;
		keyRewind=BGNTHREAD_INVALID_REWINDKEY;
	}

	BGNop(CBehaviorGraphNode *node_,Op op_)
	{
		node=node_;
		op=op_;
		keyRewind=BGNTHREAD_INVALID_REWINDKEY;
	}
	CBehaviorGraphNode *node;
	BYTE op;
	WORD keyRewind;//只在op是RewindOk/RewindFail时有效
};

struct BehaviorCounter
{
	int v;
};

struct BehaviorTimer
{
	AnimTick tExpect;
};

class CBehavior
{
public:
	CBehavior()
	{
		Zero();
	}
	~CBehavior()
	{
		Clear();
	}
	void Zero()
	{
		_bg=NULL;
		_seedBgnID=BgnID_Invalid;
		_iFrame=0;
		_tStart=0;
		_objOwner=0;
	}
	void Init(CBehaviorGraph *bg,DWORD objOwner);
	virtual void Clear();

	StringID GetName();
	CBehaviorGraph *GetBg()	{		return _bg;	}

	virtual void Start();
	AResult StartRelay(StringID nmRelay);

	virtual void Update();

	BOOL IsPadLocked(PadID idPad);
	BOOL LockPad(PadID idPad);//一个Pad被Lock后,第二次Lock将会失败
	void UnLockPad(PadID idPad);

	BehaviorCounter *FindCounter(StringID nm)
	{
		std::unordered_map<StringID,BehaviorCounter>::iterator it=_counters.find(nm);
		if (it==_counters.end())
			return NULL;
		return &(*it).second;
	}
	BehaviorTimer *FindTimer(StringID nm)
	{
		std::unordered_map<StringID,BehaviorTimer>::iterator it=_timers.find(nm);
		if (it==_timers.end())
			return NULL;
		return &(*it).second;
	}

	CBehaviorGraphNode *NodeFromNodeID(BgnID idNode);


	CBehaviorMem *GetMem(DWORD idxMem)	{		return idxMem<_mems.size()?_mems[idxMem]:NULL;	}
	AnimTick GetT()	{		return _GetT();	}
	DWORD GetFrame()	{		return _iFrame;	}
	AnimTick GetStartTime()	{		return _tStart;	}

	void SetNodeData(PadID id,unsigned __int64 data);
	BOOL GetNodeData(PadID id,unsigned __int64 &data);

protected:

	virtual AnimTick _GetT()=0;

	AResult _FlushOps(std::deque<BGNop> &nops);

	BOOL _MakeOutputs(CBehaviorGraphNode *node,BGNOutputs &outputs,std::deque<BGNop> &nops);
	void _MakeRewind(CBehaviorGraphNode *node,BOOL bOk,std::deque<BGNop> &nops);


	CBehaviorGraphNode *_CreateNode(BGPad *lpad,BgnThread thrd);
	void _DestroyNode(CBehaviorGraphNode *node);
	void _BreakNode(CBehaviorGraphNode *node,BGNOutputs &outputs);
	void _DeleteNotInPending(CBehaviorGraphNode *node);

	void _DebugStep(CBehaviorGraphNode *node,BOOL bBreaking,AResult result=A_Pending);

	AnimTick _tStart;
	DWORD _iFrame;

	std::unordered_map<PadID,unsigned __int64>_dataNodes;

	std::vector<CBehaviorGraphNode *>_pendings;
	std::unordered_map<StringID,BehaviorCounter> _counters;
	std::unordered_map<StringID,BehaviorTimer> _timers;

	//lock
	std::unordered_set<PadID> _locks;

	CBehaviorGraph *_bg;

	//mem
	std::vector<CBehaviorMem *> _mems;//第0个mem是这个behavior自己的mem,后面的是include进来的behaivor的mem

	std::deque<BGNop>_nops;
	std::unordered_set<PadID> _statesSwitched;

	//outputs
	BGNOutputs _outputs;

	BgnID _seedBgnID;

	DWORD _objOwner;

	static BehaviorDebugFrameData _dataFrameTemp;

};
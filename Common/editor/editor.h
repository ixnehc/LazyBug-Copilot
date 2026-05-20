#pragma once


#include <vector>
#include <string>
#include <map>

#include "ctrlop.h"

#include "../mod/ModBase.h"


class CGeView;
class CGeActor;
class CGeAgent;
class CGeMgr;
struct GeData;

typedef int AgentPriority;
#define AGENTPRIORITY_STANDARD 100

#define MAX_AGENT_IN_VIEW 20


class CGeAgent
{
public:
	IMPLEMENT_REFCOUNT;

	CGeAgent()
	{
		_view=NULL;
		_iLevelInView=-1;
		_bEnable=TRUE;
	}
	virtual ~CGeAgent()
	{
	}

	virtual void Enable(BOOL bEnable);
	BOOL IsEnable()	{		return _bEnable;	}

	CGeView *GetView()	{		return _view;	}
	CGeActor *GetActor()	{		return _GetActor();	}
	GeData *FindData(const char *name);
	void InvalidateView();

	virtual BOOL Respond(CtrlOp &co);//return whether the other agents need further processing

	virtual void OccupyFocus(OpType type);
	virtual void DiscardFocus(OpType type);

	//Overridables,the return value indicates whether the other agents need further processing
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnLButtonDblClk(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnLButtonClick(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnRButtonDown(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnRButtonUp(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnRButtonDblClk(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnMButtonDown(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnMButtonUp(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnMButtonDblClk(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnMButtonClick(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnMouseMove(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnMouseWheel(int delta,DWORD flag){return TRUE;}
	virtual BOOL OnTimer(int dt,DWORD flag){return TRUE;}
	virtual BOOL OnKeyDown(char c,DWORD flag){return TRUE;}//c is in upper case for 'a'~'z'
	virtual BOOL OnKeyUp(char c,DWORD flag){return TRUE;}//c is in upper case for 'a'~'z'
	virtual BOOL OnCommand(DWORD idCmd){return TRUE;}
	virtual BOOL OnSetCursor(int x,int y,DWORD flag){return TRUE;}
	virtual BOOL OnDraw(){return TRUE;}

	virtual void OnEnable()	{	}
	virtual void OnDisable()	{	}
	virtual void OnSetFocus(OpType type)	{	}
	virtual void OnKillFocus(OpType type)	{	}
	virtual void OnAttachView(CGeView *view,DWORD iLevel)	{	}
	virtual void OnDetachView(CGeView *view,DWORD iLevel)	{	}


public://take it as protected
	CGeAgent *_GetFocus(OpType ot);

	//operations
	void _Redraw(BOOL bImmediately=TRUE);
	void _DetachActor();//detach the actor of the agent's level
	CGeActor *_GetActor();	
	CModManager *_GetModMgr();



	BOOL _bEnable;

	CGeView *_view;
	int _iLevelInView;//the level this agent belongs to
	AgentPriority _priority;

	friend class CGeView;
};


class CGeView
{
public:
	CGeView()
	{
		_mgr=NULL;
		_bInvalidate=TRUE;
	}

	void Reset();

	virtual const char *GetName()=0;
	virtual BOOL Respond(CtrlOp &co);
	virtual BOOL Draw();

	virtual void Invalidate()	{		_bInvalidate=TRUE;	}
	BOOL CheckName(const char *name);//check whether this view uses the given name

	GeData *FindData(const char *name);
	CGeMgr *GetMgr()	{		return _mgr;	}
	CGeActor *GetCurActor();//得到当前最高level的那个actor
	CGeActor *GetActor(DWORD iLevel);//得到指定level的那个actor
	BOOL IsInvalidate()	{		return _bInvalidate;	}
	void ClearInvalidate()	{		_bInvalidate=FALSE;	}


	BOOL DiscardLevels(DWORD nLevelToKeep);

	BOOL AttachActor(DWORD iLevel,CGeActor *actor);
	BOOL DetachActor(DWORD iLevel,CGeActor *actor);

	void ClearFocus(DWORD iLevel);
	void ClearAgent(DWORD iLevel);

	CGeAgent *GetFocus(OpType ot);

	//This function will add refcount for the passed-in agent internally
	BOOL AddAgent(DWORD iLevel,CGeAgent *agent,AgentPriority priority=AGENTPRIORITY_STANDARD);
// 	BOOL RemoveAgent(CGeAgent *agent);


protected:
	struct _State
	{
		_State()
		{
			memset(focus,0,sizeof(focus));
			memset(agents,0,sizeof(agents));
			owner=NULL;
			nAgents=0;
		}
		void Clear()
		{
			ClearFocus();
			ClearAgent();
			owner=NULL;
		}
		void ClearAgent();
		void ClearFocus();

		struct AgentInfo
		{
			CGeAgent *agent;
			AgentPriority prior;
		};
		AgentInfo agents[MAX_AGENT_IN_VIEW];//the agents are sorted by their priorities
		DWORD nAgents;
		CGeAgent *focus[OpType_Max];
		CGeActor *owner;
	};

	BOOL _PushState();
	BOOL _PopState();
	void _CleanTopState();
	void _ReserveState(DWORD nState);

	_State *_CurState()	
	{		
		if (_states.size()>0)
			return &_states[_states.size()-1];
		return NULL;
	}

	std::vector<_State> _states;

	CGeMgr *_mgr;

	BOOL _bInvalidate;

	friend class CGeAgent;
	friend class CGeMgr;
	friend class CGeActor;
};

class CGeActor
{
public:
	CGeActor()
	{
		_mgr=NULL;
		_modmgr=NULL;
	}
	~CGeActor(){}
	virtual const char *GetName()=0;
	virtual void UpdateUI()	{}
	virtual void OnDetachView(CGeView *view,DWORD iLevel)	{	}
	virtual void OnAttachView(CGeView *view,DWORD iLevel)	{	}
	virtual void DoCommand(DWORD idCmd)	{	}
	virtual void UpdateCommandUI(DWORD idCmd,void *param)	{	}
	virtual void OnLeaveActivity()	{	}
	virtual void OnEnterActivity()	{	}

	GeData *FindData(const char *name);
	CGeView *FindView(const char *name);
	BOOL IsActive();


	CGeMgr *GetMgr()	{		return _mgr;	}
	CModManager *GetModMgr()	{		return _modmgr;	}

protected:

	virtual const char *_GetModMgrName()	{		return "";	}

	CGeMgr *_mgr;
	CModManager *_modmgr;

	friend class CGeMgr;
};


class CDataPacket;
struct GeData
{
public:
	GeData()
	{
		_mgr=NULL;
	}
	virtual const char *GetName()=0;

protected:
	CGeMgr *_mgr;
	friend class CGeMgr;
};

class CGeMgr
{
public:
	CGeMgr()
	{
		_actorActive=NULL;
	}
	void Reset();

	BOOL RegisterActor(CGeActor *actor);
	BOOL RegisterView(CGeView *view);
	BOOL RegisterData(GeData *data);
	CGeView *FindView(const char *name);
	GeData *FindData(const char *name);

	BOOL AddModMgr(const char *name);
	CModManager *FindModMgr(const char *name);

	void InvalidateView(const char *name);

	void RedrawView();

	virtual void Update();

	virtual CGeActor *GetActiveActor()	{		return NULL;	}

	void DoCommand(DWORD idCmd);
	void UpdateCommandUI(DWORD idCmd,void *param);


protected:	

	std::vector<CGeView *> _views;
	std::vector<CGeActor *> _actors;
	std::vector<GeData *> _datas;

	CGeActor *_actorActive;


	std::map<std::string,CModManager*>_modmgrs;


	friend class CGeActor;

};
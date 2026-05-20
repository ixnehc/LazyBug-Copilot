/********************************************************************
	created:	2007/2/13   13:13
	filename: 	e:\IxEngine\Common\editor\editor.cpp
	author:		cxi
	
	purpose:	class for easy managing edit
*********************************************************************/
#include "stdh.h"

#include "editor.h" 

/////////////////////////////////////////////////////////////////////////
//CGeAgent

GeData *CGeAgent::FindData(const char *name)
{
	if (!_view)
		return NULL;
	return _view->FindData(name);
}

void CGeAgent::InvalidateView()
{
	if (_view)
		_view->Invalidate();
}



void CGeAgent::Enable(BOOL bEnable)
{
	if (_bEnable==bEnable)
		return;
	_bEnable=bEnable;	

	if (bEnable)
		OnEnable();
	else
	{
		OnDisable();
		for (int i=0;i<OpType_Max;i++)
			DiscardFocus((OpType)i);
	}
}

BOOL CGeAgent::Respond(CtrlOp &co)
{
	OpType ot=co.GetType();
	if (ot==OpType_None)
		return TRUE;

	CGeAgent *agFocus=_GetFocus(ot);
	if (agFocus&&(agFocus!=this))
		return TRUE;

	BOOL bFurther=TRUE;
	switch(ot)
	{
		case OpType_Timer:
			bFurther=OnTimer(co.dt,co.flag);
			break;
		case OpType_Command:
			bFurther=OnCommand(co.idCmd);
			break;
		case OpType_SetCursor:
			bFurther=OnSetCursor(co.x,co.y,co.flag);
			break;
		case OpType_Mouse:
		{
			if (co.op==CtrlOp::Op_Wheel)
			{
				bFurther=OnMouseWheel(co.delta,co.flag);
				break;
			}
			if (co.op==CtrlOp::Op_Move)
			{
				bFurther=OnMouseMove(co.x,co.y,co.flag);
				break;
			}

			if (co.vk==VK_LBUTTON)
			{
				if (co.op==CtrlOp::Op_Down)
					bFurther=OnLButtonDown(co.x,co.y,co.flag);
				if (co.op==CtrlOp::Op_Up)
					bFurther=OnLButtonUp(co.x,co.y,co.flag);
				if (co.op==CtrlOp::Op_DblClick)
					bFurther=OnLButtonDblClk(co.x,co.y,co.flag);
				if (co.op==CtrlOp::Op_Click)
					bFurther=OnLButtonClick(co.x,co.y,co.flag);
			}
			if (co.vk==VK_RBUTTON)
			{
				if (co.op==CtrlOp::Op_Down)
					bFurther=OnRButtonDown(co.x,co.y,co.flag);
				if (co.op==CtrlOp::Op_Up)
					bFurther=OnRButtonUp(co.x,co.y,co.flag);
				if (co.op==CtrlOp::Op_DblClick)
					bFurther=OnRButtonDblClk(co.x,co.y,co.flag);
				if (co.op==CtrlOp::Op_Click)
					bFurther=OnRButtonClick(co.x,co.y,co.flag);
			}
			if (co.vk==VK_MBUTTON)
			{
				if (co.op==CtrlOp::Op_Down)
					bFurther=OnMButtonDown(co.x,co.y,co.flag);
				if (co.op==CtrlOp::Op_Up)
					bFurther=OnMButtonUp(co.x,co.y,co.flag);
				if (co.op==CtrlOp::Op_DblClick)
					bFurther=OnMButtonDblClk(co.x,co.y,co.flag);
				if (co.op==CtrlOp::Op_Click)
					bFurther=OnMButtonClick(co.x,co.y,co.flag);
			}
			break;
		}
		case OpType_Keyboard:
		{
			if (co.op==CtrlOp::Op_Down)
				bFurther=OnKeyDown(co.vk,co.flag);
			if (co.op==CtrlOp::Op_Up)
				bFurther=OnKeyUp(co.vk,co.flag);
			break;
		}
	}

	return bFurther;
}

CGeAgent *CGeAgent::_GetFocus(OpType ot)
{		
	if (!GetView())
		return NULL;
	return GetView()->GetFocus(ot);
}

void CGeAgent::OccupyFocus(OpType type)
{
	if (!GetView())
		return;

	if (type!=OpType_None)
	{
		if (GetView()->_CurState())
		{
			if (GetView()->_CurState()->focus[type])
				GetView()->_CurState()->focus[type]->OnKillFocus(type);
			GetView()->_CurState()->focus[type]=this;
			OnSetFocus(type);
		}
	}
}

void CGeAgent::DiscardFocus(OpType type)
{
	if (!GetView())
		return;
	if (type!=OpType_None)
	{
		if (GetView()->_CurState())
		{
			if (GetView()->_CurState()->focus[type]==this)
			{
				GetView()->_CurState()->focus[type]=NULL;
				OnKillFocus(type);
			}
		}
	}
}

void CGeAgent::_Redraw(BOOL bImmediately)
{
	if (!GetView())
		return;
	if (bImmediately)
		GetView()->Draw();
	else
		GetView()->Invalidate();
}

void CGeAgent::_DetachActor()
{
	if (!GetView())
		return;
	if (GetView())
		GetView()->DetachActor(_iLevelInView,NULL);
}

CModManager *CGeAgent::_GetModMgr()
{
	if (GetView())
		return GetView()->GetCurActor()->GetModMgr();
	return NULL;
}

CGeActor *CGeAgent::_GetActor()
{		
	if (!GetView())
		return NULL;
	return _view->GetActor(_iLevelInView);	
}




//////////////////////////////////////////////////////////////////////////
//CGeView::_State

void CGeView::_State::ClearAgent()
{
	for (int i=0;i<nAgents;i++)
	{
		if (agents[i].agent)
			agents[i].agent->OnDetachView(agents[i].agent->_view,agents[i].agent->_iLevelInView);
	}

	//break from me
	for (int i=0;i<nAgents;i++)
	{
		if (agents[i].agent)
		{
			agents[i].agent->_view=NULL;
			agents[i].agent->_iLevelInView=-1;
		}
		SAFE_RELEASE(agents[i].agent);
	}

	nAgents=0;
}

void CGeView::_State::ClearFocus()
{
	for (int i=0;i<OpType_Max;i++)
	{
		CGeAgent *p=focus[i];
		if (p)
			p->DiscardFocus((OpType)i);
	}
	memset(focus,0,sizeof(focus));
}



//////////////////////////////////////////////////////////////////////////
//CGeView
void CGeView::Reset()
{
	while(_PopState());
}

BOOL CGeView::CheckName(const char *name)
{
	if (strcmp(name,GetName())==0)
		return TRUE;
	return FALSE;
}


GeData *CGeView::FindData(const char *name)
{
	if (!_mgr)
		return NULL;
	return _mgr->FindData(name);
}

CGeActor *CGeView::GetCurActor()
{
	if (_states.size()>0)
		return _CurState()->owner;
	return NULL;
}

CGeActor *CGeView::GetActor(DWORD iLevel)
{
	if (iLevel>=_states.size())
		return NULL;
	return _states[iLevel].owner;
}



BOOL CGeView::Respond(CtrlOp &co)
{
	if (_states.size()<=0)
		return TRUE;
	for (int i=0;i<_CurState()->nAgents;i++)
	{
		if (_CurState()->agents[i].agent->IsEnable())
		if (FALSE==_CurState()->agents[i].agent->Respond(co))
			return FALSE;
	}
	return TRUE;
}

BOOL CGeView::Draw()
{
	if (_states.size()<=0)
		return TRUE;
	for (int i=_CurState()->nAgents-1;i>=0;i--)
	{
		if (_CurState()->agents[i].agent->IsEnable())
		if (FALSE==_CurState()->agents[i].agent->OnDraw())
			return FALSE;
	}
	return TRUE;
}


CGeAgent *CGeView::GetFocus(OpType ot)
{
	if (_states.size()<=0)
		return NULL;
	if ((DWORD)ot>=OpType_Max)
		return NULL;
	return _CurState()->focus[ot];
}


void CGeView::ClearFocus(DWORD iLevel)
{
	if (iLevel>=_states.size())
		return;
	_states[iLevel].ClearFocus();
}


void CGeView::ClearAgent(DWORD iLevel)
{
	if (iLevel>=_states.size())
		return;
	_states[iLevel].ClearAgent();
}


BOOL CGeView::_PushState()
{
	if (_states.size()>0)
		ClearFocus(_states.size()-1);
	_State state;

//	memcpy(&state,_CurState(),sizeof(_State));
//	for (int i=0;i<ARRAY_SIZE(state.agents);i++)
//		SAFE_ADDREF(state.agents[i].agent);

	_states.push_back(state);

	return TRUE;
}

BOOL CGeView::_PopState()
{
	if (_states.size()<=0)
		return FALSE;

	if (_CurState()->owner)
		_CurState()->owner->OnDetachView(this,_states.size()-1);

	ClearFocus(_states.size()-1);
	ClearAgent(_states.size()-1);
	_states.resize(_states.size()-1);

	return TRUE;
}

void CGeView::_CleanTopState()
{
	_State *p;
	while(p=_CurState())
	{
		if (!p->owner)
			_PopState();
		else
			break;
	}
}

void CGeView::_ReserveState(DWORD nState)
{
	while(_states.size()<nState)
		_PushState();
}


BOOL CGeView::DiscardLevels(DWORD nLevelToKeep)
{
	while(_states.size()>nLevelToKeep)
		_PopState();

	_CleanTopState();

	return TRUE;
}


BOOL CGeView::AttachActor(DWORD iLevel,CGeActor *actor)
{
	_ReserveState(iLevel+1);

	ClearFocus(iLevel);
	ClearAgent(iLevel);
	if (_states[iLevel].owner!=actor)
	{
		if (_states[iLevel].owner)
			_states[iLevel].owner->OnDetachView(this,iLevel);
		_states[iLevel].owner=actor;
		if (actor)
			actor->OnAttachView(this,iLevel);
	}

	_CleanTopState();

	return TRUE;
}

BOOL CGeView::DetachActor(DWORD iLevel,CGeActor *actor)
{
	if (_states.size()<=iLevel)
		return FALSE;

	if ((_states[iLevel].owner==actor)||(actor==NULL))
	{
		ClearFocus(iLevel);
		ClearAgent(iLevel);

		_states[iLevel].owner->OnDetachView(this,iLevel);
		_states[iLevel].owner=NULL;
		_CleanTopState();
		return TRUE;
	}
	return FALSE;
}


BOOL CGeView::AddAgent(DWORD iLevel,CGeAgent *agent0,AgentPriority priority)
{
	_State *state=NULL;
	if (iLevel<_states.size())
	{
		if (_states[iLevel].owner)
			state=&_states[iLevel];
	}
	if (!state)
		return FALSE;

	if (state->nAgents>=ARRAY_SIZE(state->agents))
		return FALSE;


	//Add while keeping priority sorted
	CGeAgent *agent=agent0;
	_State::AgentInfo buf[MAX_AGENT_IN_VIEW];
	DWORD n=0;
	for (int i=0;i<state->nAgents;i++)
	{
		if (agent)
		{
			if (state->agents[i].prior<=priority)
			{
				buf[n].agent=agent;
				buf[n].prior=priority;
				n++;
				agent=NULL;//mark as added
			}
		}

		buf[n]=state->agents[i];
		n++;
	}

	if (agent)
	{
		buf[n].agent=agent;
		buf[n].prior=priority;
		n++;
		agent=NULL;//mark as added
	}

	memcpy(state->agents,buf,sizeof(state->agents));
	state->nAgents=n;

	agent0->AddRef();
	agent0->_view=this;
	agent0->_iLevelInView=iLevel;
	agent0->_priority=priority;
	agent0->OnAttachView(this,iLevel);

	return TRUE;
}

// BOOL CGeView::RemoveAgent(CGeAgent *agent)
// {
// 	int iLevel=agent->_iLevelInView;
// 	if (iLevel>=_states.size())
// 		return FALSE;
// 	if (!_states[iLevel].owner)
// 		return FALSE;
// 
// 	_State *state=&_states[iLevel];
// 
// 	agent->OnDetachView(this,iLevel);
// 
// 	BOOL bRet=FALSE;
// 	//
// 	DWORD c=0;
// 	for (int i=0;i<state->nAgents;i++)
// 	{
// 		if (state->agents[i].agent==agent)
// 		{
// 			agent->Release();
// 			continue;
// 		}
// 		state->agents[c]=state->agents[i];
// 		c++;
// 	}
// 
// 	if (c!=state->nAgents)
// 	{
// 		bRet=TRUE;
// 		agent->Release();
// 	}
// 
// 
// 
// 
// 	//Çå³ýfocus
// 	for (int i=0;i<OpType_Max;i++)
// 	{
// 		if (state->focus[i]==agent)
// 			state->focus[i]=NULL;
// 	}
// 
// 
// 		
// 
// 
// 
// 	for (int)
// }



//////////////////////////////////////////////////////////////////////////
//CGeActor

GeData *CGeActor::FindData(const char *name)
{	
	if(_mgr)
		return _mgr->FindData(name);	
	return NULL;
}
CGeView *CGeActor::FindView(const char *name)	
{	
	if(_mgr)
		return _mgr->FindView(name);	
	return NULL;
}

BOOL CGeActor::IsActive()
{
	if (_mgr)
		return _mgr->_actorActive==this;
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//CGeMgr

void CGeMgr::Reset()
{
	if (_actorActive)
		_actorActive->OnLeaveActivity();
	for (int i=0;i<_views.size();i++)
	{
		_views[i]->Reset();
		_views[i]->_mgr=NULL;
	}
	for (int i=0;i<_actors.size();i++)
	{
		_actors[i]->_mgr=NULL;
		_actors[i]->_modmgr=NULL;
	}
	for (int i=0;i<_datas.size();i++)
		_datas[i]->_mgr=NULL;

	_views.clear();
	_actors.clear();
	_datas.clear();

	std::map<std::string,CModManager*>::iterator it;
	for (it=_modmgrs.begin();it!=_modmgrs.end();it++)
	{
		((*it).second)->Clear();
		delete ((*it).second);
	}
	_modmgrs.clear();

	_actorActive=NULL;

}

BOOL CGeMgr::RegisterActor(CGeActor *actor)
{
	_actors.push_back(actor);
	actor->_mgr=this;
	actor->_modmgr=FindModMgr(actor->_GetModMgrName());

	return TRUE;
}

BOOL CGeMgr::RegisterView(CGeView *view)
{
	if (FindView(view->GetName()))
		return FALSE;
	_views.push_back(view);
	view->_mgr=this;
	return TRUE;
}

BOOL CGeMgr::RegisterData(GeData *data)
{
	if (FindData(data->GetName()))
		return FALSE;
	_datas.push_back(data);
	data->_mgr=this;
	return TRUE;
}


CGeView *CGeMgr::FindView(const char *name)
{
	for (int i=0;i<_views.size();i++)
	{
		if (strcmp(_views[i]->GetName(),name)==0)
			return _views[i];
	}
	return NULL;
}

void CGeMgr::InvalidateView(const char *name)
{
	CGeView *view=FindView(name);
	if (view)
		view->Invalidate();
}


GeData *CGeMgr::FindData(const char *name)
{
	for (int i=0;i<_datas.size();i++)
	{
		if (strcmp(_datas[i]->GetName(),name)==0)
			return _datas[i];
	}
	return NULL;
}

void CGeMgr::RedrawView()
{
	for (int i=0;i<_views.size();i++)
	{
		if (_views[i]->IsInvalidate())
		{
			_views[i]->Draw();
			_views[i]->ClearInvalidate();
		}
	}
}

void CGeMgr::Update()
{
	CGeActor *a=GetActiveActor();
	if (a!=_actorActive)
	{
		if (_actorActive)
			_actorActive->OnLeaveActivity();
		_actorActive=a;
		if (_actorActive)
			_actorActive->OnEnterActivity();
	}
	for (int i=0;i<_actors.size();i++)
		_actors[i]->UpdateUI();
}

void CGeMgr::DoCommand(DWORD idCmd)
{
	if (_actorActive)
		_actorActive->DoCommand(idCmd);
}

void CGeMgr::UpdateCommandUI(DWORD idCmd,void *param)
{
	if (_actorActive)
		_actorActive->UpdateCommandUI(idCmd,param);
}

BOOL CGeMgr::AddModMgr(const char *name0)
{
	std::string name=name0;
	if (_modmgrs.find(name)!=_modmgrs.end())
		return FALSE;

	_modmgrs[name]=new CModManager;

	return TRUE;
}

CModManager *CGeMgr::FindModMgr(const char *name0)
{
	std::string name=name0;
	std::map<std::string,CModManager*>::iterator it=_modmgrs.find(name);
	if (it==_modmgrs.end())
		return NULL;
	return (*it).second;
}

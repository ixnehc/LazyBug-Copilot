/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 核心的BGN
*********************************************************************/
#include "stdh.h"
#include "BehaviorGraphs.h"
#include "Behavior.h"

#include "BgnState.h"

#include "../Random/Random.h"

#include "commondefines/general_stl.h"

////////////////////////////////////////////////////////////////////////
//CBgn_State

BIND_BGN_CLASS(CBgn_State,CBgp_State);

void CBgn_State::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_State *pad=_GetPad<CBgp_State>();
	if (pad)
	{
		if (pad->_flag!=0)
		{
			CBehaviorMem *mem=_GetMem();
			if (mem)
				mem->AddState(pad->GetID(),pad->_flag);
		}
		if (pad->_nm!=StringID_Invalid)
		{
			if (_bhv->LockPad(pad->GetID()))
			{
				BgnThread thrd;
				thrd.padState=pad->GetID();
				thrd.idNode=BgnID_Invalid;//不使用_id,State启动的thread通过padState就可以区分了
				_VerifyStbName(0,"触发");
				outputs.Add(0,thrd);
			}
		}
	}
	_SetResult(A_Ok);
}

void CBgn_State::StartPending(DWORD iStb)
{
	CBgp_State *pad=_GetPad<CBgp_State>();
	if (pad)
	{
		if (pad->_flag!=0)
		{
			CBehaviorMem *mem=_GetMem();
			if (mem)
				mem->AddState(pad->GetID(),pad->_flag);
		}
	}
}


void CBgn_State::Update(BGNOutputs &outputs)
{
	CBgp_State *pad=_GetPad<CBgp_State>();
	if (pad)
	{
		if (pad->_nm!=StringID_Invalid)
		{
			if (_bhv->LockPad(pad->GetID()))
			{
				BgnThread thrd;
				thrd.padState=pad->GetID();
				thrd.idNode=BgnID_Invalid;//不使用_id,State启动的thread通过padState就可以区分了
				_VerifyStbName(0,"触发");
				outputs.Add(0,thrd);
			}
		}
	}
	_SetResult(A_Ok);
}


////////////////////////////////////////////////////////////////////////
//CBgn_SwitchState
BIND_BGN_CLASS(CBgn_SwitchState,CBgp_SwitchState);
void RemoveStateMem(CBehavior *bhv,PadID idPad)
{
	CBehaviorMem *mem=bhv->GetMem(0);
	if (mem)
		mem->RemoveState(idPad);
}

void CBgn_SwitchState::Start(DWORD iStb,BGNOutputs &outputs)
{ 
	CBgp_SwitchState *pad=_GetPad<CBgp_SwitchState>();
	if (pad)
	{
		if (pad->_nm!=StringID_Invalid)
		{
			CBehaviorGraph *bg=_bhv->GetBg();

			PadID idOwnerState=_thrd.padState;
			PadID idStateToSwitch=bg->PadIDFromStateName(pad->_nm);

			if (!_bhv->IsPadLocked(idStateToSwitch))
			{
				if (idStateToSwitch!=PadID_Null)
				{
					std::vector<PadID> *breaks,*starts;
					if (bg->FindStateSwitch(idOwnerState,idStateToSwitch,breaks,starts))
					{
						if (breaks&&starts)
						{
							BgnThread thrd;
							for (int i=0;i<breaks->size();i++)
							{
								PadID idPad=(*breaks)[i];
								thrd.padState=idPad;
								thrd.idNode=BgnID_Invalid;
								outputs.thrdsBreak.push_back(thrd);

								_bhv->UnLockPad(idPad);

								RemoveStateMem(_bhv,idPad);
							}

							outputs.idsNewState=(*starts);
						}
					}
				}
			}
		}
	}
	_SetResult(A_Ok);
}



////////////////////////////////////////////////////////////////////////
//CBgn_ActivateStates
BIND_BGN_CLASS(CBgn_ActivateStates,CBgp_ActivateStates);
void CBgn_ActivateStates::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_ActivateStates*pad=_GetPad<CBgp_ActivateStates>();
	if (pad)
	{
		CBehaviorGraph *bg=_bhv->GetBg();
		for (int i=0;i<pad->_nms.size();i++)
		{
			StringID id=pad->_nms[i];
			if (id!=StringID_Invalid)
			{
				std::vector<PadID> *starts;
				if (bg->FindStateActivate(bg->PadIDFromStateName(id),starts))
				{
					VEC_APPEND(outputs.idsNewState,*starts);
				}
			}
		}
	}
	_SetResult(A_Ok);

	_VerifyStbName(1,"结束");
	outputs.Add(1,_thrd);
}

////////////////////////////////////////////////////////////////////////
//CBgn_TerminateState
BIND_BGN_CLASS(CBgn_TerminateState,CBgp_TerminateState);
void CBgn_TerminateState::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBehaviorGraph *bg=_bhv->GetBg();
	PadID idOwnerState=_thrd.padState;

	if (idOwnerState!=PadID_Null)
	{
		BgnThread thrd;
		thrd.padState=idOwnerState;
		thrd.idNode=BgnID_Invalid;
		outputs.thrdsBreak.push_back(thrd);
		_bhv->UnLockPad(idOwnerState);
		RemoveStateMem(_bhv,idOwnerState);
	}
	_SetResult(A_Ok);
}


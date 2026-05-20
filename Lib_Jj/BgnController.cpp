/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 核心的BGN
*********************************************************************/
#include "stdh.h"
#include "behaviorgraph/BehaviorGraphs.h"
#include "behaviorgraph/Behavior.h"

#include "BgnController.h"

#include "Random/Random.h"




////////////////////////////////////////////////////////////////////////
//CBgn_Repeater_Obsolete
BIND_BGN_CLASS(CBgn_Repeater_Obsolete,CBgp_Repeater_Obsolete);
int CBgn_Repeater_Obsolete::_FindFirstLink(int iStart)
{
	CBgp_Repeater_Obsolete*pad=_GetPad<CBgp_Repeater_Obsolete>();
	int iLast=pad->_nSteps;

	while(iStart<iLast)
	{
		if (_TestStbLink(Repeater_OutputStart+iStart))
			return iStart;
		iStart++;
	}

	return -1;

}

void CBgn_Repeater_Obsolete::_Fire(BGNOutputs &outputs)
{
	if (_iCurStep<0)
		return;
	CBgp_Repeater_Obsolete*pad=_GetPad<CBgp_Repeater_Obsolete>();
	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=(BYTE)_iCurStep;
	
	outputs.Add(_iCurStep+Repeater_OutputStart,thrd);
}

void CBgn_Repeater_Obsolete::Start(DWORD iStb,BGNOutputs &outputs)
{
	_iCurStep=0;
	_iCurLoop=0;
	_bAnyOk=FALSE;

	_iCurStep=_FindFirstLink(_iCurStep);
	if (_iCurStep==-1)
	{
		_SetResult(A_Fail);
		return;
	}

	_Fire(outputs);

}

void CBgn_Repeater_Obsolete::Update(BGNOutputs &outputs)
{
	if (_bNeedFire)
		_Fire(outputs);
	_bNeedFire=FALSE;
}

void CBgn_Repeater_Obsolete::Break(BGNOutputs &outputs)
{
	if (!_bNeedFire)
	{
		if (_iCurStep!=-1)
		{
			BgnThread thrd=_thrd;
			thrd.idNode=_id;
			thrd.keyRewind=(BYTE)_iCurStep;

			outputs.thrdsBreak.push_back(thrd);
		}
	}
}


void CBgn_Repeater_Obsolete::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	CBgp_Repeater_Obsolete*pad=_GetPad<CBgp_Repeater_Obsolete>();
	_bAnyOk=TRUE;
	if ((pad->_mode==CBgp_Repeater_Obsolete::StepByStep)||
		(pad->_mode==CBgp_Repeater_Obsolete::AbandonIfFalse)||
		(pad->_mode==CBgp_Repeater_Obsolete::EnsureTrue))
	{
		_iCurStep=_FindFirstLink(_iCurStep+1);
		if (_iCurStep==-1)
		{
			_iCurStep=_FindFirstLink(0);
			_iCurLoop++;
		}
	}
	else
		_iCurStep=_FindFirstLink(0);

	if (pad->_nLoop>0)
	{
		if (_iCurLoop>=pad->_nLoop)
		{
			_SetResult(_bAnyOk?A_Ok:A_Fail);
			return;
		}
	}

	_bNeedFire=TRUE;
}

void CBgn_Repeater_Obsolete::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	CBgp_Repeater_Obsolete*pad=_GetPad<CBgp_Repeater_Obsolete>();
	_bAnyOk=TRUE;
	if (pad->_mode==CBgp_Repeater_Obsolete::EnsureTrue)
	{
		//不做什么,继续执行_iCurStep
	}
	else
	{
		if ((pad->_mode==CBgp_Repeater_Obsolete::StepByStep)||(pad->_mode==CBgp_Repeater_Obsolete::TryUntilTrue))
		{
			_iCurStep=_FindFirstLink(_iCurStep+1);
			if (_iCurStep==-1)
			{
				_iCurStep=_FindFirstLink(0);
				_iCurLoop++;
			}
		}
		else
			_iCurStep=_FindFirstLink(0);
	}

	if (pad->_nLoop>0)
	{
		if (_iCurLoop>=pad->_nLoop)
		{
			_SetResult(_bAnyOk?A_Ok:A_Fail);
			return;
		}
	}

	_bNeedFire=TRUE;
}


////////////////////////////////////////////////////////////////////////
//CBgn_Sequence
BIND_BGN_CLASS(CBgn_Sequence,CBgp_Sequence);
int CBgn_Sequence::_FindFirstLink(int iStart)
{
	CBgp_Sequence*pad=_GetPad<CBgp_Sequence>();
	int iLast=pad->_nSteps;

	while(iStart<iLast)
	{
		if (_TestStbLink(Sqeuence_OutputStart+iStart))
			return iStart;
		iStart++;
	}
	return -1;
}

void CBgn_Sequence::_Fire(BGNOutputs &outputs)
{
	if (_iCurStep<0)
		return;
	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=(BYTE)_iCurStep;

	outputs.Add(_iCurStep+Sqeuence_OutputStart,thrd);
}

void CBgn_Sequence::Start(DWORD iStb,BGNOutputs &outputs)
{
	_iCurStep=0;
	_iCurStep=_FindFirstLink(_iCurStep);
	if (_iCurStep==-1)
	{
		_SetResult(A_Ok);
		return;
	}

	_Fire(outputs);

}

void CBgn_Sequence::Break(BGNOutputs &outputs)
{
	if (_iCurStep!=-1)
	{
		BgnThread thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=(BYTE)_iCurStep;

		outputs.thrdsBreak.push_back(thrd);
	}
}



void CBgn_Sequence::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	_iCurStep=_FindFirstLink(_iCurStep+1);
	if (_iCurStep==-1)
	{
		_SetResult(A_Ok);
		return;
	}

	_Fire(outputs);
}

void CBgn_Sequence::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	_SetResult(A_Fail);
}

////////////////////////////////////////////////////////////////////////
//CBgn_Selector
BIND_BGN_CLASS(CBgn_Selector,CBgp_Selector);
int CBgn_Selector::_FindFirstLink(int iStart)
{
	CBgp_Selector*pad=_GetPad<CBgp_Selector>();
	int iLast=pad->_nSteps;

	while(iStart<iLast)
	{
		if (_TestStbLink(Sqeuence_OutputStart+iStart))
			return iStart;
		iStart++;
	}
	return -1;
}

void CBgn_Selector::_Fire(BGNOutputs &outputs)
{
	if (_iCurStep<0)
		return;
	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=(BYTE)_iCurStep;

	outputs.Add(_iCurStep+Sqeuence_OutputStart,thrd);
}

void CBgn_Selector::Start(DWORD iStb,BGNOutputs &outputs)
{
	_iCurStep=0;
	_iCurStep=_FindFirstLink(_iCurStep);
	if (_iCurStep==-1)
	{
		_SetResult(A_Ok);
		return;
	}

	_Fire(outputs);
}

void CBgn_Selector::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	_SetResult(A_Ok);
}

void CBgn_Selector::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	_iCurStep=_FindFirstLink(_iCurStep+1);
	if (_iCurStep==-1)
	{
		_SetResult(A_Fail);
		return;
	}

	_Fire(outputs);
}

void CBgn_Selector::Break(BGNOutputs &outputs)
{
	if (_iCurStep!=-1)
	{
		BgnThread thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=(BYTE)_iCurStep;

		outputs.thrdsBreak.push_back(thrd);
	}
}



////////////////////////////////////////////////////////////////////////
//CBgn_Repeater
BIND_BGN_CLASS(CBgn_Repeater,CBgp_Repeater);

void CBgn_Repeater::_Fire(BGNOutputs &outputs)
{
	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=0;

	outputs.Add(1,thrd);
}

void CBgn_Repeater::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Repeater*pad=_GetPad<CBgp_Repeater>();

	_Fire(outputs);

}

void CBgn_Repeater::Update(BGNOutputs &outputs)
{
	if (_bNeedFire)
		_Fire(outputs);
	_bNeedFire=FALSE;

}

void CBgn_Repeater::Break(BGNOutputs &outputs)
{
	if (!_bNeedFire)
	{
		BgnThread thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=0;

		outputs.thrdsBreak.push_back(thrd);
	}
}

void CBgn_Repeater::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	CBgp_Repeater*pad=_GetPad<CBgp_Repeater>();
	if (pad->_mode==CBgp_Repeater::UntilSuccess)
	{
		_SetResult(A_Ok);
		return;
	}

	_bNeedFire=TRUE;
}

void CBgn_Repeater::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	CBgp_Repeater*pad=_GetPad<CBgp_Repeater>();
	if (pad->_mode==CBgp_Repeater::UntilFailure)
	{
		_SetResult(A_Ok);
		return;
	}


	_bNeedFire=TRUE;
}


////////////////////////////////////////////////////////////////////////
//CBgn_Ensure
BIND_BGN_CLASS(CBgn_Ensure,CBgp_Ensure);

void CBgn_Ensure::_Fire(BGNOutputs &outputs)
{
	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=0;

	outputs.Add(1,thrd);
}

void CBgn_Ensure::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Ensure*pad=_GetPad<CBgp_Ensure>();

	_nToRepeat=CSysRandom::RandVaryUInt(pad->_nMaxRepeat,pad->_nMaxRepeatVary);
	if (_nToRepeat<1)
		_nToRepeat=1;//至少做一次

	_Fire(outputs);
}

void CBgn_Ensure::Update(BGNOutputs &outputs)
{
	if (_bNeedFire)
		_Fire(outputs);
	_bNeedFire=FALSE;
}

void CBgn_Ensure::Break(BGNOutputs &outputs)
{
	if (!_bNeedFire)
	{
		BgnThread thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=0;

		outputs.thrdsBreak.push_back(thrd);
	}
}



void CBgn_Ensure::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	_SetResult(A_Ok);
	return;
}

void CBgn_Ensure::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	_nToRepeat--;
	if (_nToRepeat<=0)
	{
		_SetResult(A_Fail);
		return;
	}

	_bNeedFire=TRUE;
}


////////////////////////////////////////////////////////////////////////
//CBgn_Loop
BIND_BGN_CLASS(CBgn_Loop,CBgp_Loop);

void CBgn_Loop::_Fire(BGNOutputs &outputs)
{
	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=0;

	outputs.Add(1,thrd);
}

void CBgn_Loop::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Loop*pad=_GetPad<CBgp_Loop>();
	_nLoop=(DWORD)pad->_nLoop;
	if (_nLoop>0)
		_Fire(outputs);
}

void CBgn_Loop::Update(BGNOutputs &outputs)
{
	if (_bNeedFire)
		_Fire(outputs);
	_bNeedFire=FALSE;
}

void CBgn_Loop::Break(BGNOutputs &outputs)
{
	if (!_bNeedFire)
	{
		BgnThread thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=0;

		outputs.thrdsBreak.push_back(thrd);
	}
}


void CBgn_Loop::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	CBgp_Loop*pad=_GetPad<CBgp_Loop>();
	if (_nLoop<=1)
	{
		_SetResult(A_Ok);
		return;
	}

	_nLoop--;
	_bNeedFire=TRUE;
}

void CBgn_Loop::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	CBgp_Loop*pad=_GetPad<CBgp_Loop>();
	if (_nLoop<=1)
	{
		_SetResult(A_Ok);
		return;
	}

	_nLoop--;
	_bNeedFire=TRUE;
}


////////////////////////////////////////////////////////////////////////
//CBgn_Parallel
BIND_BGN_CLASS(CBgn_Parallel,CBgp_Parallel);

void CBgn_Parallel::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Parallel*pad=_GetPad<CBgp_Parallel>();
	DWORD nSteps=pad->_nSteps;

	_flags.resize(nSteps);
	_flags.resetAll();

	for (int i=0;i<nSteps;i++)
	{
		int iStb=Parallel_OutputStart+i;
		if (!_TestStbLink(iStb))
		{
			_flags.set(i);
			continue;
		}
		BgnThread thrd;
		thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=i;
		outputs.Add(iStb,thrd);
	}
	if (outputs.nOutputs<=0)
		_SetResult(A_Ok);

}

void CBgn_Parallel::_Break(BGNOutputs &outputs)
{
	CBgp_Parallel*pad=_GetPad<CBgp_Parallel>();

	DWORD nSteps=pad->_nSteps;

	for (int i=0;i<nSteps;i++)
	{
		int iStb=Parallel_OutputStart+i;
		if (!_TestStbLink(iStb))
			continue;

		if (_flags.test(i))
			continue;//already finished

		BgnThread thrd;
		thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=i;
		outputs.thrdsBreak.push_back(thrd);
	}
}


void CBgn_Parallel::Break(BGNOutputs &outputs)
{
	_Break(outputs);
}


void CBgn_Parallel::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	_flags.set(keyRewind);

	CBgp_Parallel*pad=_GetPad<CBgp_Parallel>();
	if (pad->_mode==CBgp_Parallel::AllReturn)
	{
		if (_flags.all())
			_SetResult(A_Ok);
	}
	else
	{
		_Break(outputs);
		_SetResult(A_Ok);
	}
}

void CBgn_Parallel::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	_flags.set(keyRewind);
	CBgp_Parallel*pad=_GetPad<CBgp_Parallel>();
	if (pad->_mode==CBgp_Parallel::AllReturn)
	{
		if (_flags.all())
			_SetResult(A_Ok);
	}
	else
	{
		_Break(outputs);
		_SetResult(A_Fail);
	}
}


////////////////////////////////////////////////////////////////////////
//CBgn_Random
BIND_BGN_CLASS(CBgn_Random,CBgp_Random);

void CBgn_Random::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Random*pad=_GetPad<CBgp_Random>();
	DWORD nSteps=pad->_nSteps;

	if (nSteps>0)
	{
		int iStb=CSysRandom::RandRangeInt<int>(0,nSteps)+Random_OutputStart;
		if (_TestStbLink(iStb))
			outputs.Add(iStb,_thrd);
	}

	_SetResult(A_Ok);

}


////////////////////////////////////////////////////////////////////////
//CBgn_Attempt
BIND_BGN_CLASS(CBgn_Attempt_Obsolete,CBgp_Attempt_Obsolete);

void CBgn_Attempt_Obsolete::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Attempt_Obsolete*pad=_GetPad<CBgp_Attempt_Obsolete>();

	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=0;
	_VerifyStbName(1,"尝试");
	outputs.Add(1,thrd);

}

void CBgn_Attempt_Obsolete::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	_OutputOk(outputs,2,"成功");
}

void CBgn_Attempt_Obsolete::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	_OutputFail(outputs,3,"失败");
}

void CBgn_Attempt_Obsolete::Break(BGNOutputs &outputs)
{
	CBgp_Attempt_Obsolete*pad=_GetPad<CBgp_Attempt_Obsolete>();

	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=0;
	outputs.thrdsBreak.push_back(thrd);
}



////////////////////////////////////////////////////////////////////////
//CBgn_Xto1
BIND_BGN_CLASS(CBgn_Xto1,CBgp_Xto1);

void CBgn_Xto1::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Xto1*pad=_GetPad<CBgp_Xto1>();

	_OutputOk(outputs,0,"出口");
}



////////////////////////////////////////////////////////////////////////
//CBgn_Fail
BIND_BGN_CLASS(CBgn_Fail,CBgp_Fail);

void CBgn_Fail::Start(DWORD iStb,BGNOutputs &outputs)
{
	_SetResult(A_Fail);
}

////////////////////////////////////////////////////////////////////////
//CBgn_Succeed
BIND_BGN_CLASS(CBgn_Succeed,CBgp_Succeed);

void CBgn_Succeed::Start(DWORD iStb,BGNOutputs &outputs)
{
	_SetResult(A_Ok);
}


////////////////////////////////////////////////////////////////////////
//CBgn_Rate
BIND_BGN_CLASS(CBgn_Rate,CBgp_Rate);
void CBgn_Rate::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Rate*pad=_GetPad<CBgp_Rate>();
	if (CSysRandom::Roll(pad->rate))
	{
		_OutputOk(outputs,1,"成功");
	}
	else
	{
		_OutputFail(outputs,2,"失败");
	}

}

////////////////////////////////////////////////////////////////////////
//CBgn_Delay_Obsolete
BIND_BGN_CLASS(CBgn_Delay_Obsolete,CBgp_Delay_Obsolete);
void CBgn_Delay_Obsolete::Start(DWORD iStb,BGNOutputs &outputs)
{
	_tStart=_GetT();
}

void CBgn_Delay_Obsolete::Update(BGNOutputs &outputs)
{
	CBgp_Delay_Obsolete*pad=_GetPad<CBgp_Delay_Obsolete>();

	AnimTick t=_GetT();
	if (t>_tStart+pad->_dur)
	{
		_OutputOk(outputs,1,"时间到");
	}
}


////////////////////////////////////////////////////////////////////////
//CBgn_Delay
BIND_BGN_CLASS(CBgn_Delay,CBgp_Delay);
void CBgn_Delay::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Delay*pad=_GetPad<CBgp_Delay>();
	float dur=CSysRandom::RandVary(pad->_dur,pad->_varyDur);
	_tEnd=_GetT()+ANIMTICK_FROM_SECOND(dur);
}

void CBgn_Delay::Update(BGNOutputs &outputs)
{
	CBgp_Delay*pad=_GetPad<CBgp_Delay>();

	AnimTick t=_GetT();
	if (t>_tEnd)
	{
		_OutputOk(outputs,1,"时间到");
	}
}


////////////////////////////////////////////////////////////////////////
//CBgn_Interrupt
BIND_BGN_CLASS(CBgn_Interrupt,CBgp_Interrupt);

BOOL CBgn_Interrupt::_Finalize(BGNOutputs &outputs)
{
	if (_bFinalized)
		return FALSE;

	if (_TestStbLink(2))
	{
		BgnThread thrd=_thrd;
		thrd.idNode=_id;
		thrd.bFinalizing=1;
		thrd.keyRewind=1;
		_VerifyStbName(2,"中断/结束");
		outputs.Add(2,thrd);

		_bFinalized=TRUE;
		return TRUE;
	}

	_bFinalized=TRUE;
	return FALSE;
}


void CBgn_Interrupt::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Interrupt*pad=_GetPad<CBgp_Interrupt>();

	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=0;
	_VerifyStbName(1,"执行");
	outputs.Add(1,thrd);

}

void CBgn_Interrupt::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	if (keyRewind==0)
	{
		_bOk=TRUE;
		if (_Finalize(outputs))
			return;
	}

	if (_bOk)
		_SetResult(A_Ok);
	else
		_SetResult(A_Fail);
}

void CBgn_Interrupt::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	if (keyRewind==0)
	{
		_bOk=FALSE;
		if (_Finalize(outputs))
			return;
	}

	if (_bOk)
		_SetResult(A_Ok);
	else
		_SetResult(A_Fail);

}

void CBgn_Interrupt::Break(BGNOutputs &outputs)
{
	if (!_bFinalized)
	{
		BgnThread thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=0;

		outputs.thrdsBreak.push_back(thrd);

		_Finalize(outputs);
	}
}


//////////////////////////////////////////////////////////////////////////
//CBgn_Strategy
BIND_BGN_CLASS(CBgn_Strategy,CBgp_Strategy);

CBgn_Decision *CBgn_Strategy::_GetParent()
{
	CBehaviorGraphNode *node=_bhv->NodeFromNodeID(_thrd.idNode);
	if (node->GetClass()==Class_Ptr2(CBgn_Decision))
	{
		return (CBgn_Decision *)node;
	}
	return NULL;
}



void CBgn_Strategy::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgn_Decision *decision=_GetParent();
	if (decision)
	{
		BgnThread thrdCond;
		if (_TestStbLink(1))
		{
			thrdCond=_thrd;
			thrdCond.idNode=_id;
			thrdCond.keyRewind=0;
		}
		BgnThread thrdExec;
		if (_TestStbLink(2))
		{
			thrdExec=_thrd;
			thrdExec.idNode=_id;
			thrdExec.keyRewind=1;
		}

		decision->RegisterStrategy(_thrd.keyRewind,thrdCond,thrdExec);
	}


//	Update(outputs);
}


void CBgn_Strategy::Update(BGNOutputs &outputs)
{
	CBgn_Decision *decision=_GetParent();
	if (!decision)
		return;

	if (decision->_iConditionToStart!=_thrd.keyRewind)
		return;

	if (decision->_iCondition!=-1)
		return;//当前有一个branch正在condition

	decision->_iCondition=decision->_iConditionToStart;
	decision->_iConditionToStart=-1;
	decision->_FireCondition(outputs);
}

void CBgn_Strategy::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	CBgn_Decision *decision=_GetParent();
	if (!decision)
		return;

	if (keyRewind==0)
	{//来自于condition的Rewind
		int iCondition=(int)_thrd.keyRewind;
		if (decision->_iCondition==iCondition)
		{
			decision->_iCondition=-1;
			int iExecute=(int)_thrd.keyRewind;
			if ((iExecute<decision->_iExecute)||(decision->_iExecute==-1))
			{
				//终止当前的execute
				if ((decision->_iExecute>=0)&&(decision->_iExecute<decision->_thrdsStrategy.size()))
				{
					outputs.thrdsBreak.push_back(decision->_thrdsStrategy[decision->_iExecute].thrdExec);
				}

				//开始新的execute
				decision->_iExecute=iExecute;
				decision->_FireExecute(outputs);
				return;
			}
		}
	}
	else
	{//来自Execute的Rewind
		int iExecute=(int)_thrd.keyRewind;
		if (decision->_iExecute==iExecute)
		{
			decision->_iExecute=-1;
			return;
		}
	}
}

void CBgn_Strategy::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	CBgn_Decision *decision=_GetParent();
	if (!decision)
		return;

	if (keyRewind==0)
	{//来自于condition的Rewind
		int iCondition=(int)_thrd.keyRewind;
		if (decision->_iCondition==iCondition)
		{
			decision->_iCondition=-1;
			iCondition++;
			if (iCondition<decision->_thrdsStrategy.size())
			{
				if ((decision->_iExecute==-1)||(iCondition<decision->_iExecute))
					decision->_iConditionToStart=iCondition;
			}
		}
	}
	else
	{//来自Execute的Rewind
		int iExecute=(int)_thrd.keyRewind;
		if (decision->_iExecute==iExecute)
		{
			decision->_iExecute=-1;
			return;
		}
	}
}


////////////////////////////////////////////////////////////////////////
//CBgn_Decision
BIND_BGN_CLASS(CBgn_Decision,CBgp_Decision);

void CBgn_Decision::RegisterStrategy(DWORD idx,BgnThread &thrdCond,BgnThread &thrdExec)
{
	if (idx<_thrdsStrategy.size())
	{
		_thrdsStrategy[idx].thrdCond=thrdCond;
		_thrdsStrategy[idx].thrdExec=thrdExec;
	}
}



void CBgn_Decision::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Decision*pad=_GetPad<CBgp_Decision>();
	DWORD nBranches=pad->_nBranches;

	int idx=0;
	for (int i=0;i<nBranches;i++)
	{
		int iStb=Decision_OutputStart+i;
		if (!_TestStbLink(iStb))
			continue;

		BgnThread thrd;
		thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=idx;
		idx++;
		outputs.Add(iStb,thrd);
	}

	_thrdsStrategy.resize(outputs.nOutputs);
	_iConditionToStart=0;
	if (outputs.nOutputs<=0)
		_SetResult(A_Ok);
}

void CBgn_Decision::_Break(BGNOutputs &outputs)
{
	CBgp_Decision*pad=_GetPad<CBgp_Decision>();

	DWORD nBranches=pad->_nBranches;
	int idx=0;
	for (int i=0;i<nBranches;i++)
	{
		int iStb=Decision_OutputStart+i;
		if (!_TestStbLink(iStb))
			continue;

		BgnThread thrd;
		thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=idx;
		idx++;
		outputs.thrdsBreak.push_back(thrd);
	}

	if (_iCondition!=-1)
		outputs.thrdsBreak.push_back(_thrdsStrategy[_iCondition].thrdCond);
	if (_iExecute!=-1)
		outputs.thrdsBreak.push_back(_thrdsStrategy[_iExecute].thrdExec);

	_iCondition=-1;
	_iExecute=-1;
}


void CBgn_Decision::Break(BGNOutputs &outputs)
{
	_Break(outputs);
}

void CBgn_Decision::_FireCondition(BGNOutputs &outputs)
{
	if (_iCondition<0)
		return;

	if (_iCondition<_thrdsStrategy.size())
	{
		if (_thrdsStrategy[_iCondition].thrdCond.IsValid())
			outputs.Add(1,_thrdsStrategy[_iCondition].thrdCond);
		else
		{
			_iExecute=_iCondition;
			_iCondition=-1;
			_FireExecute(outputs);
		}
	}
}

void CBgn_Decision::_FireExecute(BGNOutputs &outputs)
{
	if (_iExecute<0)
		return;

	if (_iExecute<_thrdsStrategy.size())
	{
		if (_thrdsStrategy[_iExecute].thrdExec.IsValid())
			outputs.Add(2,_thrdsStrategy[_iExecute].thrdExec);
		else
			_iExecute=-1;
	}
}

void CBgn_Decision::Update(BGNOutputs &outputs)
{
	if (_iCondition==-1)
	{
		if ((_iExecute==-1)||(_iExecute>0))
			_iConditionToStart=0;
	}
}


////////////////////////////////////////////////////////////////////////
//CBgn_KeepCheck
BIND_BGN_CLASS(CBgn_KeepCheck,CBgp_KeepCheck);

void CBgn_KeepCheck::_Fire(BGNOutputs &outputs)
{
	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=0;

	outputs.Add(1,thrd);
}

void CBgn_KeepCheck::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_KeepCheck*pad=_GetPad<CBgp_KeepCheck>();

	_tStart=_GetT();
	_Fire(outputs);

}

void CBgn_KeepCheck::Update(BGNOutputs &outputs)
{
	if (_bNeedFire)
		_Fire(outputs);
	_bNeedFire=FALSE;

}

void CBgn_KeepCheck::Break(BGNOutputs &outputs)
{
	if (!_bNeedFire)
	{
		BgnThread thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=0;

		outputs.thrdsBreak.push_back(thrd);
	}
}



void CBgn_KeepCheck::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	CBgp_KeepCheck*pad=_GetPad<CBgp_KeepCheck>();
	if (pad->_mode==CBgp_KeepCheck::CheckSuccess)
	{
		if (_GetT()>=_tStart+pad->_dur)
		{
			_SetResult(A_Ok);
			return;
		}
	}
	if (pad->_mode==CBgp_KeepCheck::CheckFailure)
	{
		//重置时间
		_tStart=_GetT();
	}

	_bNeedFire=TRUE;
}

void CBgn_KeepCheck::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	CBgp_KeepCheck*pad=_GetPad<CBgp_KeepCheck>();
	if (pad->_mode==CBgp_KeepCheck::CheckFailure)
	{
		if (_GetT()>=_tStart+pad->_dur)
		{
			_SetResult(A_Ok);
			return;
		}
	}
	if (pad->_mode==CBgp_KeepCheck::CheckSuccess)
	{
		//重置时间
		_tStart=_GetT();
	}

	_bNeedFire=TRUE;
}


////////////////////////////////////////////////////////////////////////
//CBgn_Always
BIND_BGN_CLASS(CBgn_Always,CBgp_Always);

void CBgn_Always::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Always*pad=_GetPad<CBgp_Always>();

	DWORD iCurFrame=((CLevelBehavior*)_bhv)->GetFrame();

	unsigned __int64 data=0;
	if (FALSE==((CLevelBehavior*)_bhv)->GetNodeData(pad->GetID(),data))
		_tStart=_GetT();
	else
	{
		DWORD iFrame=(DWORD)(data>>32);
		_tStart=(AnimTick)(data&0xffffffff);
		if (iCurFrame>iFrame+1)//这个Node上一帧没有被调用到
			_tStart=_GetT();
	}

	_SetStartTime(_tStart);


	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=0;
	outputs.Add(1,thrd);

	_bThreading=TRUE;

}

void CBgn_Always::Break(BGNOutputs &outputs)
{
	if (_bThreading)
	{
		BgnThread thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=0;

		outputs.thrdsBreak.push_back(thrd);
	}
}

void CBgn_Always::_SetStartTime(AnimTick t)
{
	DWORD iCurFrame=((CLevelBehavior*)_bhv)->GetFrame();
	unsigned __int64 data;
	data=iCurFrame;
	data<<=32;
	data|=(unsigned __int64)t;

	CBgp_Always*pad=_GetPad<CBgp_Always>();
	((CLevelBehavior*)_bhv)->SetNodeData(pad->GetID(),data);
}

void CBgn_Always::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	_bThreading=FALSE;
	CBgp_Always*pad=_GetPad<CBgp_Always>();
	if (pad->_mode==CBgp_Always::CheckSuccess)
	{
		_SetStartTime(_tStart);
		if (_GetT()>=_tStart+pad->_dur)
		{
			_SetResult(A_Ok);
			return;
		}
	}
	if (pad->_mode==CBgp_Always::CheckFailure)
	{
		//重置时间
		_SetStartTime(_GetT());
	}

	_SetResult(A_Fail);
}

void CBgn_Always::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	_bThreading=FALSE;
	CBgp_Always*pad=_GetPad<CBgp_Always>();
	if (pad->_mode==CBgp_Always::CheckFailure)
	{
		_SetStartTime(_tStart);
		if (_GetT()>=_tStart+pad->_dur)
		{
			_SetResult(A_Ok);
			return;
		}
	}
	if (pad->_mode==CBgp_Always::CheckSuccess)
	{
		//重置时间
		_SetStartTime(_GetT());
	}

	_SetResult(A_Fail);
}


////////////////////////////////////////////////////////////////////////
//CBgn_WeaksMod
BIND_BGN_CLASS(CBgn_WeaksMod,CBgp_WeaksMod);

BOOL CBgn_WeaksMod::_Finalize(BGNOutputs &outputs)
{
	if (_bFinalized)
		return FALSE;

	if (_bOverriden)
	{
		LevelBehaviorContext *ctx=_GetCtx();
		CBgp_WeaksMod*pad=_GetPad<CBgp_WeaksMod>();

		if (pad->bBackup)
		{
			LevelAttr_Weaks *attrWeaks=ctx->lo->GetAttr_Weaks();
			if (attrWeaks)
			{
				attrWeaks->Pop();
			}
		}
		_bOverriden=FALSE;
	}

	_bFinalized=TRUE;
	return FALSE;
}


void CBgn_WeaksMod::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_WeaksMod*pad=_GetPad<CBgp_WeaksMod>();
	LevelBehaviorContext *ctx=_GetCtx();

	LevelAttr_Weaks *attrWeaks=ctx->lo->GetAttr_Weaks();
	if (attrWeaks)
	{
		if (pad->bBackup)
			attrWeaks->Push();

		switch(pad->mode)
		{
			case CBgp_WeaksMod::Mode_Set:
			{
				LevelWeaksPack wkpk;
				pad->weaksOverride.ToWeakPack(wkpk);
				attrWeaks->Cur()=wkpk;
				break;
			}
			case CBgp_WeaksMod::Mode_Modify:
			{
				LevelWeaksPack wkpk;
				pad->weaksAdd.ToWeakPack(wkpk);
				attrWeaks->Cur().MergeFrom(wkpk);
				wkpk.Zero();
				pad->weaksRemove.ToWeakPack(wkpk);
				attrWeaks->Cur().Exclude(wkpk);
				break;
			}
			case CBgp_WeaksMod::Mode_Filter:
			{
				LevelWeaksPack wkpk;
				pad->weaksFilter.ToWeakPack(wkpk);
				attrWeaks->Cur().Filter(wkpk);
				break;
			}
		}

		_bOverriden=TRUE;
	}

	if (pad->bBackup)
	{
		BgnThread thrd=_thrd;
		thrd.idNode=_id;
		thrd.keyRewind=0;
		_VerifyStbName(1,"出口");
		outputs.Add(1,thrd);
	}
	else
		_OutputOk(outputs,1,"出口");
}

void CBgn_WeaksMod::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	_Finalize(outputs);

	_SetResult(A_Ok);
}

void CBgn_WeaksMod::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	_Finalize(outputs);

	_SetResult(A_Fail);
}

void CBgn_WeaksMod::Break(BGNOutputs &outputs)
{
	_Finalize(outputs);
}

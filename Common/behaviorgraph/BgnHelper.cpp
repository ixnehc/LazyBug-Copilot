/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 核心的BGN
*********************************************************************/
#include "stdh.h"
#include "BehaviorGraphs.h"
#include "Behavior.h"

#include "BgnHelper.h"
#include "BehaviorCustomConst.h"

#include "BehaviorValue.h"

#include "Random/Random.h"

BIND_BGN_CLASS(CBgn_Graph,CBgp_Graph);

void CBgn_Graph::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Graph *pad=_GetPad<CBgp_Graph>();

	BgnThread thrd;
	thrd.padState=PadID_Null;
	thrd.idNode=BgnID_Invalid;
	_VerifyStbName(0,"启动");
	outputs.Add(0,thrd);
	_SetResult(A_Ok);
}


BIND_BGN_CLASS(CBgn_Counter,CBgp_Counter);
BIND_BGN_CLASS(CBgn_Timer,CBgp_Timer);
BIND_BGN_CLASS(CBgn_Register,CBgp_Register);



BIND_BGN_CLASS(CBgn_Consts,CBgp_Consts);


void FillValuesDesc(std::string &s,FillDescAssist *assist,std::vector<BhvValDeclare*> &values)
{
	s="";
	BOOL bFirst=TRUE;
	for (int i=0;i<values.size();i++)
	{
		BhvValDeclare*value=values[i];
		if (!bFirst)
			s+="\n\r";
		bFirst=FALSE;
		AppendFmtString(s,"%s : ",StrLib_GetStr(value->nm));

		if (value->tp.tpMem!=BehaviorMemType_None)
			s+=GetBehaviorMemTypeDesc((BehaviorMemType)value->tp.tpMem);
		else
			s+=value->tp.subname;
	}

	if (s=="")
		s="n/a";
}

void CBgp_Consts::FillDesc(std::string &s,FillDescAssist *assist)
{
	if (_declares2.size()>0)
	{
		std::vector<BhvValDeclare *> declares;
		declares.resize(_declares2.size());
		for (int i=0;i<_declares2.size();i++)
			declares[i]=&_declares2[i];
		FillValuesDesc(s,assist,declares);
	}
}

BIND_BGN_CLASS(CBgn_Vars,CBgp_Vars);

void CBgp_Vars::FillDesc(std::string &s,FillDescAssist *assist)
{
	if (_declares2.size()>0)
	{
		s="";
		BOOL bFirst=TRUE;
		for (int i=0;i<_declares2.size();i++)
		{
			BhvVarDeclare *declare=&_declares2[i];
			if (!bFirst)
				s+="\n\r";
			AppendFmtString(s,"%s : ",StrLib_GetStr(declare->nm));
			s+=GetBehaviorMemTypeDesc((BehaviorMemType)declare->tp);
			bFirst=FALSE;
		}
	}
}



BIND_BGN_CLASS(CBgn_Proxy,CBgp_Proxy);

void CBgn_Proxy::Start(DWORD iStb,BGNOutputs &outputs)
{
	_OutputOk(outputs,1,"出口");
}


BIND_BGN_CLASS(CBgn_Troops,CBgp_Troops);

void CBgp_Troops::FillDesc(std::string &s,FillDescAssist *assist)
{
	if (_declares.size()>0)
	{
		s="";
		BOOL bFirst=TRUE;
		for (int i=0;i<_declares.size();i++)
		{
			if (!bFirst)
				s+="\n\r";
			AppendFmtString(s,"%s",StrLib_GetStr(_declares[i]));
			bFirst=FALSE;
		}
	}
}



BIND_BGN_CLASS(CBgn_Roll,CBgp_Roll);

void CBgn_Roll::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Roll *pad=_GetPad<CBgp_Roll>();

	float rate=pad->_rate;

	if (pad->_bTimedRate)
	{
		DWORD iCurFrame=_bhv->GetFrame();
		AnimTick tStartTime=_bhv->GetStartTime();

		float gap=1.0f;
		DWORD nFrames=iCurFrame+1;
		gap=((float)(_GetT()-tStartTime))/(float)(nFrames);
		gap=ANIMTICK_TO_SECOND(gap);

		float c=gap/1.0f;

		rate=1.0f-pow((1.0f-pad->_rate),c);
	}

	if (CSysRandom::Roll(rate))
		_OutputOk(outputs,1,"成功");
	else
		_OutputFail(outputs,2,"失败");
}

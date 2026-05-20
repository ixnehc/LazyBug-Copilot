/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LoAgent.h"
#include "LevelRecordAgent.h"
#include "LevelDetectWeights.h"
#include "LevelUtil.h"

#include "BgnDetectAgent.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectSpecifiedAgent
BIND_BGN_CLASS(CBgn_DetectSpecifiedAgent,CBgp_DetectSpecifiedAgent);
void CBgn_DetectSpecifiedAgent::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_DetectSpecifiedAgent*pad=_GetPad<CBgp_DetectSpecifiedAgent>();

	CLevelObj *loDetect=NULL;
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		LevelObjMapEnumCallBack dlgt;
		dlgt.bind(this,&CBgn_DetectSpecifiedAgent::_EnumCallBack);
		LevelDetectTargetFlag flag=pad->flagsDetect;
		flag=(LevelDetectTargetFlag)((DWORD)flag|LevelDetectTarget_Agent|(DWORD)LevelDetectTargetFlag_Method);

		LevelUtilDetectParam param;
		param.loSrc=lo;
		param.pos=lo->GetFramePos();
		param.flags=&flag;
		param.nFlags=1;
		param.requires=NULL;
		param.nRequires=0;
		param.rangeMin=pad->rangeMin;
		param.rangeMax=pad->range;
		param.weights.CopyFrom(pad->weights);

		extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
		loDetect=LevelUtil_DetectBest(param,dlgt);
	}

	if (loDetect)
	{
		if (pad->nmVar!=StringID_Invalid)
			_SetID(pad->nmVar,BehaviorMemType_ObjID,loDetect->GetID());
		_OutputOk(outputs,1,"侦测到");
		return;
	}

	if (pad->nmVar!=StringID_Invalid)
		_SetID(pad->nmVar,BehaviorMemType_ObjID,LevelObjID_Invalid);
	_OutputFail(outputs,2,"未侦测到");
}

BOOL CBgn_DetectSpecifiedAgent::_EnumCallBack(CLevelObj *lo,float dist2)
{
	if(lo->GetType()!=LevelObjType_Agent)
		return FALSE;
	LevelRecordAgent *rec=((CLoAgent*)lo)->GetRec();
	if (!rec)
		return FALSE;
	CBgp_DetectSpecifiedAgent*pad=_GetPad<CBgp_DetectSpecifiedAgent>();
	if (rec->GetID()!=pad->idAgent)
		return FALSE;

	if(pad->idBuff!=RecordID_Invalid)
	{
		extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
		if (!LevelUtil_FindBuffByRecordID(lo,pad->idBuff))
			return FALSE;
	}

	return TRUE;
}

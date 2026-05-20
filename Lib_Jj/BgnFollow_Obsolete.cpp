/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelBehavior.h"
#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "BgnFollow_Obsolete.h"

#include "LevelSkillDriver.h"

#include "LevelUtil.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Follow_Obsolete
BIND_BGN_CLASS(CBgn_Follow_Obsolete,CBgp_Follow_Obsolete);

void CBgn_Follow_Obsolete::_UpdateFollow(AnimTick t)
{
	if (_bTimeUp)
		return;
	CLevelObj *lo=_GetLo();
	LevelBehaviorContext *ctx=_GetCtx();

	CBgp_Follow_Obsolete*pad=_GetPad<CBgp_Follow_Obsolete>();
	if (pad->dur!=0)
	{
		if (t>_tStart+pad->dur)
		{
			_bTimeUp=TRUE;
			extern BOOL LevelUtil_StopMove(CLevelObj *lo);
			LevelUtil_StopMove(lo);
			return;
		}
	}

	if (_idFollow==LevelObjID_Invalid)
	{
		CLevelObj *loDetect=NULL;
		if (pad->bLockPlayer)
			loDetect=_GetLockLo();
		else
		{
			if (lo)
			{
				LevelObjRequire requires[]={LevelObjRequire_Attackable};

				LevelUtilDetectParam param;
				param.loSrc=lo;
				param.pos=lo->GetFramePos();
				param.flags=&pad->flagsDetect[0];
				param.nFlags=pad->flagsDetect.size();
				param.requires=requires;
				param.nRequires=ARRAY_SIZE(requires);
				param.rangeMin=0.0f;
				param.rangeMax=pad->radius;
				param.weights.AddFlag(LevelDetectWeights_Dist);
				param.weights.wtDist=100.0f;

				extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
				loDetect=LevelUtil_DetectBest(param,NULL);
			}
		}

		if (loDetect)
		{
			if (loDetect->GetFramePos().getDistanceSQFrom(lo->GetFramePos())>=pad->dist*pad->dist)
			{
				extern BOOL LevelUtil_Follow(CLevelObj *lo,CLevelObj *loTarget,float range,BOOL bClosestFollow);
				LevelUtil_Follow(lo,loDetect,-1.0f,TRUE);
				_idFollow=loDetect->GetID();
			}
			_tFollow=t;
		}
	}
	else
	{
		CLevelObj *loFollow=ctx->level->GetIDs()->LoFromID(_idFollow);
		if (!loFollow)
			_idFollow=LevelObjID_Invalid;
		else
		{
			if (loFollow->GetFramePos().getDistanceSQFrom(lo->GetFramePos())<pad->dist*pad->dist)
			{
				extern BOOL LevelUtil_StopMove(CLevelObj *lo);
				LevelUtil_StopMove(lo);
				_idFollow=LevelObjID_Invalid;
			}
		}
	}
}

void CBgn_Follow_Obsolete::Start(DWORD iStb,BGNOutputs &outputs)
{
	_tStart=_GetT();
	_UpdateFollow(_tStart);
}

void CBgn_Follow_Obsolete::Update(BGNOutputs &outputs)
{
	_UpdateFollow(_GetT());
	if (_bTimeUp)
	{
		_OutputOk(outputs,1,"结束");
	}
}

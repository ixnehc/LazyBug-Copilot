/********************************************************************
	created:	2016/09/14 
	author:		cxi
	
	purpose:	 攻击Threat
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnThreat_KeepDist.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"
#include "LevelTroops.h"

#include "LevelUtil.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_KeepDist
BIND_BGN_CLASS(CBgnThreat_KeepDist,CBgpThreat_KeepDist);

void CBgnThreat_KeepDist::Destroy()
{
	CLevelObj *lo=_GetLo();

	if (TRUE)
	{
		CUnit *unit=lo->GetUnit();
		if (unit)
			unit->ClearOverrideFace();
	}

	SAFE_RELEASE(_target);
}


extern BOOL LevelUtil_Flee(CLevelObj *lo,CLevelObj *loEscapeFrom,float distKeep,int &signAvoid,CLevelObj *loCenter,float radius);
void CBgnThreat_KeepDist::_Start(CLevelObj *target,BOOL bEscapce)
{
	CBgpThreat_KeepDist*pad=_GetPad<CBgpThreat_KeepDist>();
	AnimTick t=_GetT();

	CLevelObj *lo=_GetLo();
	LevelBehaviorContext *ctx=_GetCtx();

	if (bEscapce)
	{
		//目标离自己太近,我们先要离远一点
		int signAvoid=0;

		CLevelObj *loCenter=NULL;
		float radiusToCenter=0.0f;
		if(pad->radiusToOwner>0.0f)
		{
			CLevelObj *loOwner=LevelUtil_GetOwnerLo(lo);
			if (loOwner)
			{
				loCenter=loOwner;
				radiusToCenter=pad->radiusToOwner;
			}
		}
		LevelUtil_Flee(lo,target,pad->distMax,signAvoid,loCenter,radiusToCenter);

		_state=Escape;
		_tStateStart=t;

		SAFE_REPLACE(_target,target);

		return;
	}
	else
	{
		//开始Follow

		LevelUtil_Follow(lo,target,-1.0f,TRUE);

		_state=Follow;	
		_tStateStart=t;

		SAFE_REPLACE(_target,target);

		return;
	}
}

void CBgnThreat_KeepDist::_Stop()
{
	if (_GetLo())
	{
		LevelUtil_StopMove(_GetLo());
	}

	SAFE_RELEASE(_target);
	_state=None;

}


void CBgnThreat_KeepDist::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpThreat_KeepDist*pad=_GetPad<CBgpThreat_KeepDist>();
	if (pad->distMin>=pad->distMax)
	{
		_OutputOk(outputs,1,"结束");//无效参数
		return;
	}

	Update(outputs);
}

BOOL CBgnThreat_KeepDist::_IsTooClose(CLevelObj *lo,CLevelObj *target)
{
	CBgpThreat_KeepDist*pad=_GetPad<CBgpThreat_KeepDist>();
	return (target->GetFramePos().getDistanceSQFrom(lo->GetFramePos())<pad->distMin*pad->distMin);
}

BOOL CBgnThreat_KeepDist::_IsTooFar(CLevelObj *lo,CLevelObj *target)
{
	CBgpThreat_KeepDist*pad=_GetPad<CBgpThreat_KeepDist>();
	return (target->GetFramePos().getDistanceSQFrom(lo->GetFramePos())>pad->distMax*pad->distMax);
}

BOOL CBgnThreat_KeepDist::_IsFarEnough(CLevelObj *lo,CLevelObj *target)
{
	CBgpThreat_KeepDist*pad=_GetPad<CBgpThreat_KeepDist>();
	float distMid=(pad->distMax+pad->distMin)/2.0f;
	return (target->GetFramePos().getDistanceSQFrom(lo->GetFramePos())>distMid*distMid);
}

BOOL CBgnThreat_KeepDist::_IsCloseEnough(CLevelObj *lo,CLevelObj *target)
{
	CBgpThreat_KeepDist*pad=_GetPad<CBgpThreat_KeepDist>();
	float distMid=(pad->distMax+pad->distMin)/2.0f;
	return (target->GetFramePos().getDistanceSQFrom(lo->GetFramePos())<distMid*distMid);
}


void CBgnThreat_KeepDist::_Respond(CLevelObj *lo,CLevelObj *target)
{
	if (target)
	{
		if (_IsTooClose(lo,target))
			_Start(target,TRUE);
		else
		{
			if (_IsTooFar(lo,target))
				_Start(target,FALSE);
		}
	}
	else
		_Stop();
}


void CBgnThreat_KeepDist::Update(BGNOutputs &outputs)
{
	CBgpThreat_KeepDist*pad=_GetPad<CBgpThreat_KeepDist>();
	AnimTick t=_GetT();
	CLevelObj *lo=_GetLo();

	LevelBehaviorContext *ctx=_GetCtx();
	CLevelObj *target=_GetThreat();
	VerifyLevelObjAlive(_target);

	switch(_state)
	{
		case None:
		{
			_Respond(lo,target);
			_tLastCheck=t;
			break;
		}
		case Escape:
		{
			if (target==_target)
			{
				if (target)
				{
					if (_IsFarEnough(lo,target))
						_Stop();
					if (t>_tLastCheck+pad->durCheck)
					{
						_Start(target,TRUE);
						_tLastCheck=t;
					}
				}
			}
			else
			{
				_Respond(lo,target);
				_tLastCheck=t;
			}
			break;
		}
		case Follow:
		{
			if (target==_target)
			{
				if (target)
				{
					if (_IsCloseEnough(lo,target))
						_Stop();
					if (t>_tLastCheck+pad->durCheck)
					{
						_Start(target,FALSE);
						_tLastCheck=t;
					}
				}
			}
			else
			{
				_Respond(lo,target);
				_tLastCheck=t;
			}
			break;
		}
	}

	if (pad->bKeepFacingThreat)
	{
		if (target)
		{
			LevelPos dir=target->GetFramePos()-lo->GetFramePos();
			if (dir.getLengthSQ()>0.01f)
			{
				CUnit *unit=lo->GetUnit();
				if (unit)
					unit->OverrideFace(LevelFaceFromDir(dir));
			}
		}
	}

	if (!target)
		_OutputOk(outputs,1,"结束");//没有Threat结束
}


void CBgnThreat_KeepDist::Break(BGNOutputs &outputs)
{
	Destroy();
}




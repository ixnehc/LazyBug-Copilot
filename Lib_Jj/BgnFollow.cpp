/********************************************************************
	created:	2016/11/01 
	author:		cxi
	
	purpose:	 跟随
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelBehavior.h"
#include "LevelObjMap.h"
#include "LevelObj.h"
#include "LevelObjMove.h"
#include "Level.h"
#include "BgnFollow.h"

#include "LevelSkillDriver.h"

#include "LevelUtil.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Follow
BIND_BGN_CLASS(CBgn_Follow,CBgp_Follow);

void CBgn_Follow::Destroy()
{
	LevelUtil_StopMove(_GetLo());
}



void CBgn_Follow::_UpdateFollow(AnimTick t)
{
	if (_idFollow==LevelObjID_Invalid)
		return;
	CLevelObj *lo=_GetLo();
	LevelBehaviorContext *ctx=_GetCtx();

	CBgp_Follow*pad=_GetPad<CBgp_Follow>();
	if (pad->dur!=0)
	{
		if (t>_tStart+pad->dur)
		{
			_idFollow=LevelObjID_Invalid;
			extern BOOL LevelUtil_StopMove(CLevelObj *lo);
			LevelUtil_StopMove(lo);
			return;
		}
	}

	if (_idFollow!=LevelObjID_Invalid)
	{
		CLevelObj *loFollow=LevelUtil_GetAliveLo(_GetLevel(),_idFollow);
		if (!loFollow)
		{
			_idFollow=LevelObjID_Invalid;
		}
		else
		{
			extern BOOL LevelUtil_CheckFollow(CLevelObj *lo,CLevelObj *loTarget);
			if (!LevelUtil_CheckFollow(lo,loFollow))
			{
				if (!CLevelDecider::CheckInRange(lo,loFollow,pad->dist+0.1f))
				{
					float avoid=-1.0f;
					CUnit *unit=lo->GetMove()->GetGroundUnit();
					if (unit)
						avoid=unit->GetLastAvoidRad();
					LevelUtil_Follow(lo,loFollow,pad->dist,pad->bClosestFollow);
					if (unit)
						unit->SetLastAvoidRad(avoid);
				}
				else
				{
					if (pad->bFinishWhenReached)
						_idFollow=LevelObjID_Invalid;
				}
			}
		}
	}
}

void CBgn_Follow::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Follow*pad=_GetPad<CBgp_Follow>();
	_tStart=_GetT();
	if (_GetID(pad->nmVar,BehaviorMemType_ObjID,_idFollow))
	{
		_UpdateFollow(_tStart);
	}
}

void CBgn_Follow::Update(BGNOutputs &outputs)
{
	_UpdateFollow(_GetT());
	if (_idFollow==LevelObjID_Invalid)
	{
		_OutputOk(outputs,1,"结束");
	}
}

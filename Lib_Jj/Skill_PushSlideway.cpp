/********************************************************************
	created:	2022/12/02 
	author:		cxi
*********************************************************************/
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "Skill_PushSlideway.h"

#include "LevelRecordSkill.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "LevelDecider.h"
#include "LevelRecords.h"
#include "LevelRecordGlobal.h"

#include "LoSlideway.h"


#include "timer/timer.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_General
BIND_SKILLPARAM(Skill_PushSlideway,SkillParam_PushSlideway);


void Skill_PushSlideway::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);
	_SetState(SkillState_Casting);

//	GetLevel()->GetDbgDraw().DrawCircle(_owner->GetFramePos(),0.06f,RGB(0,0,255),5.f);

}



void Skill_PushSlideway::_OnUpdate(AnimTick dt)
{
	if (_state==SkillState_Casting)
	{
		CLevel *level=GetLevel();
//		level->GetDbgDraw().DrawCircle(_owner->GetFramePos(),0.08f,RGB(0,255,0),5.0f);

		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		CLevelObj *loTarget=LevelUtil_GetTargetObj(level,_target);
		if (loTarget->GetType()==LevelObjType_Agent)
		{
			CLoSlideway *loSlideway=loTarget->ToPtr<CLoSlideway>();
			if (loSlideway)
			{
				LosSlideway *los=loSlideway->GetLos<LosSlideway>();
				if (los)
				{
					float spDrain=los->spDrain*ANIMTICK_TO_SECOND(_tCasting);

					if (spDrain>_spConsumed)
					{
						level->GetDecider()->CommitSPDrain(GetOwner(),spDrain-_spConsumed);
						_spConsumed=spDrain;
					}
				}

			}
		}

	}
}

void Skill_PushSlideway::_OnBreak()
{
	_Finish();
}


void Skill_PushSlideway::_Finish()
{
	_SetState(SkillState_Finished);
}


void Skill_PushSlideway::StopCast(AnimTick tCasting)
{
	_Finish();
}

void Skill_PushSlideway::NotifyCasted()
{
	_Finish();
}

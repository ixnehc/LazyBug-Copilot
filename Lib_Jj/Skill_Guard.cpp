
#include "stdh.h"


#include "Skill_Guard.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelDecider.h"

#include "LevelObjMove.h"

#include "LevelBlocking.h"

#include "LevelOSB.h"



//////////////////////////////////////////////////////////////////////////
//CSkill_Guard
BIND_SKILLPARAM(Skill_Guard,SkillParam_Guard);


void Skill_Guard::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_SetState(SkillState_Casting);

	CLevelBlocking *blocking=_owner->GetBlocking();
	if (blocking)
		blocking->Activate();

}


void Skill_Guard::_OnUpdate(AnimTick dt)
{
	SkillParam_Guard*param=(SkillParam_Guard*)_param;

	if (GetState()==SkillState_Casting)
	{
		//累加Cast的时间
		extern void LevelUtil_AccumCastingTime(CLevelObj *lo,AnimTick dt,AnimTick &tCasting);
		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		if (_tCasting>_rec->HitDelay)
		{
// 				_tGuarding=0;
		}
	}

}


void Skill_Guard::_Finish()
{
	if (_state==SkillState_Finished)
		return;


	//通知Client这个技能Cast完了
	if (TRUE)
	{
		LevelOp_SkillCasted *op=NewOp<LevelOp_SkillCasted>(LevelOpLink());
		_owner->AddOp(op);
	}

	CLevelBlocking *blocking=_owner->GetBlocking();
	if (blocking)
		blocking->Deactiveate();


	_SetState(SkillState_Finished);


}

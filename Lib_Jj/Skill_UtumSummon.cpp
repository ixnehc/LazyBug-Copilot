
#include "stdh.h"


#include "Skill_UtumSummon.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelDecider.h"
#include "Buff_UtumBirth.h"

#include "LevelRtnus.h"
#include "LevelBehavior.h"

#include "behaviorgraph/BehaviorMem.h"

#include "LevelRecordUnit.h"

#include "Ability_UtumTide.h"


#include "Buff_Dead.h"


//////////////////////////////////////////////////////////////////////////
//Skill_UtumSummon

BIND_SKILLPARAM(Skill_UtumSummon,SkillParam_UtumSummon);


void Skill_UtumSummon::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_SetState(SkillState_Casting);

	_tCasting=0;
	_UpdateSummon(0);
}

void Skill_UtumSummon::_UpdateSummon(AnimTick dt)
{
	_tCasting+=dt;

	if(_bSummon)
	{
		if (_tCasting>=_rec->CastTime)
		{
			_SetState(SkillState_Finished);
		}
	}
	else
	{
		if (_tCasting>=_rec->HitDelay)
		{
			_DoSummon();
			_bSummon=TRUE;
		}
	}

}

void Skill_UtumSummon::_OnUpdate(AnimTick dt)
{
	_UpdateSummon(dt);

}

void Skill_UtumSummon::_DoSummon()
{
// 	CLevelRecords *records=_owner->GetLevel()->GetRecords();
// 	CLevel *level=_owner->GetLevel();
// 
// 	SkillParam_UtumSummon*param=_rec->GetParam<SkillParam_UtumSummon>();
// 
// 	if (param&&_arg)
// 	{
// 		CLevelDecider *decider=level->GetDecider();
// 
// 		CLevelAbility_UtumTide *ability=NULL;
// 		if (TRUE)
// 		{
// 			extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
// 			CLevelPlayer *player=LevelUtil_PlayerFromLo(_owner);
// 			if (player)
// 				ability=(CLevelAbility_UtumTide *)player->GetAbilities().GetActiveAbility(LevelAbilityType_UtumTide);
// 		}
// 
// 
// 		LevelPos3D pos3D0=_owner->GetFramePos3D();
// 		pos3D0.y+=_owner->GetCastHeight();
// 		DWORD nArgs=_arg->data.size()/sizeof(UtumSummonArg);
// 		UtumSummonArg *args=(UtumSummonArg*)&_arg->data[0];
// 		if (nArgs>param->count)
// 			nArgs=param->count;
// 		for (int i=0;i<nArgs;i++)
// 		{
// 			UtumSummonArg*arg=&args[i];
// 			CLoUnit* lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));
// 			LevelPos posOff=arg->GetOff();
// 			LevelPos3D pos3D=pos3D0;
// 			pos3D.x+=posOff.x;
// 			pos3D.z+=posOff.y;
// 			lo->PostCreate(_owner->GetPlayerID(),NULL,param->idUnit,_grd,NULL,EquipSetPick_None,pos3D0);
// 
// 			//填充冲刺方向变量
// 			if (TRUE)
// 			{
// 				CLevelBehavior *bhv=lo->GetBehaviorAI();
// 				if (bhv)
// 				{
// 					CBehaviorMem *mem=bhv->GetMem(0);
// 					LevelPos dir=_target.Aim()-_owner->GetFramePos();
// 					dir.normalize();
// 					mem->SetFloat(param->nmThrustEuler,dir.getEuler());
// 				}
// 			}
// 
// 			level->AddToActives(lo);
// 
// 			//Birth的Buff
// 			BuffArg_UtumBirth arg2;
// 			arg2.posStart=pos3D0;
// 			arg2.posEnd=pos3D;
// 
// 			decider->MakeBuff(lo,param->idBirthBuff,ANIMTICK_FROM_SECOND(1.0f),&arg2,LevelOpLinkID_Invalid);
// 
// 			if (ability)
// 				ability->AddSummon(lo->GetID());
// 
// 			SAFE_RELEASE(lo);
// 		}
// 
// 	}
}



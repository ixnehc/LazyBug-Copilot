
#include "stdh.h"

#include "LevelEvents.h"

#include "LevelOSB.h"

#include "Ability_TeleportSword.h"

/*
BloodTeeth:

DeathCall:

FlameBlade:

FlashSwing:

HonorSword:

LightningBow:

ObliterateBow:

PhantomDagger:

SkullSword:

TeleportSword:

*/

 
//////////////////////////////////////////////////////////////////////////
//CUpgradeTeleportSword_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeTeleportSword_Init);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeTeleportSword_LevelUp);


BOOL CUpgradeTeleportSword_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_TeleportSword *ability=(CLevelAbility_TeleportSword *)ability_;

	ability->_idSkill=idSkill;
	ability->_idDefSkill=idDefaultSkill;

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_TeleportSword

void CLevelAbility_TeleportSword::_InitTechs()
{
}

void CLevelAbility_TeleportSword::_OnBuildRT()
{
	_BuildGradeRT();
	_AddSkillRT(LevelAbilityAction_AttackA,_idDefSkill);
	_AddSkillRT(LevelAbilityAction_FuryA,_idSkill);
}

void CLevelAbility_TeleportSword::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_TeleportSword::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);
}

void CLevelAbility_TeleportSword::_LoadSync(CDataPacket &dp,CRecords *records)
{
	_LoadSync_SkillsRT(dp,records);
}


void CLevelAbility_TeleportSword::_OnUpdate(LevelTick dt0)
{
}


void CLevelAbility_TeleportSword::_OnEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_PreDamage)
	{
		LePreDamage &e=(LePreDamage &)e0;
		if (e.osb)
		{
			CLevelSkill *skill=e.osb->GetSkill();
			if (skill)
			{
				LevelRecordSkill *recSkill=skill->GetRec();
				RecordID idSkill=recSkill->GetID();

				if (idSkill==_idSkill)
				{
					LevelSkillTarget &target=skill->GetTarget();
					if (target.bOrg)
					{
						float dist=_owner->GetFramePos().getDistanceFrom(target.org);
						CUpgradeTeleportSword_Init *upgrade=_GetInitialUpgrade<CUpgradeTeleportSword_Init>();
						if (upgrade)
						{
							float multiply=upgrade->vsDmgMultiply.GetFloat(ANIMTICK_FROM_SECOND(dist));
							e.scaleDmg*=multiply;
						}
					}
				}
			}
		}
	}

}

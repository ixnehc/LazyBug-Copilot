
#include "stdh.h"

#include "Protocal.h"

#include "Ability_Bomb.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelRecordEO.h"

#include "LevelEvents.h"
#include "LevelOSB.h"


//////////////////////////////////////////////////////////////////////////
//CUpgradeBomb_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeBomb_Init);
BOOL CUpgradeBomb_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_Bomb *ability=(CLevelAbility_Bomb *)ability_;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CLevelAbility_Bomb

void CLevelAbility_Bomb::_OnBuildRT()
{
	_BuildGradeRT();

	CUpgradeBomb_Init *upgradeInitial=_upgradeInitial;

	_AddSkillRT(LevelAbilityAction_AttackA,upgradeInitial->idSkill);

}

void CLevelAbility_Bomb::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_Bomb::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);
}

void CLevelAbility_Bomb::_LoadSync(CDataPacket &dp,CRecords *recordsSkill)
{
	_LoadSync_SkillsRT(dp,recordsSkill);
}

void CLevelAbility_Bomb::_UpdateStackCount()
{
	extern LevelItemState *LevelUtil_GetRawArtifactItemState(CLevelObj *lo,LevelArtifactType tp);
	LevelItemState *is=LevelUtil_GetRawArtifactItemState(_owner,LevelArtifact_Bomb);
	if (is)
	{
		_SetStack(LevelAbilityAction_AttackA,is->nStack);
	}
}

void CLevelAbility_Bomb::_OnUpdate(LevelTick dt)
{
}

void CLevelAbility_Bomb::_OnBuildArtifactState(LevelItemState &state)
{
}



void CLevelAbility_Bomb::_OnEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_PostCreateEo)
	{
		LePostCreateEo &e=(LePostCreateEo &)e0;	
		if (e.osbSrc)
		{
			CLevelSkill *skill=e.osbSrc->GetSkill();
			if (skill)
			{
				LevelRecordSkill *recSkill=skill->GetRec();
				RecordID idSkill=recSkill->GetID();

				if (idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_AttackA))
				{
					extern int LevelUtil_DecArtifactStack(CLevelObj *lo,LevelArtifactType tpArtifact,int nStack);
					LevelUtil_DecArtifactStack(_owner,LevelArtifact_Bomb,1);
				}
			}
		}
	}


}

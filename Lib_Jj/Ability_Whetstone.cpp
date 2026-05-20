
#include "stdh.h"

#include "Protocal.h"

#include "Ability_Whetstone.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelRecordEO.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeWhetstone_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeWhetstone_Init);
BOOL CUpgradeWhetstone_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_Whetstone *ability=(CLevelAbility_Whetstone *)ability_;

	return TRUE;
}

BOOL CUpgradeWhetstone_Init::ExistSkillID(RecordID idSkill)
{
	if (cacheSkills.empty())
	{
		for (int i=0;i<skills.size();i++)
			cacheSkills.insert(skills[i]);
	}

	if (cacheSkills.find(idSkill)!=cacheSkills.end())
		return TRUE;
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_Whetstone

CLevelAbility_Whetstone::~CLevelAbility_Whetstone()
{
	GDestructor();
}


void CLevelAbility_Whetstone::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_Whetstone::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_Whetstone::_SaveSync(CDataPacket &dp)
{
}

void CLevelAbility_Whetstone::_LoadSync(CDataPacket &dp,CRecords *records)
{
}


void CLevelAbility_Whetstone::_OnBuildArtifactState(LevelItemState &state)
{
}

void CLevelAbility_Whetstone::_OnEvent(LevelEvent &e0)
{
	CUpgradeWhetstone_Init *upgradeInitial=(CUpgradeWhetstone_Init*)_upgradeInitial;

	if (e0.GetType()==LET_ModDamageAttr)
	{
		LeModDamageAttr &e=(LeModDamageAttr &)e0;	
		if (e.osbSrc)
		{
			CLevelSkill *skill=e.osbSrc->GetSkill();
			if (skill)
			{
				LevelRecordSkill *recSkill=skill->GetRec();
				RecordID idSkill=recSkill->GetID();

				if (upgradeInitial->ExistSkillID(idSkill))
				{
					if (e.modsAttack)
					{
						e.modsAttack->modsDamage[DamageAttrType_Pierce].atkRate+=upgradeInitial->ratePierce;
//						e.modsAttack->modsDamage[DamageAttrType_Bleed].atkAdd+=(short)FloatToNearestInt(upgradeInitial->addBleed);
					}
				}
			}
		}
	}

}


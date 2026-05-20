
#include "stdh.h"

#include "Level.h"
#include "LevelRtnus.h"

#include "LevelEvents.h"

#include "LevelOSB.h"

#include "LoUnit.h"

#include "Buff_FlyBirth.h"

#include "Ability_SkullSword.h"

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
//CUpgradeSkullSword_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeSkullSword_Init);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeSkullSword_LevelUp);

BOOL CUpgradeSkullSword_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_SkullSword *ability=(CLevelAbility_SkullSword *)ability_;

	ability->_idSkill=idSkill;
	ability->_idDefSkill=idDefaultSkill;
	ability->_idSummon=idSummon;

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_SkullSword

void CLevelAbility_SkullSword::_InitTechs()
{
}

void CLevelAbility_SkullSword::_OnBuildRT()
{
	_BuildGradeRT();
	_AddSkillRT(LevelAbilityAction_AttackA,_idDefSkill);
	_AddSkillRT(LevelAbilityAction_FuryA,_idSkill);
}

void CLevelAbility_SkullSword::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_SkullSword::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);
}

void CLevelAbility_SkullSword::_LoadSync(CDataPacket &dp,CRecords *records)
{
	_LoadSync_SkillsRT(dp,records);
}


void CLevelAbility_SkullSword::_OnUpdate(LevelTick dt0)
{
}

void CLevelAbility_SkullSword::_OnEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_Kill)
	{
		LeKill &e=(LeKill &)e0;
		if (e.osbSrc)
		{
			CLevelSkill *skill=e.osbSrc->GetSkill();
			if (skill)
			{
				LevelRecordSkill *recSkill=skill->GetRec();
				RecordID idSkill=recSkill->GetID();

				if (idSkill==_idSkill)
				{
					if (e.loTarget)
					{
						CLevel *level=e.loTarget->GetLevel();
						if (level)
						{
							if (_idSummon!=RecordID_Invalid)
							{
								LevelPos3D pos=e.loTarget->GetFramePos3D();

								CLoUnit* lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));

								lo->PostCreate(LevelPlayerID_Wild,NULL,_idSummon,1,NULL,EquipSetPick_None,pos);//使用技能的等级
								level->AddToActives(lo);

								extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
								CLevelPlayer *player=LevelUtil_PlayerFromLo(_owner);
								if (player)
								{
									if (player->GetRtnus())
										player->GetRtnus()->Add_New(lo,FALSE);
								}

								CUpgradeSkullSword_Init *upgrade=_GetInitialUpgrade<CUpgradeSkullSword_Init>();
								if (upgrade)
								{
									if (upgrade->idBirth)
									{
										if (e.strike)
										{
											BuffArg_FlyBirth arg;
											arg.posInit=pos;
											arg.dir=e.strike->GetDir();

											level->GetDecider()->MakeBuff(*e.osbSrc,lo,upgrade->idBirth,ANIMTICK_FROM_SECOND(1.0f),&arg,e.link);
										}
									}
								}

								SAFE_RELEASE(lo);
							}
						}
					}
				}
			}
		}
	}
}


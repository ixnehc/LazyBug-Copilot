
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LevelEvents.h"

#include "LevelOSB.h"

#include "LoUnit.h"

#include "Ability_ObliterateBow.h"
#include "Ability_DeathCall.h"
#include "LevelObliterateArg.h"
 
#include "LevelRecords.h" 
#include "LevelRecordEO.h"

#include "EoBullet.h"

#include "Deal_CreateEo.h"
#include "Skill_Zeal.h"


/*
BloodTeeth:
命中敌人后,向周围敌人射出骨牙箭
DeathCall:
有几率射出死亡之箭,可以一击必杀
FlameBlade:

FlashSwing:

HonorSword:

LightningBow:

ObliterateBow:

PhantomDagger:

SkullSword:
杀死的敌人的尸体上出现一个骷髅,玩家走近它,会触发一次范围伤害
TeleportSword:

*/

 
//////////////////////////////////////////////////////////////////////////
//CUpgradeObliterateBow_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeObliterateBow_Init);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeObliterateBow_LevelUp);

BOOL CUpgradeObliterateBow_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_ObliterateBow *ability=(CLevelAbility_ObliterateBow *)ability_;

	ability->_idSkill=idSkill;
	ability->_idDefSkill=idDefaultSkill;

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_ObliterateBow

static void SetZealParam(LevelRecordSkill *recSkill,int count,AnimTick dur)
{
	if (recSkill)
	{
		SkillParam_Zeal *param=recSkill->GetParam<SkillParam_Zeal>();
		if (param)
		{
			SkillGradeInfo_Zeal *info=param->GetGrdInfo(1);
			if (info)
			{
				info->count=count;
				info->dur=dur;
			}
		}
	}
}

static BOOL GetZealParam(LevelRecordSkill *recSkill,int &count,AnimTick &dur)
{
	if (recSkill)
	{
		SkillParam_Zeal *param=recSkill->GetParam<SkillParam_Zeal>();
		if (param)
		{
			SkillGradeInfo_Zeal *info=param->GetGrdInfo(1);
			if (info)
			{
				count=info->count;
				dur=info->dur;

				return TRUE;
			}
		}
	}
	return FALSE;
}



void CLevelAbility_ObliterateBow::_InitTechs()
{
}

void CLevelAbility_ObliterateBow::_OnBuildRT()
{
	_BuildGradeRT();

	if (!_BuildSkillRT_FlashSwing())
		_BuildSkillRT_Default();
}

void CLevelAbility_ObliterateBow::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_ObliterateBow::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);

	if (TRUE)
	{
		int count;
		AnimTick dur;
		if (GetZealParam(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA),count,dur))
		{
			dp.Data_NextByte()=1;
			dp.Data_WriteSimple(count);
			dp.Data_WriteSimple(dur);
		}
		else
			dp.Data_NextByte()=0;
	}
}

void CLevelAbility_ObliterateBow::_LoadSync(CDataPacket &dp,CRecords *records)
{
	_LoadSync_SkillsRT(dp,records);

	if (dp.Data_NextByte()==1)
	{
		int count;
		AnimTick dur;
		dp.Data_ReadSimple(count);
		dp.Data_ReadSimple(dur);

		assert (_skillsRT.GetSkillRecord(LevelAbilityAction_MissileA));
		assert (_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA));
		SetZealParam(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA),count,dur);
		SetZealParam(_skillsRT.GetSkillRecord(LevelAbilityAction_MissileA),count,dur);
	}

}


void CLevelAbility_ObliterateBow::_OnUpdate(LevelTick dt0)
{
}

void CLevelAbility_ObliterateBow::_ApplyBulletDamageRate(LevelRecordSkill *recSkill,float rate)
{
	if (recSkill)
	{
		Deal_CreateEo *deal=recSkill->GetDeal<Deal_CreateEo>();
		if (deal)
		{
			LevelRecordEo *rec=deal->BeginModify(_level);
			if (rec)
			{
				Deal_Dmg *dealDmg=rec->GetDeal<Deal_Dmg>();
				if (dealDmg)
				{
					dealDmg->_attacks.pierce.Scale(rate);
					dealDmg->_attacks.crush.Scale(rate);
				}

				deal->EndModify();
			}
		}
	}
}


BOOL CLevelAbility_ObliterateBow::_BuildSkillRT_Default()
{
	CUpgradeObliterateBow_Init *upgradeInitial=(CUpgradeObliterateBow_Init *)_upgradeInitial;
	_AddSkillRT(LevelAbilityAction_MissileA,_idDefSkill);
	_AddSkillRT(LevelAbilityAction_FuryA,_idSkill);

	_ApplyBulletAttack(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA),upgradeInitial->attackNormal);
	_ApplyBulletAttack(_skillsRT.GetSkillRecord(LevelAbilityAction_MissileA),upgradeInitial->attackNormal);
	_ApplyBulletCount(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA));
	_ApplyBulletCount(_skillsRT.GetSkillRecord(LevelAbilityAction_MissileA));

	return TRUE;
}

BOOL CLevelAbility_ObliterateBow::_BuildSkillRT_FlashSwing()
{
	CUpgradeObliterateBow_Init *upgradeInitial=(CUpgradeObliterateBow_Init *)_upgradeInitial;
	ObliterateBowIdc_FlashSwing &idc=_upgradeInitial->idcFlashSwing;
	WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
	if (infoIdc.grdInduction>0)
	{
		if (infoIdc.inactive==LevelAbilityType_FlashSwing)
		{
			assert(idc.grdinfosIdc.size()>=infoIdc.grdInduction);

			_AddSkillRT(LevelAbilityAction_MissileA,_idDefSkill);
			_AddSkillRT(LevelAbilityAction_FuryA,_idSkill);

			_ApplyBulletAttack(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA),upgradeInitial->attackNormal);
			_ApplyBulletAttack(_skillsRT.GetSkillRecord(LevelAbilityAction_MissileA),upgradeInitial->attackNormal);
			_ApplyBulletCount(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA));
			_ApplyBulletCount(_skillsRT.GetSkillRecord(LevelAbilityAction_MissileA));

			int count=idc.grdinfosIdc[infoIdc.grdInduction-1].count;
			AnimTick dur=ANIMTICK_FROM_SECOND(idc.grdinfosIdc[infoIdc.grdInduction-1].dur);
			SetZealParam(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA),count,dur);
			SetZealParam(_skillsRT.GetSkillRecord(LevelAbilityAction_MissileA),count,dur);

			float rateBase=idc.grdinfosIdc[infoIdc.grdInduction-1].rateBaseDmg;//不同的感应等级有不同的基础伤害比率
			float rate=_CalcUpgradedValue(rateBase,idc.ratePerGrade,infoIdc.grdInactive);//根据FlashSwing等级提升这个伤害比率

			_ApplyBulletDamageRate(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA),rate);
			_ApplyBulletDamageRate(_skillsRT.GetSkillRecord(LevelAbilityAction_MissileA),rate);

			return TRUE;
		}
	}

	return FALSE;

}


BOOL CLevelAbility_ObliterateBow::_MakeDmg_BloodTeeth(LeDamage &e)
{
	ObliterateBowIdc_BloodTeeth &idc=_upgradeInitial->idcBloodTeeth;
	assert(idc.idEo!=RecordID_Invalid);
	WpnIdcInfo &infoIdc=_GetWpnIdcInfo();

	LevelOSB &osbSrc=*e.osbSrc;

	if ((infoIdc.grdInduction>0)&&(infoIdc.inactive==LevelAbilityType_BloodTeeth))
	{
		if (e.loTarget)
		{
			LevelPos3D pos=e.loTarget->GetFramePos3D();
			pos.y+=e.loTarget->GetAimHeight();

			int nBullets=FloatToNearestInt(_CalcUpgradedValue(idc.countBase,idc.countPerGrade,infoIdc.grdInactive));

			LevelPos dirs[32];//Big enough
			if (nBullets>ARRAY_SIZE(dirs))
				nBullets=ARRAY_SIZE(dirs);

			extern void ScatterBulletDirs_Uniform(LevelPos &dir,LevelPos *dirs,DWORD c,float fov);
			ScatterBulletDirs_Uniform(e.strike->GetDir(),dirs,nBullets,270.0f*i_math::GRAD_PI2);

			for (int i=0;i<nBullets;i++)
			{
				LevelRecordEo *rec=_level->GetRecords()->GetEo(idc.idEo);
				CLoEffectObj *eo=NULL;
				if (rec)
				{
					if (TRUE)
					{
						rec=(LevelRecordEo*)rec->Clone();
						if (TRUE)
						{
							DealEntry *entry=rec->GetDealEntry<Deal_Dmg>();

							float dmg=_CalcUpgradedValue(idc.dmgBase,idc.dmgPerGrade,infoIdc.grdInactive);
							Deal_Dmg *deal=(Deal_Dmg *)entry->deal;
							if (deal)
								deal->_attacks.pierce.lo=deal->_attacks.pierce.hi=(WORD)FloatToNearestInt(dmg);
						}
						if (TRUE)
						{
							EoBullet *param=rec->GetParam<EoBullet>();
							if (param)
							{
							}
						}
					}

					eo=(CLoEffectObj*)_level->CreateObj(rec->param->GetEoClass());
					if (eo)
					{
						eo->SetHost(e.loTarget->GetID());
						i_math::vector3df dir;
						dir.setXZ(dirs[i]);
						eo->PostCreate(_owner->GetPlayerID(),rec,pos,dir,1,osbSrc,e.link);
						_level->AddToActives(eo);
					}
				}
				SAFE_RELEASE(rec);
				SAFE_RELEASE(eo);
			}
		}
	}
	return TRUE;
}

void CLevelAbility_ObliterateBow::_MakeDmg(LeDamage &e)
{
	_MakeDmg_BloodTeeth(e);
}

BOOL CLevelAbility_ObliterateBow::_PreCreateBullet_SacredArrow(LePreCreateEo &e)
{
	CUpgradeObliterateBow_Init *upgradeInitial=(CUpgradeObliterateBow_Init *)_upgradeInitial;

	if (LevelUtil_CheckAbilityToggledOn(_owner,LevelAbilityType_SacredArrow))
	{
		if (e.argDeal)
		{
			if (e.argDeal->link.iSerial==0)//第一箭
			{
				Deal_CreateEo *deal=(Deal_CreateEo *)upgradeInitial->infoSacredArrow.deal;
				_ApplyBulletAttack(deal,upgradeInitial->infoSacredArrow.attack);
				e.eoCreated=deal->CreateEo(*e.osbSrc,e.pos,*e.argDeal,LevelObjID_Invalid);

				LevelUtil_HandleSacredArrowFired(_owner);

				return TRUE;
			}
		}
	}
	return FALSE;
}


BOOL CLevelAbility_ObliterateBow::_PreCreateBullet_DeathCall(LePreCreateEo &e)
{
	ObliterateBowIdc_DeathCall &idc=_upgradeInitial->idcDeathCall;
	WpnIdcInfo &infoIdc=_GetWpnIdcInfo();

	LevelOSB &osbSrc=*e.osbSrc;
	assert(e.argDeal);

	if ((infoIdc.grdInduction>0)&&(infoIdc.inactive==LevelAbilityType_DeathCall))
	{
		float rate=_CalcUpgradedValue(idc.rateBase,idc.ratePerGrade,infoIdc.grdInactive);
		if (CSysRandom::Roll(rate))
		{
			Deal_CreateEo *deal=(Deal_CreateEo *)idc.deal;
			_ApplyBulletAttack(deal,idc.attack);
			e.eoCreated=deal->CreateEo(osbSrc,e.pos,*e.argDeal,LevelObjID_Invalid);
		}
	}

	return TRUE;
}

BOOL CLevelAbility_ObliterateBow::_MakeModDmg_Default(LeModDamageAttr &e)
{
	CUpgradeObliterateBow_Init *upgradeInitial=(CUpgradeObliterateBow_Init *)_upgradeInitial;
	e.bAttackMods=TRUE;
	_ApplyAttackMods(*e.modsAttack,upgradeInitial->attackNormal);

	return TRUE;
}

BOOL CLevelAbility_ObliterateBow::_MakeModDmg_DeathCall(LeModDamageAttr &e)
{
	CUpgradeObliterateBow_Init *upgradeInitial=(CUpgradeObliterateBow_Init *)_upgradeInitial;
	ObliterateBowIdc_DeathCall &idc=upgradeInitial->idcDeathCall;

	CLevelObj *loOwner=e.osbSrc->GetOwner();
	if (loOwner->GetType()==LevelObjType_Eo)
	{
		LevelRecordEo *rec=((CLoEffectObj*)loOwner)->GetRec();
		if (rec)
		{
			Deal_CreateEo *deal=(Deal_CreateEo *)idc.deal;

			if (rec->GetID()==deal->_idEo)
			{
				e.bAttackMods=TRUE;
				_ApplyAttackMods(*e.modsAttack,idc.attack);
				return TRUE;
			}
		}
	}

	return FALSE;

}



BOOL CLevelAbility_ObliterateBow::_PreCreateBullet(LePreCreateEo &e)
{
	if (!_PreCreateBullet_SacredArrow(e))
		_PreCreateBullet_DeathCall(e);

	return TRUE;
}

void CLevelAbility_ObliterateBow::_MakeModDmg(LeModDamageAttr &e)
{
	if (!_MakeModDmg_DeathCall(e))
		_MakeModDmg_Default(e);
}



BOOL CLevelAbility_ObliterateBow::_CheckArrow(CLevelObj *lo)
{
	if (lo)
	{
		BOOL bObliterateBullet=FALSE;
		if (lo->GetType()==LevelObjType_Eo)
		{
			CLoEffectObj *eo=(CLoEffectObj *)lo;
			LevelRecordEo *rec=eo->GetRec();
			if (rec)
			{
				for (int i=0;i<_upgradeInitial->idsBullet.size();i++)
				{
					if (rec->GetID()==_upgradeInitial->idsBullet[i])
						return TRUE;
				}
			}
		}
	}
	return FALSE;
}



void CLevelAbility_ObliterateBow::_OnEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_PreCreateEo)
	{
		LePreCreateEo &e=(LePreCreateEo &)e0;
		if (e.osbSrc)
		{
			CLevelSkill *skill=e.osbSrc->GetSkill();
			if (skill)
			{
				LevelRecordSkill *recSkill=skill->GetRec();
				RecordID idSkill=recSkill->GetID();

				if ((idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_FuryA))||
					(idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_MissileA)))
				{
					_PreCreateBullet(e);
				}
			}
		}
	}

	if (e0.GetType()==LET_PreKill)
	{
		LePreKill &e=(LePreKill &)e0;
		if (e.osbSrc)
		{
			CLevelObj *loOwner=e.osbSrc->GetOwner();
			if (_CheckArrow(loOwner))
			{
				if (e.argObliterate)
				{
					if (e.loTarget)
					{
						LevelAttr_Base *attrBase=e.loTarget->GetAttr_Base();
						if (attrBase)
						{
							int hpMax=attrBase->hp.GetMax_Int();
							e.argObliterate->tp=LevelObliterate_Blood;

							Damage &dmgPierce=e.argObliterate->dmgs.damages[DamageAttrType_Pierce];
							Damage &dmgCrush=e.argObliterate->dmgs.damages[DamageAttrType_Crush];
							dmgPierce.lo=dmgPierce.hi=(WORD)hpMax*1/10;
							dmgCrush.lo=dmgCrush.hi=(WORD)hpMax*4/10;
						}
					}
				}
			}
		}
	}

	if (e0.GetType()==LET_Damage)
	{
		LeDamage &e=(LeDamage &)e0;	
		if (e.osbSrc)
		{
			CLevelObj *loOwner=e.osbSrc->GetOwner();
			if (_CheckArrow(loOwner))
			{
				_MakeDmg(e);
			}
		}
	}

	if (e0.GetType()==LET_ModDamageAttr)
	{
		LeModDamageAttr &e=(LeModDamageAttr &)e0;	
		if (e.osbSrc)
		{
			CLevelObj *loOwner=e.osbSrc->GetOwner();
			if (_CheckArrow(loOwner))
			{
				_MakeModDmg(e);
			}
		}
	}



}

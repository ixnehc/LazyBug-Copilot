
#include "stdh.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

#include "LevelUtil.h"

#include "Ability_LightningBow.h"

#include "LevelRecords.h"
#include "LevelRecordEO.h"

#include "EoBullet.h"

#include "Deal_CreateEo.h"

/*
BloodTeeth:
命中敌人后,向周围敌人射出骨牙箭
DeathCall:
命中敌人后,有机会产生闪电爆炸
FlameBlade:
在敌人身上的闪电延长时间
FlashSwing:
额外射出[1]支箭
HonorSword:
Honor加伤害
LightningBow:
n/a
ObliterateBow:
右键按住不放,可以抽取周围[5]米内敌人的血,来累积能量,释放时发射出火红色闪电箭,附加额外的类似尸体爆炸属性的伤害
PhantomDagger:

SkullSword:
有[50%]机会射出额外的跟踪箭
TeleportSword:
有[50%]几率使受到伤害的单位眩晕

*/

 
//////////////////////////////////////////////////////////////////////////
//CUpgradeLightningBow_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeLightningBow_Init);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeLightningBow_LevelUp);


BOOL CUpgradeLightningBow_Init::Init(CLevelAbility *ability)
{
	CLevelAbility_LightningBow *abilityFire=(CLevelAbility_LightningBow *)ability;

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_LightningBow

void CLevelAbility_LightningBow::_InitTechs()
{
}

void CLevelAbility_LightningBow::_OnBuildRT()
{
	CUpgradeLightningBow_Init *upgradeInitial=(CUpgradeLightningBow_Init *)_upgradeInitial;

	_BuildGradeRT();

	_BuildSkillRT(upgradeInitial->settings);

	_ApplyBulletAttack(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA),upgradeInitial->attack);
	_ApplyBulletAttack(_skillsRT.GetSkillRecord(LevelAbilityAction_MissileA),upgradeInitial->attack);
	_ApplyBulletCount(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA));
	_ApplyBulletCount(_skillsRT.GetSkillRecord(LevelAbilityAction_MissileA));

}

void CLevelAbility_LightningBow::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}


void CLevelAbility_LightningBow::_OnUpdate(LevelTick dt0)
{
}

void CLevelAbility_LightningBow::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);
}

void CLevelAbility_LightningBow::_LoadSync(CDataPacket &dp,CRecords *records)
{
	_LoadSync_SkillsRT(dp,records);
}



BOOL CLevelAbility_LightningBow::_MakeCast_BloodTeeth(LePostCreateEo &e)
{
	LightningBowIdc_BloodTeeth &idc=_upgradeInitial->idcBloodTeeth;
	assert(idc.dealShoot);
	WpnIdcInfo &infoIdc=_GetWpnIdcInfo();

	LevelOSB &osbSrc=*e.osbSrc;

	CLevelSkill *skill=osbSrc.GetSkill();
	if (skill)
	{
		if ((infoIdc.grdInduction>0)&&(infoIdc.inactive==LevelAbilityType_BloodTeeth))
		{
			int nBullets=FloatToNearestInt(_CalcUpgradedValue(idc.countBase,idc.countPerGrade,infoIdc.grdInactive));

			DWORD c=0;
			CLevelObj **buf=e.eo->DetectRange(_owner->GetFramePos(),15.0f,c);

			LevelPos dir=e.eo->GetInitialDir3D().getXZ();
			LevelFace face0=LevelFaceFromDir(dir);
			for (int i=0;i<nBullets;i++)
			{
				LevelFace face=face0;
				LevelObjID idHost=LevelObjID_Invalid;
				if (c>0)
				{
					CLevelObj *loTarget=buf[CSysRandom::RandRangeInt(0,(int)c)];
					if (loTarget)
					{
						idHost=loTarget->GetID();
						LevelPos dir2=loTarget->GetFramePos()-e.eo->GetInitialPos3D().getXZ();
						face=LevelFaceFromDir(dir2);
					}
				}

				LevelPos3D dir3D;
				dir3D.setXZ(LevelFaceToDir(face+CSysRandom::RandRange(-i_math::Pi/2.0f,i_math::Pi/2.0f)));

				DealArg arg;
				arg.link=e.link;
				arg.dir=dir3D;

				CLoEffectObj *eo=((Deal_CreateEo *)idc.dealShoot)->CreateEo(osbSrc,e.eo->GetInitialPos3D(),arg,idHost);
				SAFE_RELEASE(eo);
			}
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CLevelAbility_LightningBow::_MakeModDmg_Default(LeModDamageAttr &e)
{
	CUpgradeLightningBow_Init *upgradeInitial=(CUpgradeLightningBow_Init *)_upgradeInitial;
	e.bAttackMods=TRUE;
	_ApplyAttackMods(*e.modsAttack,upgradeInitial->attack);

	return TRUE;
}

BOOL CLevelAbility_LightningBow::_PreCreateBullet_SacredArrow(LePreCreateEo &e)
{
	CUpgradeLightningBow_Init *upgradeInitial=(CUpgradeLightningBow_Init *)_upgradeInitial;

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



void CLevelAbility_LightningBow::_MakeCast(LePostCreateEo &e)
{
	_MakeCast_BloodTeeth(e);
}


void CLevelAbility_LightningBow::_MakeDmg(LeDamage &e)
{

}

void CLevelAbility_LightningBow::_MakeModDmg(LeModDamageAttr &e)
{
	_MakeModDmg_Default(e);
}

BOOL CLevelAbility_LightningBow::_PreCreateBullet(LePreCreateEo &e)
{
	_PreCreateBullet_SacredArrow(e);
	return FALSE;
}



BOOL CLevelAbility_LightningBow::_CheckArrow(CLevelObj *lo)
{
	if (lo)
	{
		BOOL bArrow=FALSE;
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



void CLevelAbility_LightningBow::_OnEvent(LevelEvent &e0)
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

				if ((idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_MissileA))||
					(idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_FuryA)))
				{
					_PreCreateBullet(e);
				}
			}
		}
	}

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

				if ((idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_MissileA))||
					(idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_FuryA)))
				{
					_MakeCast(e);
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
 
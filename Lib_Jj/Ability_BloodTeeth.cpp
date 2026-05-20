
#include "stdh.h"

#include "Level.h"

#include "LevelEvents.h"
#include "LevelRecords.h"

#include "LevelOSB.h"

#include "LoUnit.h"
#include "EoBullet.h"

#include "LevelRecordBuff.h"

#include "Buff_FlyBirth.h"

#include "Deal_CreateEo.h"

#include "Ability_BloodTeeth.h"

#include "LevelObjMiscFlags.h"

/*
Induction:

*V* BloodTeeth:
n/a
*V* DeathCall:
命中后,有一定几率产生双倍血液吸收,有很小几率产生加最大生命值的血球
*V* FlameBlade:
命中后,有几率产生跟踪火球飞向敌人,并且攻击可以累加FlameBlade的Fury
*V* FlashSwing:
发射地面刀光,高级下可以双挥
HonorSword:
命中后,有一定几率产生五彩球,增加Honor
*V* LightnignBow:
命中后,有几率产生跟踪闪电球飞向敌人,高级状态下闪电球会施放闪电链
*V* ObliterateBow:
杀死敌人后,产生根据敌人最大HP决定的回复最大体力值的血球
PhantomDagger:
命中后,有一定几率产生恢复体力的血球
SkullSword:
命中后有几率召唤出一个吸血骷髅,在外面持续飞行一段时间,吸收地方的血液,最后攻击敌人
*V* TeleportSword:
有几率产生一个震荡波,击退敌人,高级模式下,会扣血



*/

 
//////////////////////////////////////////////////////////////////////////
//CUpgradeBloodTeeth_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeBloodTeeth_Init);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeBloodTeeth_LevelUp);


BOOL CUpgradeBloodTeeth_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_BloodTeeth *ability=(CLevelAbility_BloodTeeth *)ability_;


	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//Deal_BloodTeethFireBlood
BIND_DEAL(Deal_BloodTeethFireBlood);

void Deal_BloodTeethFireBlood::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg)
{
	CLevelObj *owner=osbSrc.GetOwner();
	if (owner)
	{
		if(idBuff!=RecordID_Invalid)
		{
			LevelAttr_Base *base=owner->GetAttr_Base();
			if (base)
			{
				float dur=base->hp.GetCur_Float();

				if (owner->GetLevel())
				{
					LevelRecordBuff *recBuff=owner->GetLevel()->GetRecords()->GetBuff(idBuff);
					if (recBuff)
					{
						recBuff=(LevelRecordBuff *)recBuff->Clone();
						Deal_Dmg *deal=recBuff->GetDeal<Deal_Dmg>();
						if (deal)
						{
							float rate=1.0f;
							if (base->grade==2)
								rate=1.4f;
							if (base->grade>=3)
								rate=2.5f;
							LevelAttr_Damages *attacks=deal->_attacks.Get();
							attacks->damages[DamageAttrType_Fire].lo=(short)(rate*(float)attacks->damages[DamageAttrType_Fire].lo);
							attacks->damages[DamageAttrType_Fire].hi=(short)(rate*(float)attacks->damages[DamageAttrType_Fire].hi);
						}
						owner->GetLevel()->GetDecider()->MakeBuff(osbSrc,loTarget,recBuff,ANIMTICK_FROM_SECOND(dur),NULL,arg.link);

						SAFE_RELEASE(recBuff);
					}
				}

			}
		}
	}

}


//////////////////////////////////////////////////////////////////////////
//Deal_BloodTeethLightningBlood
BIND_DEAL(Deal_BloodTeethLightningBlood);

void Deal_BloodTeethLightningBlood::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg)
{
	CLevelObj *owner=osbSrc.GetOwner();
	if (owner)
	{
		LevelAttr_Base *base=owner->GetAttr_Base();
		if (base)
		{
			float dmg=base->hp.GetCur_Float();

			float rate=1.0f;
			if (base->grade==2)
				rate=1.4f;
			if (base->grade>=3)
				rate=2.5f;

			dmg*=rate;

			CLevel *level=osbSrc.GetLevel();
			if (level)
			{
				LevelStrike strike;
				strike.idSrc=osbSrc.GetOwnerID();
				strike.SetDir(arg.dir.getXZ());
				strike.SetStr(0.0f);

				LevelAttr_Damages attrDmg;
				attrDmg.damages[DamageAttrType_Lightning].lo=
					attrDmg.damages[DamageAttrType_Lightning].hi=(WORD)dmg;

				level->GetDecider()->MakeDamage(osbSrc,loTarget,strike,&attrDmg,arg.link,DmgBlockType_NotBlockable,FALSE);
			}
		}
	}
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_BloodTeeth

void CLevelAbility_BloodTeeth::_InitTechs()
{
}

BOOL CLevelAbility_BloodTeeth::_BuildSkillRT_FlashSwing()
{
	WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
	if (infoIdc.grdInduction>0)
	{
		if (infoIdc.inactive==LevelAbilityType_FlashSwing)
		{
			assert(_upgradeInitial->idcFlashSwing.idSkill!=RecordID_Invalid);

			_AddSkillRT(LevelAbilityAction_AttackA,_upgradeInitial->idDefaultSkill);
			_AddSkillRT(LevelAbilityAction_FuryA,_upgradeInitial->idcFlashSwing.idSkill);

			if (_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA))
			{
				float range=infoIdc.grdInactive*_upgradeInitial->idcFlashSwing.rangePerGrade+_upgradeInitial->idcFlashSwing.rangeBase;
				int dmgMin=infoIdc.grdActive*_upgradeInitial->idcFlashSwing.dmgPerGrade+_upgradeInitial->idcFlashSwing.dmgBaseMin;
				int dmgMax=infoIdc.grdActive*_upgradeInitial->idcFlashSwing.dmgPerGrade+_upgradeInitial->idcFlashSwing.dmgBaseMax;

				_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA)->CastRange=range-0.5f;
				Deal_CreateEo *deal=_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA)->GetDeal<Deal_CreateEo>();
				if (deal)
				{
					LevelRecordEo *rec=deal->BeginModify(_level);
					if (rec)
					{
						EoParamBullet *param=rec->GetParam<EoParamBullet>();
						if (param)
							param->range=range;

						deal->MakeDelta();

						Deal_Dmg *dealDmg=rec->GetDeal<Deal_Dmg>();
						if (dealDmg)
						{
							dealDmg->_attacks.pierce.lo=dmgMin;
							dealDmg->_attacks.pierce.hi=dmgMax;
						}

						deal->EndModify();
					}
				}
			}

			return TRUE;
		}
	}

	return FALSE;

}


void CLevelAbility_BloodTeeth::_OnBuildRT()
{
	_BuildGradeRT();

	if (!_BuildSkillRT_FlashSwing())
	{
		_AddSkillRT(LevelAbilityAction_AttackA,_upgradeInitial->idDefaultSkill);
		_AddSkillRT(LevelAbilityAction_FuryA,_upgradeInitial->idSkill);

		_ApplyAttack(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA),_upgradeInitial->attackNormal);
		_ApplyAttack(_skillsRT.GetSkillRecord(LevelAbilityAction_AttackA),_upgradeInitial->attackNormal);
	}
}

void CLevelAbility_BloodTeeth::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_BloodTeeth::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);

	//一些额外的参数
	if (_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA))
	{
		dp.Data_NextByte()=1;
		dp.Data_WriteSimple(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA)->CastRange);
	}
	else
		dp.Data_NextByte()=0;

}

void CLevelAbility_BloodTeeth::_LoadSync(CDataPacket &dp,CRecords *recordsSkill)
{
	_LoadSync_SkillsRT(dp,recordsSkill);

	//一些额外的参数
	if (dp.Data_NextByte()==1)
	{
		assert (_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA));
		dp.Data_ReadSimple(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA)->CastRange);
	}
}


void CLevelAbility_BloodTeeth::_OnUpdate(LevelTick dt0)
{
}

int CLevelAbility_BloodTeeth::_RollCount(float rate)
{
	int c=(int)rate;
	if (CSysRandom::Roll(rate-(float)c))
		c++;

	return c;
}



void CLevelAbility_BloodTeeth::_MakeBloodBirth(LevelOSB &osbSrc,CLevelObj *lo,LevelPos3D &pos,LevelStrike *strike,LevelOpLink &link)
{
	if (_upgradeInitial->idBirth)
	{
		if (strike)
		{
			LevelPos dir=strike->GetDir();
			LevelFace face=LevelFaceFromDir(dir);
			face+=CSysRandom::RandRange(-45.0f*i_math::GRAD_PI2,45.0f*i_math::GRAD_PI2);
			dir=LevelFaceToDir(face);
			_level->GetDecider()->MakeFlyBirth(osbSrc,lo,_upgradeInitial->idBirth,pos,dir,link);
		}
	}
}


BOOL CLevelAbility_BloodTeeth::_MakeBlood_DeathCall(LevelOSB &osbSrc,CLevelObj *loTarget,int hp,LevelStrike *strike,LevelOpLink &link)
{
	assert(_upgradeInitial->idcDeathCall.idSummon!=RecordID_Invalid);

	if (loTarget)
	{
		WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
		BOOL bTriggered=FALSE;
		if (infoIdc.grdInduction>0)
		{
			if (infoIdc.inactive==LevelAbilityType_DeathCall)
			{
				float pssb=_upgradeInitial->idcDeathCall.possibilityBase;
				pssb+=_upgradeInitial->idcDeathCall.possibilityUpgrade*infoIdc.grdInactive;
				if (infoIdc.grdInduction==2)
					pssb*=_upgradeInitial->idcDeathCall.mulIdcGrd02;
				if (infoIdc.grdInduction==3)
					pssb*=_upgradeInitial->idcDeathCall.mulIdcGrd03;
				bTriggered=CSysRandom::Roll(pssb);
			}
		}

		if (bTriggered)
		{
			LevelPos3D pos=loTarget->GetFramePos3D();
			CLoUnit* lo=(CLoUnit*)_level->CreateObj(Class_Ptr2(CLoUnit));

			if (TRUE)
			{
				lo->PostCreate(_owner->GetPlayerID(),NULL,_upgradeInitial->idcDeathCall.idSummon,1,NULL,EquipSetPick_None,pos);
				_level->AddToActives(lo);
				LevelAttr_Base *attr=lo->GetAttr_Base();
				if (attr)
					attr->hp.SetCur_Int(1);
			}

			_MakeBloodBirth(osbSrc,lo,pos,strike,link);

			SAFE_RELEASE(lo);

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CLevelAbility_BloodTeeth::_MakeBlood_FlameBlade(LevelOSB &osbSrc,CLevelObj *loTarget,int hp,LevelStrike *strike,LevelOpLink &link)
{
	assert(_upgradeInitial->idcFlameBlade.idSummon!=RecordID_Invalid);

	if (loTarget)
	{
		WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
		int nTriggered=0;
		if (infoIdc.grdInduction>0)
		{
			if (infoIdc.inactive==LevelAbilityType_FlameBlade)
			{
				float rate=_upgradeInitial->idcFlameBlade.rateBase+_upgradeInitial->idcFlameBlade.ratePerGrade*(float)infoIdc.grdInactive;
				nTriggered=_RollCount(rate);
			}
		}

		for (int i=0;i<nTriggered;i++)
		{
			LevelPos3D pos=loTarget->GetFramePos3D();
			CLoUnit* lo=(CLoUnit*)_level->CreateObj(Class_Ptr2(CLoUnit));

			if (TRUE)
			{
				lo->PostCreate(_owner->GetPlayerID(),NULL,_upgradeInitial->idcFlameBlade.idSummon,infoIdc.grdInduction,NULL,EquipSetPick_None,pos);
				_level->AddToActives(lo);
				LevelAttr_Base *attr=lo->GetAttr_Base();
				if (attr)
				{
					//根据血球的hp,计算Burn的时间,再保存为血球的hp
					float dur=_upgradeInitial->idcFlameBlade.durBase;
					dur+=_upgradeInitial->idcFlameBlade.durPerHP*(float)hp;

					attr->hp.SetMax_Float(dur);
					attr->hp.SetCur_Float(dur);
				}
			}

			_MakeBloodBirth(osbSrc,lo,pos,strike,link);

			SAFE_RELEASE(lo);

		}
	}

	return FALSE;
}

BOOL CLevelAbility_BloodTeeth::_MakeBlood_LightningBow(LevelOSB &osbSrc,CLevelObj *loTarget,int hp,LevelStrike *strike,LevelOpLink &link)
{
	assert(_upgradeInitial->idcLightningBow.idSummon!=RecordID_Invalid);

	if (loTarget)
	{
		WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
		int nTriggered=0;
		if (infoIdc.grdInduction>0)
		{
			if (infoIdc.inactive==LevelAbilityType_LightningBow)
			{
				float rate=_upgradeInitial->idcLightningBow.rateBase+_upgradeInitial->idcLightningBow.ratePerGrade*(float)infoIdc.grdInactive;
				nTriggered=_RollCount(rate);
			}
		}

		for (int i=0;i<nTriggered;i++)
		{
			LevelPos3D pos=loTarget->GetFramePos3D();
			CLoUnit* lo=(CLoUnit*)_level->CreateObj(Class_Ptr2(CLoUnit));

			if (TRUE)
			{
				lo->PostCreate(_owner->GetPlayerID(),NULL,_upgradeInitial->idcLightningBow.idSummon,infoIdc.grdInduction,NULL,EquipSetPick_None,pos);
				_level->AddToActives(lo);
				LevelAttr_Base *attr=lo->GetAttr_Base();
				if (attr)
				{
					//根据血球的hp,计算伤害,再保存为血球的hp
					float dmg=_upgradeInitial->idcLightningBow.dmgBase;
					dmg+=_upgradeInitial->idcLightningBow.dmgPerHP*(float)hp;

					attr->hp.SetMax_Float(dmg);
					attr->hp.SetCur_Float(dmg);
				}
			}

			_MakeBloodBirth(osbSrc,lo,pos,strike,link);

			SAFE_RELEASE(lo);

		}
	}

	return FALSE;
}

BOOL CLevelAbility_BloodTeeth::_MakeBlood_ObliterateBow(LevelOSB &osbSrc,CLevelObj *loTarget,LevelStrike *strike,LevelOpLink &link)
{
	assert(_upgradeInitial->idcObliterateBow.idSummon!=RecordID_Invalid);

	if (loTarget)
	{
		LevelAttr_Base *attrTarget=loTarget->GetAttr_Base();
		if (attrTarget)
		{
			WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
			int nTriggered=0;
			if (infoIdc.grdInduction>0)
			{
				if (infoIdc.inactive==LevelAbilityType_ObliterateBow)
				{
					float rate=_upgradeInitial->idcObliterateBow.rateBase+_upgradeInitial->idcObliterateBow.ratePerGrade*(float)infoIdc.grdInactive;
					nTriggered=_RollCount(rate);
				}
			}

			for (int i=0;i<nTriggered;i++)
			{
				LevelPos3D pos=loTarget->GetFramePos3D();
				CLoUnit* lo=(CLoUnit*)_level->CreateObj(Class_Ptr2(CLoUnit));

				if (TRUE)
				{
					lo->PostCreate(_owner->GetPlayerID(),NULL,_upgradeInitial->idcObliterateBow.idSummon,infoIdc.grdInduction,NULL,EquipSetPick_None,pos);
					_level->AddToActives(lo);
					LevelAttr_Base *attr=lo->GetAttr_Base();
					if (attr)
					{
						//计算体力回复,保存为血球的hp
						float percent=_upgradeInitial->idcObliterateBow.percentBase;
						percent+=_upgradeInitial->idcObliterateBow.percentPerGrade*(float)infoIdc.grdInduction;

						float amount=attrTarget->hp.GetMax_Float();
						amount=amount*percent/100.0f;
						attr->hp.SetMax_Float(amount);
						attr->hp.SetCur_Float(amount);
					}
				}

				_MakeBloodBirth(osbSrc,lo,pos,strike,link);

				SAFE_RELEASE(lo);

			}
		}
	}

	return FALSE;
}


BOOL CLevelAbility_BloodTeeth::_MakeDmg_TeleportSword(LevelOSB &osbSrc,CLevelObj *loTarget,LevelStrike *strike,LevelOpLink &link)
{
	assert(_upgradeInitial->idcTeleportSword.idEo!=RecordID_Invalid);

	if (!osbSrc.GetSkill())
		return FALSE;//只有技能直接产生的伤害会触发震荡波

	if (loTarget)
	{
		LevelAttr_Base *attrTarget=loTarget->GetAttr_Base();
		if (attrTarget)
		{
			WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
			int bTriggered=0;
			if (infoIdc.grdInduction>0)
			{
				if (infoIdc.inactive==LevelAbilityType_TeleportSword)
				{
					float rate=_upgradeInitial->idcTeleportSword.rateBase+_upgradeInitial->idcTeleportSword.ratePerGrade*(float)infoIdc.grdInactive;
					bTriggered=CSysRandom::Roll(rate);
				}
			}

			if (bTriggered)
			{
				LevelPos3D pos=loTarget->GetFramePos3D();

				LevelRecordEo *rec=_level->GetRecords()->GetEo(_upgradeInitial->idcTeleportSword.idEo);
				CLoEffectObj *eo=NULL;
				if (rec)
				{
					if (TRUE)
					{
						rec=(LevelRecordEo*)rec->Clone();
						DealEntry *entry=rec->GetDealEntry<Deal_Dmg>();
						entry->chance=0.0f;
						if (infoIdc.grdInduction>=2)
							entry->chance=1.0f;

						float dmg=_upgradeInitial->idcTeleportSword.dmgBase+_upgradeInitial->idcTeleportSword.dmgPerGrade*(float)infoIdc.grdInactive;
						Deal_Dmg *deal=(Deal_Dmg *)entry->deal;
						if (deal)
							deal->_attacks.pierce.lo=deal->_attacks.pierce.hi=(WORD)FloatToNearestInt(dmg);
					}

					eo=(CLoEffectObj*)_level->CreateObj(rec->param->GetEoClass());
					if (eo)
					{
						i_math::vector3df dir;
						dir.setXZ(strike->GetDir());
						eo->SetHost(loTarget->GetID());
						eo->PostCreate(_owner->GetPlayerID(),rec,pos,dir,1,osbSrc,link);
						_level->AddToActives(eo);
					}
				}
				SAFE_RELEASE(rec);
				SAFE_RELEASE(eo);
			}
		}
	}

	return FALSE;
}



void CLevelAbility_BloodTeeth::_MakeBlood(LevelOSB &osbSrc,CLevelObj *loTarget,int hp,LevelStrike *strike,LevelOpLink &link)
{
	assert(_upgradeInitial->idSummon!=RecordID_Invalid);

	if (loTarget)
	{
		CLevel *level=loTarget->GetLevel();
		if (level)
		{
			LevelPos3D pos=loTarget->GetFramePos3D();
			CLoUnit* lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));

			if (TRUE)
			{
				lo->PostCreate(_owner->GetPlayerID(),NULL,_upgradeInitial->idSummon,1,NULL,EquipSetPick_None,pos);
				level->AddToActives(lo);
				LevelAttr_Base *attr=lo->GetAttr_Base();
				if (attr)
					attr->hp.SetCur_Int(hp);
			}

			_MakeBloodBirth(osbSrc,lo,pos,strike,link);

			SAFE_RELEASE(lo);
		}
	}
}


void CLevelAbility_BloodTeeth::_MakeDmgBlood(LevelOSB &osbSrc,CLevelObj *loTarget,int nDmg,LevelStrike *strike,LevelOpLink &link)
{
	CUpgradeBloodTeeth_Init*upgrade=_GetInitialUpgrade<CUpgradeBloodTeeth_Init>();
	if (!upgrade)
		return;

	int hpTotal=nDmg;

	int maxHP=200;
	int minHP=10;

	if (minHP<nDmg/6)//最多分成6份
		minHP=nDmg/6;

	if (minHP>maxHP/2)
		minHP=maxHP/2;


	while(hpTotal>0)
	{
		int hp=0;
		if (minHP>hpTotal-minHP)
			hp=hpTotal;
		else
		{
			if (hpTotal-minHP<maxHP)
				hp=CSysRandom::RandRangeInt(minHP,hpTotal-minHP);
			else
				hp=CSysRandom::RandRangeInt(minHP,maxHP);
		}
		hpTotal-=hp;

		if (!_MakeBlood_LightningBow(osbSrc,loTarget,hp,strike,link))
		if (!_MakeBlood_FlameBlade(osbSrc,loTarget,hp,strike,link))
		if (!_MakeBlood_DeathCall(osbSrc,loTarget,hp,strike,link))
			_MakeBlood(osbSrc,loTarget,hp,strike,link);
	}
}

BOOL CLevelAbility_BloodTeeth::_MakeModDmg_Default(BOOL bDefaultSkill,LeModDamageAttr &e)
{
	CUpgradeBloodTeeth_Init*upgradeInitial=_upgradeInitial;

	e.bAttackMods=TRUE;

	_ApplyAttackMods(*e.modsAttack,upgradeInitial->attackNormal);

	return TRUE;
}


void CLevelAbility_BloodTeeth::_MakeDmg(LevelOSB &osbSrc,CLevelObj *loTarget,int nDmg,LevelStrike *strike,LevelOpLink &link)
{
	_MakeDmg_TeleportSword(osbSrc,loTarget,strike,link);
}


void CLevelAbility_BloodTeeth::_MakeKillBlood(LevelOSB &osbSrc,CLevelObj *loTarget,LevelStrike *strike,LevelOpLink &link)
{
	_MakeBlood_ObliterateBow(osbSrc,loTarget,strike,link);
};

void CLevelAbility_BloodTeeth::_MakeModDmg(BOOL bDefaultSkill,LeModDamageAttr &e)
{
	_MakeModDmg_Default(bDefaultSkill,e);
}



void CLevelAbility_BloodTeeth::_OnEvent(LevelEvent &e0)
{
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

				if ((idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_FuryA))||
					(idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_AttackA)))
				{
					BOOL bDefaultSkill=idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_AttackA);
					_MakeModDmg(bDefaultSkill,e);
				}
			}
		}
	}

	if (e0.GetType()==LET_Damage)
	{
		LeDamage &e=(LeDamage &)e0;	
		if (e.osbSrc)
		{
			CLevelSkill *skill=e.osbSrc->GetRootSkill();
			if (skill)
			{
				LevelRecordSkill *recSkill=skill->GetRec();
				RecordID idSkill=recSkill->GetID();

				if ((idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_FuryA))||
					(idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_AttackA)))
				{
					LoMiscFlags *flags=e.loTarget->GetMiscFlags();
					if (flags)
					{
						if (flags->GetAllowBloodTeeth())
							_MakeDmgBlood(*e.osbSrc,e.loTarget,e.nDmg,e.strike,e.link);
					}

					_MakeDmg(*e.osbSrc,e.loTarget,e.nDmg,e.strike,e.link);
				}
			}
		}
	}

	if (e0.GetType()==LET_Kill)
	{
		LeKill &e=(LeKill &)e0;	
		if (e.osbSrc)
		{
			CLevelSkill *skill=e.osbSrc->GetRootSkill();
			if (skill)
			{
				LevelRecordSkill *recSkill=skill->GetRec();
				RecordID idSkill=recSkill->GetID();

				if ((idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_FuryA))||
					(idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_FuryA)))
				{
					LoMiscFlags *flags=e.loTarget->GetMiscFlags();
					if (flags)
					{
						if (flags->GetAllowBloodTeeth())
							_MakeKillBlood(*e.osbSrc,e.loTarget,e.strike,e.link);
					}
				}

			}
		}
	}

}

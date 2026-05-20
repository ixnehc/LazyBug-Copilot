
#include "stdh.h"

#include "LevelUtil.h"

#include "Ability_DeathCall.h"
#include "LevelOSB.h"
#include "LevelEvents.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelSensor.h"

#include "LoEffectObj.h"
#include "LoUnit.h"
#include "EoArea.h"
#include "EoChain.h"
 
#include "LevelRecordBuff.h"

#include "LevelAttrs.h"

#include "Buff_Dead.h"

#include "Deal_CureHP.h"
#include "Deal_CreateEo.h"
#include "Deal_Dizzy.h"



/*
*V* BloodTeeth:
每次必杀攻击,回满血
*V* DeathCall:
n/a
*V* FlameBlade:
每次必杀攻击,原地产生一个火焰堆
*V* FlashSwing:
每次必杀攻击,在指定地点生成一个持续刀光挥舞,有必杀效果
HonorSword:
Hornor提升致命一击的几率
*V* LightnignBow:
蓄能球持续向敌人发射闪电链攻击
*V* ObliterateBow:
杀死敌人后,产生一个飞箭球,向敌人射出[5]支跟踪箭后消失
PhantomDagger:
每次必杀攻击产生黑色的必杀幻影
SkullSword:
每次必杀产生一个必杀骷髅
*V* TeleportSword:
每次必杀产生一个震荡波,可以使敌人眩晕

*/



//////////////////////////////////////////////////////////////////////////
//CUpgradeDeathCall_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeDeathCall_Init);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeDeathCall_LevelUp);

BOOL CUpgradeDeathCall_Init::Init(CLevelAbility *ability)
{
	CLevelAbility_DeathCall *abilityFire=(CLevelAbility_DeathCall *)ability;

	return TRUE;
}




//////////////////////////////////////////////////////////////////////////
//CLevelAbility_DeathCall
void CLevelAbility_DeathCall::_OnBuildRT()
{
	CUpgradeDeathCall_Init *upgradeInitial=(CUpgradeDeathCall_Init *)_upgradeInitial;

	_BuildGradeRT();
	if (!_BuildSkillRT_FlashSwing())
	{
		if (_nStars>0)
		{
			_BuildSkillRT(upgradeInitial->settings);

			_ApplyAttack(_skillsRT.GetSkillRecord(LevelAbilityAction_AttackA),_upgradeInitial->attackNormal);
			_ApplyAttack(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA),_upgradeInitial->attackLethal);

		}
		else
		{
			_BuildSkillRT(upgradeInitial->settings);

			_ApplyAttack(_skillsRT.GetSkillRecord(LevelAbilityAction_AttackA),_upgradeInitial->attackNormal);
			_ApplyAttack(_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA),_upgradeInitial->attackNormal);
		}
	}
}

void CLevelAbility_DeathCall::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_DeathCall::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);

	dp.Data_NextByte()=_nStars;
}

void CLevelAbility_DeathCall::_LoadSync(CDataPacket &dp,CRecords *recordsSkill)
{
	_LoadSync_SkillsRT(dp,recordsSkill);

	_nStars=dp.Data_NextByte();
}

void CLevelAbility_DeathCall::_FlushFlashSwingEos()
{
	std::unordered_map<LevelObjID,AnimTick>::iterator it=_eosFlashSwing.begin();
	while(it!=_eosFlashSwing.end())
	{
		std::unordered_map<LevelObjID,AnimTick>::iterator itCur=it;
		it++;

		if ((*itCur).second+ANIMTICK_FROM_SECOND(0.2f)<_level->GetT_())
			_eosFlashSwing.erase(itCur);
	}
}


void CLevelAbility_DeathCall::_OnUpdate(LevelTick dt)
{
	_Update_LightningBow(dt);
	_Update_ObliterateBow(dt);

	_FlushFlashSwingEos();
}

BOOL CLevelAbility_DeathCall::_BuildSkillRT_FlashSwing()
{
	CUpgradeDeathCall_Init *upgradeInitial=(CUpgradeDeathCall_Init *)_upgradeInitial;

	WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
	if (infoIdc.grdInduction>0)
	{
		if (infoIdc.inactive==LevelAbilityType_FlashSwing)
		{
			assert(_upgradeInitial->idcFlashSwing.idSkill!=RecordID_Invalid);

			if (_nStars>0)
			{
				_BuildSkillRT(upgradeInitial->settings);

				if (_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA))
				{
					float dur=_CalcUpgradedValue(_upgradeInitial->idcFlashSwing.durBase,_upgradeInitial->idcFlashSwing.durPerGrade, infoIdc.grdInactive);
					float dmg=_CalcUpgradedValue(_upgradeInitial->idcFlashSwing.dmgBase,_upgradeInitial->idcFlashSwing.dmgPerGrade, infoIdc.grdInactive);

					Deal_CreateEo *deal=_skillsRT.GetSkillRecord(LevelAbilityAction_FuryA)->GetDeal<Deal_CreateEo>();
					if (deal)
					{
						LevelRecordEo *rec=deal->BeginModify(_level);
						if (rec)
						{
							EoParamArea *param=rec->GetParam<EoParamArea>();
							if (param)
								param->dur=ANIMTICK_FROM_SECOND(dur);

							Deal_Dmg *dealDmg=rec->GetDeal<Deal_Dmg>();
							if (dealDmg)
							{
								dealDmg->_attacks.pierce.lo=FloatToNearestInt(dmg);
								dealDmg->_attacks.pierce.hi=FloatToNearestInt(dmg);
							}

							deal->EndModify();
						}
					}
				}

				return TRUE;
			}
		}
	}

	return FALSE;

}

void CLevelAbility_DeathCall::_DecStar()
{
	if (_nStars>0)
	{
		_nStars--;
		_verCache.Zero();
	}

	_tLightningAccum=0;
	_nLightning=0;

}


BOOL CLevelAbility_DeathCall::_MakePostCreateEo_FlashSwing(LePostCreateEo &e)
{
	if (e.eo)
	{
		CLevelSkill *skill=e.eo->GetRootSkill();
		if (skill)
		{
			if (skill->GetRec()->GetID()==_upgradeInitial->idcFlashSwing.idSkill)
			{
				_DecStar();

				return TRUE;
			}
		}
	}
	return FALSE;
}


BOOL CLevelAbility_DeathCall::_MakePreDmg_FlashSwing(LePreDamage &e)
{
	if (e.osb)
	{
		CLevelObj *owner=e.osb->GetOwner();
		if (owner)
		{
			if (owner->GetType()==LevelObjType_Eo)
			{
				CLoEffectObj *eo=(CLoEffectObj *)owner;
				CLevelSkill *skill=eo->GetRootSkill();
				if (skill)
				{
					if (skill->GetRec()->GetID()==_upgradeInitial->idcFlashSwing.idSkill)
					{
						if (_eosFlashSwing.find(eo->GetID())!=_eosFlashSwing.end())
						{
							e.bAbandon=TRUE;
							return TRUE;
						}
						else
						{
							if (e.strike)
							{
// 								if (e.strike->maskDmg&(1<<DamageAttrType_Lethal))
// 								{
// 									//触发了Lethal,终止这个eo
// 									_eosFlashSwing[eo->GetID()]=_level->GetT_();
// 									eo->DeferDestroy();
// 
// 									return TRUE;
// 								}
							}
						}
					}
				}
			}
		}
	}

	return FALSE;
}


BOOL CLevelAbility_DeathCall::_MakeDmg_Default(BOOL bDefaultSkill,LevelOSB &osbSrc,CLevelObj *loTarget,int nDmg,LevelStrike *strike,LevelOpLink &link)
{
	if (osbSrc.GetSkill())
	{
		if (bDefaultSkill)
		{
			WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
			BOOL bAllowIncStar=FALSE;
			if (_nStars==0)
				bAllowIncStar=TRUE;
			else
			{
				if ((infoIdc.grdInduction>0)&&(infoIdc.inactive!=LevelAbilityType_None))
				{
					if (_nStars<=infoIdc.grdInduction)
						bAllowIncStar=TRUE;
				}
			}
			if (bAllowIncStar)
			{
				float rate=_CalcUpgradedValue(_upgradeInitial->rateStar,_grd);

				if (CSysRandom::Roll(rate))
				{
					_nStars++;
					_verCache.Zero();
				}
			}
		}
		else
		{
			_DecStar();
		}
	}
	return TRUE;
}

BOOL CLevelAbility_DeathCall::_MakeModDmg_Default(BOOL bDefaultSkill,LeModDamageAttr &e,LevelOSB &osbSrc,CLevelObj *loTarget)
{
	e.bAttackMods=TRUE;

	WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
	if (osbSrc.GetSkill())
	{
		if (!bDefaultSkill)
		{
			if (_nStars>0)
			{
				_ApplyAttackMods(*e.modsAttack,_upgradeInitial->attackLethal);

				return TRUE;
			}
		}

		_ApplyAttackMods(*e.modsAttack,_upgradeInitial->attackNormal);

	}
	return TRUE;
}

BOOL CLevelAbility_DeathCall::_MakeKill_BloodTeeth(BOOL bDefaultSkill,LeKill &e)
{
	assert(_upgradeInitial->idcBloodTeeth.idBuff!=RecordID_Invalid);

	LevelOSB &osbSrc=*e.osbSrc;

	WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
	if (!bDefaultSkill)
	{
		if (osbSrc.GetSkill())
		{
			if ((infoIdc.grdInduction>0)&&(infoIdc.inactive==LevelAbilityType_BloodTeeth))
			{
				if (e.loTarget)
				{
					LevelAttr_Base *attrBase=e.loTarget->GetAttr_Base();
					if (attrBase)
					{
						float percent=_upgradeInitial->idcBloodTeeth.percentBase;
						if (infoIdc.grdInactive>1)
							percent+=_upgradeInitial->idcBloodTeeth.percentPerGrade*(float)(infoIdc.grdInactive-1);

						LevelRecordBuff *rec=_level->GetRecords()->GetBuff(_upgradeInitial->idcBloodTeeth.idBuff);
						if (rec)
						{
							rec=(LevelRecordBuff *)rec->Clone();
							Deal_CureHP *deal=rec->GetDeal<Deal_CureHP>();
							deal->_nCure=FloatToNearestInt(percent*attrBase->hp.GetMax_Float());

							_level->GetDecider()->MakeBuff(*e.osbSrc,_owner,rec,0,NULL,e.link);
						}
						SAFE_RELEASE(rec);
					}
				}
				return TRUE;
			}
		}
	}
	return FALSE;

}

BOOL CLevelAbility_DeathCall::_MakeKill_FlameBlade(BOOL bDefaultSkill,LeKill &e)
{
	assert(_upgradeInitial->idcBloodTeeth.idBuff!=RecordID_Invalid);

	LevelOSB &osbSrc=*e.osbSrc;

	WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
	if ((infoIdc.grdInduction>0)&&(infoIdc.inactive==LevelAbilityType_FlameBlade))
	{
		if (!bDefaultSkill)
		{
			if (osbSrc.GetSkill())
			{
				if (e.loTarget)
				{
					LevelPos3D pos=e.loTarget->GetFramePos3D();

					LevelRecordEo *rec=_level->GetRecords()->GetEo(_upgradeInitial->idcFlameBlade.idEo);
					CLoEffectObj *eo=NULL;
					if (rec)
					{
						if (TRUE)
						{
							rec=(LevelRecordEo*)rec->Clone();
							if (TRUE)
							{
								DealEntry *entry=rec->GetDealEntry<Deal_Dmg>();

								float dmg=_CalcUpgradedValue(_upgradeInitial->idcFlameBlade.dmgBase,
													_upgradeInitial->idcFlameBlade.dmgPerGrade,infoIdc.grdInactive);
								Deal_Dmg *deal=(Deal_Dmg *)entry->deal;
								if (deal)
									deal->_attacks.fire.lo=deal->_attacks.fire.hi=(WORD)FloatToNearestInt(dmg);
							}
							if (TRUE)
							{
								EoParamArea *param=rec->GetParam<EoParamArea>();
								if (param)
								{
									float dur=_CalcUpgradedValue(_upgradeInitial->idcFlameBlade.durBase,
										_upgradeInitial->idcFlameBlade.durPerGrade,infoIdc.grdInactive);
									param->dur=ANIMTICK_FROM_SECOND(dur);
								}
							}
						}

						eo=(CLoEffectObj*)_level->CreateObj(rec->param->GetEoClass());
						if (eo)
						{
							i_math::vector3df dir;
							dir.setXZ(e.strike->GetDir());
							eo->PostCreate(_owner->GetPlayerID(),rec,pos,dir,1,osbSrc,e.link);
							_level->AddToActives(eo);
						}
					}
					SAFE_RELEASE(rec);
					SAFE_RELEASE(eo);
				}
			}
		}
	}
	return TRUE;
}


BOOL CLevelAbility_DeathCall::_Update_LightningBow(AnimTick dt)
{
	DeathCallIdc_LightningBow &idc=_upgradeInitial->idcLightningBow;
	WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
	if ((infoIdc.grdInduction>0)&&(infoIdc.inactive==LevelAbilityType_LightningBow)&&(infoIdc.active==LevelAbilityType_DeathCall))
	{
		if (_nStars>0)
		{
			const AnimTick durPerLightning=ANIMTICK_FROM_SECOND(0.5f);
			_tLightningAccum+=dt;
			int nLightning=_tLightningAccum/durPerLightning;
			if (nLightning>_nLightning)
			{

				Deal_CreateEo *deal=(Deal_CreateEo *)idc.deal;
				LevelRecordEo *rec=deal->BeginModify(_level);
				if (rec)
				{
					EoParamChain *param=rec->GetParam<EoParamChain>();

					float range=_CalcUpgradedValue(idc.rangeBase,idc.rangePerGrade,infoIdc.grdInactive);
					param->radiusAffect=param->radiusStep=range;

					Deal_Dmg *dealDmg=rec->GetDeal<Deal_Dmg>();
					if (dealDmg)
					{
						float dmg=_CalcUpgradedValue(idc.dmgBase,idc.dmgPerGrade,infoIdc.grdInactive);
						dealDmg->_attacks.lightning.lo=dealDmg->_attacks.lightning.hi=FloatToNearestInt(dmg);
					}

					deal->EndModify();
				}

				DealArg arg;
				deal->Make(LevelOSB(_owner),_owner,arg,NULL);

				_nLightning=nLightning;
			}

			AnimTick durPerStar=ANIMTICK_FROM_SECOND(_CalcUpgradedValue(idc.durBase,idc.durPerGrade,infoIdc.grdInactive));
			if (_tLightningAccum>durPerStar)
			{
				_tLightningAccum-=durPerStar;
				_nLightning=_tLightningAccum/durPerLightning;

				_DecStar();
			}
		}

		return TRUE;
	}

	return FALSE;

}

BOOL CLevelAbility_DeathCall::_Update_ObliterateBow(AnimTick dt)
{
	DeathCallIdc_ObliterateBow &idc=_upgradeInitial->idcObliterateBow;
	assert(idc.idDie!=RecordID_Invalid);

	AnimTick tCur=_level->GetT_();
	AnimTick durCD=ANIMTICK_FROM_SECOND(0.5f);
	AnimTick durDying=ANIMTICK_FROM_SECOND(1.0f);
	int i=0;
	while(i<_entriesObliterateBow.size())
	{
		ObliterateBowEoEntry &e=_entriesObliterateBow[i];

		CLoUnit *lo=(CLoUnit *)LevelUtil_GetAliveLo(_level,e.id);
		if (lo)
		{
			if (!((e.tRecentShot!=ANIMTICK_INFINITE)&&(tCur<e.tRecentShot+durCD)))
			{
				if (!LevelUtil_FindBuffByRecordID(lo,idc.idBirth))
				{
					if (e.nShots<e.nToShots)
					{
						CLevelSensor *sensor=lo->GetSensor();
						if (sensor)
						{
							CLevelObj *loThreat=sensor->GetThreat();
							if (loThreat)
							{
								Deal_CreateEo *deal=(Deal_CreateEo *)idc.dealShoot;
								if (deal)
								{
									LevelRecordEo *rec=deal->BeginModify(_level);
									if (rec)
									{
										Deal_Dmg *dealDmg=rec->GetDeal<Deal_Dmg>();
										if (dealDmg)
											dealDmg->_attacks.pierce.lo=dealDmg->_attacks.pierce.hi=e.nDmgPerShot;

										deal->EndModify();
									}

									DealArg arg;
									arg.dir=loThreat->GetFramePos3D()-lo->GetFramePos3D();
									arg.dir.normalize();
									arg.link.id=_level->GenOpLinkID();
									deal->Make(LevelOSB(lo),lo,arg,NULL);

									e.nShots++;
									e.tRecentShot=tCur;

									if (e.nShots>=e.nToShots)
									{
										BuffArg_Dead argDead;
										LevelBuffID idBuff=_level->GetDecider()->MakeBuff(LevelOSB(lo),lo,idc.idDie,ANIMTICK_INFINITE,&argDead,arg.link);
									}
								}
							}
						}
					}
					else
					{
						if (e.tRecentShot+durDying<tCur)
						{
							//死了一段时间了
							lo->DeferDestroy();
						}
					}
				}

			}

			i++;
			continue;
		}

		//清除这个entry
		_entriesObliterateBow[i]=_entriesObliterateBow[_entriesObliterateBow.size()-1];
		_entriesObliterateBow.pop_back();

	}

	return TRUE;
}


BOOL CLevelAbility_DeathCall::_MakeKill_ObliterateBow(BOOL bDefaultSkill,LeKill &e)
{
	DeathCallIdc_ObliterateBow &idc=_upgradeInitial->idcObliterateBow;
	assert(idc.idSummon!=RecordID_Invalid);
	assert(idc.idBirth!=RecordID_Invalid);

	LevelOSB &osbSrc=*e.osbSrc;

	WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
	if ((infoIdc.grdInduction>0)&&(infoIdc.inactive==LevelAbilityType_ObliterateBow))
	{
		if (!bDefaultSkill)
		{
			if (osbSrc.GetSkill())
			{
				if (e.loTarget)
				{
					LevelAttr_Base *attrTarget=e.loTarget->GetAttr_Base();
					if (attrTarget)
					{
						LevelPos3D pos=e.loTarget->GetFramePos3D();
						CLoUnit* lo=(CLoUnit*)_level->CreateObj(Class_Ptr2(CLoUnit));

						if (TRUE)
						{
							lo->PostCreate(_owner->GetPlayerID(),NULL,idc.idSummon,infoIdc.grdInduction,NULL,EquipSetPick_None,pos);
							_level->AddToActives(lo);
							LevelAttr_Base *attr=lo->GetAttr_Base();
							if (attr)
							{
								float amount=attrTarget->hp.GetMax_Float();
								attr->hp.SetMax_Float(amount);
								attr->hp.SetCur_Float(amount);
							}
						}

						if (idc.idBirth)
						{
							if (e.strike)
							{
								LevelPos dir=e.strike->GetDir();
								_level->GetDecider()->MakeFlyBirth(osbSrc,lo,idc.idBirth,pos,dir,e.link);
							}
						}

						ObliterateBowEoEntry e;
						e.id=lo->GetID();
						e.nToShots=5;
						e.nDmgPerShot=FloatToNearestInt(attrTarget->hp.GetMax_Float()/(float)e.nToShots);
						e.tRecentShot=ANIMTICK_INFINITE;
						_entriesObliterateBow.push_back(e);

						SAFE_RELEASE(lo);
					}
				}
			}
		}
		return TRUE;
	}

	return FALSE;
}


BOOL CLevelAbility_DeathCall::_MakeKill_TeleportSword(BOOL bDefaultSkill,LeKill &e)
{
	DeathCallIdc_TeleportSword &idc=_upgradeInitial->idcTeleportSword;
	assert(idc.idEo!=RecordID_Invalid);

	LevelOSB &osbSrc=*e.osbSrc;

	WpnIdcInfo &infoIdc=_GetWpnIdcInfo();
	if ((infoIdc.grdInduction>0)&&(infoIdc.inactive==LevelAbilityType_TeleportSword))
	{
		if (!bDefaultSkill)
		{
			if (osbSrc.GetSkill())
			{
				if (e.loTarget)
				{
					LevelPos3D pos=e.loTarget->GetFramePos3D();

					LevelRecordEo *rec=_level->GetRecords()->GetEo(idc.idEo);
					CLoEffectObj *eo=NULL;
					if (rec)
					{
						if (TRUE)
						{
							rec=(LevelRecordEo*)rec->Clone();
							float dmg=_CalcUpgradedValue(idc.dmgBase,idc.dmgPerGrade,infoIdc.grdInactive);
							if (TRUE)
							{
								DealEntry *entry=rec->GetDealEntry<Deal_Dmg>();
								if (entry)
								{
									Deal_Dmg *deal=(Deal_Dmg *)entry->deal;
									deal->_attacks.crush.lo=deal->_attacks.crush.hi=(WORD)FloatToNearestInt(dmg);
								}
							}

							float dur=_CalcUpgradedValue(idc.durDizzyBase,idc.durDizzyPerGrade,infoIdc.grdInactive);
							if (TRUE)
							{
								DealEntry *entry=rec->GetDealEntry<Deal_Dizzy>();
								if (entry)
								{
									Deal_Dizzy*deal=(Deal_Dizzy*)entry->deal;
									deal->dur=ANIMTICK_FROM_SECOND(dur);
								}
							}
						}

						eo=(CLoEffectObj*)_level->CreateObj(rec->param->GetEoClass());
						if (eo)
						{
							i_math::vector3df dir;
							dir.setXZ(e.strike->GetDir());
							eo->SetHost(e.loTarget->GetID());
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

	return FALSE;
}


void CLevelAbility_DeathCall::_MakeDmg(BOOL bDefaultSkill,LevelOSB &osbSrc,CLevelObj *loTarget,int nDmg,LevelStrike *strike,LevelOpLink &link)
{
	_MakeDmg_Default(bDefaultSkill,osbSrc,loTarget,nDmg,strike,link);
}

void CLevelAbility_DeathCall::_MakeModDmg(BOOL bDefaultSkill,LeModDamageAttr &e,LevelOSB &osbSrc,CLevelObj *loTarget)
{
}

void CLevelAbility_DeathCall::_MakeKill(BOOL bDefaultSkill,LeKill &e)
{
	if (!_MakeKill_TeleportSword(bDefaultSkill,e))
	if (!_MakeKill_ObliterateBow(bDefaultSkill,e))
	if (!_MakeKill_FlameBlade(bDefaultSkill,e))
		_MakeKill_BloodTeeth(bDefaultSkill,e);
}

void CLevelAbility_DeathCall::_OnEvent(LevelEvent &e0)
{
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

				if ((idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_AttackA))||(idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_FuryA)))
				{
					BOOL bDefaultSkill=idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_AttackA);
					_MakeDmg(bDefaultSkill,*e.osbSrc,e.loTarget,e.nDmg,e.strike,e.link);
				}
			}
		}
	}

	if (e0.GetType()==LET_PreDamage)
	{
		LePreDamage &e=(LePreDamage&)e0;	
		if (e.osb)
		{
			_MakePreDmg_FlashSwing(e);
		}
	}

	if (e0.GetType()==LET_PostCreateEo)
	{
		LePostCreateEo &e=(LePostCreateEo&)e0;	
		if (e.osbSrc)
		{
			_MakePostCreateEo_FlashSwing(e);
		}
	}


	if (e0.GetType()==LET_ModDamageAttr)
	{
		LeModDamageAttr &e=(LeModDamageAttr &)e0;	
		if (e.osbSrc)
		{
			CLevelSkill *skill=e.osbSrc->GetRootSkill();
			if (skill)
			{
				LevelRecordSkill *recSkill=skill->GetRec();
				RecordID idSkill=recSkill->GetID();

				if ((idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_AttackA))||
					(idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_FuryA)))
				{
					BOOL bDefaultSkill=idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_AttackA);
					_MakeModDmg(bDefaultSkill,e,*e.osbSrc,e.loTarget);
				}
			}
		}
	}


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

				if ((idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_AttackA))||
					(idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_FuryA)))
				{
					BOOL bDefaultSkill=idSkill==_skillsRT.GetSkillRecordID(LevelAbilityAction_AttackA);
					_MakeKill(bDefaultSkill,e);
				}
			}
		}

	}

}


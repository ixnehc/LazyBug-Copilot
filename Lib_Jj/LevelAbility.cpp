
#include "stdh.h"


#include "LevelRecordUpgrade.h"

#include "Level.h"
#include "LevelRecords.h"

#include "LevelEvents.h"

#include "Deal_CreateEo.h"
 
#include "Skill_Zeal.h"
  
#include "LevelAbility.h"

#include "Ability_Unarmed.h"
#include "Ability_Fire.h"
#include "Ability_UtumTide.h"
#include "Ability_FlashSwing.h"
#include "Ability_DeathCall.h"
#include "Ability_FlameBlade.h"
#include "Ability_LightningBow.h"
#include "Ability_HonorSword.h"
#include "Ability_SkullSword.h"
#include "Ability_TeleportSword.h"
#include "Ability_PhantomDagger.h"
#include "Ability_BloodTeeth.h"
#include "Ability_ObliterateBow.h"
#include "Ability_Nameless.h"

#include "Ability_WolfSkin.h"
#include "Ability_TalBless.h"
#include "Ability_AnWeep.h"
#include "Ability_BlackSteel.h"
#include "Ability_HunterPlate.h"
#include "Ability_SimCurse.h"
#include "Ability_WhirlWind.h"
#include "Ability_HonorPlate.h"
#include "Ability_7sonLeather.h"
#include "Ability_Frost.h"
#include "Ability_EliPromise.h"

#include "Spell_FireBall.h"

#include "Ability_ExplodeOil.h"
#include "Ability_WeaponInductionStone.h"
#include "Ability_ToeStone.h"
#include "Ability_SacredArrow.h"
#include "Ability_Bomb.h"
#include "Ability_MagicRing.h"
#include "Ability_MoneyBag.h"
#include "Ability_GemPot.h"
#include "Ability_HPAmulet.h"
#include "Ability_SPAmulet.h"
#include "Ability_HPPotion.h"
#include "Ability_SPPotion.h"
#include "Ability_VampireRing.h"
#include "Ability_ShieldMask.h"
#include "Ability_Whetstone.h"
#include "Ability_HonorBook.h"
#include "Ability_ShieldAmulet.h"
#include "Ability_HPFlusk.h"

#include "Ability_StormEye.h"

#include "Poem_ChartI.h"
#include "Poem_ChartII.h"
#include "Poem_ChartIII.h"

#include "Banner_Fire.h"
#include "Banner_Wolf.h"

#include "Ability_PushSlideway.h"
//XXXXX:More Ability



void CLevelAbilityUpgrade_LevelUp::Upgrade(CLevelAbility *ability,LevelAwardSeed &seed)
{
	ability->_grd++;
}

//////////////////////////////////////////////////////////////////////////
//AbilitiesVerCache

BOOL AbilitiesVerCache::CheckUpdateToDate(LevelPlayerStates *lps)
{
	if ((lps->abilities.GetVerDB()==verAbilities)&&
		(lps->artifacts.GetVerDB()==verArtifacts)&&
		(lps->equip.GetVerDB()==verEquip))
		return TRUE;
	return FALSE;
}

void AbilitiesVerCache::SetUpdateToData(LevelPlayerStates *lps)
{
	verAbilities=lps->abilities.GetVerDB();
	verArtifacts=lps->artifacts.GetVerDB();
	verEquip=lps->equip.GetVerDB();
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility::SkillRuntime

void CLevelAbility::SkillRuntime::Clear()
{
	for (int i=0;i<ARRAY_SIZE(recSkills);i++)
	{
		SAFE_RELEASE(recSkills[i]);
	}
	Zero();
}

//////////////////////////////////////////////////////////////////////////
//CLevelAbility

void CLevelAbility::Init(CLevelAbilities *abilities)	
{		
	_ownerAbilities=abilities;
	_owner=abilities->GetOwner();	
	if (_owner)
		_level=_owner->GetLevel();

	_EquipInitialUpgrade();
	_InitTechs();
	_ValidateTechs();
}

WpnIdcInfo &CLevelAbility::_GetWpnIdcInfo()
{
	if (_ownerAbilities)
		return _ownerAbilities->GetWpnIdcInfo();
	static WpnIdcInfo t;
	return t;
}

void CLevelAbility::_AddSkillRT(CRecords *recordsSkill,LevelAbilityAction action,RecordID idSkill)
{
	if (action>=LevelAbilityAction_Max)
		return;

	if (idSkill!=RecordID_Invalid)
	{
		LevelRecordSkill *rec=(LevelRecordSkill *)recordsSkill->GetRecord(idSkill);
		if (rec)
			_skillsRT.recSkills[action]=(LevelRecordSkill *)rec->Clone();
	}
	_skillsRT.idSkills[action]=idSkill;

}

void CLevelAbility::_AddSkillRT(LevelAbilityAction action,RecordID idSkill)
{
	if (!_owner)
		return;

	CLevelRecords *records=_level->GetRecords();
	assert(records);
	assert(records->GetRecords_Skill());

	_AddSkillRT(records->GetRecords_Skill(),action,idSkill);
}

void CLevelAbility::_BuildSkillRT(AbilityActionSettings &settings)
{
	for (int i=0;i<settings.settings.size();i++)
	{
		AbilityActionSetting &setting=settings.settings[i];
		_AddSkillRT(setting.action,setting.idSkill);

//		_ApplyAttack(_skillsRT.GetSkillRecord(setting.action),setting.attack);
	}
}


void CLevelAbility::_SaveSync_SkillsRT(CDataPacket &dp)
{
	assert(LevelAbilityAction_Max<=32);
	Bitset<1> bits;
	bits.resize(LevelAbilityAction_Max);
	bits.resetAll();
	for (int i=0;i<LevelAbilityAction_Max;i++)
	{
		if (_skillsRT.idSkills[i]!=RecordID_Invalid)
			bits.set(i);
	}

	DWORD *v=(DWORD *)bits.getdata();
	dp.Data_NextDword()=*v;

	for (int i=0;i<LevelAbilityAction_Max;i++)
	{
		if (_skillsRT.idSkills[i]!=RecordID_Invalid)
			dp.Data_WriteSimple(_skillsRT.idSkills[i]);
	}
}

void CLevelAbility::_LoadSync_SkillsRT(CDataPacket &dp,CRecords *recordsSkill)
{
	_ClearSkillsRT();
	
	assert(LevelAbilityAction_Max<=32);
	Bitset<1> bits;
	bits.resize(LevelAbilityAction_Max);

	DWORD v=dp.Data_NextDword();
	bits.setdata((BYTE*)&v);

	for (int i=0;i<LevelAbilityAction_Max;i++)
	{
		if (bits.test(i))
		{
			RecordID idSkill;
			dp.Data_ReadSimple(idSkill);
			_AddSkillRT(recordsSkill,(LevelAbilityAction)i,idSkill);
		}
	}
}


void CLevelAbility::_BuildGradeRT()
{
	_grdRT=LeModAbilityGrade::Send(this,_grd);
}

LevelRecordSkill *CLevelAbility::GetSkillRecordRT_OnClient(LevelAbilityAction action)
{			
	if (action<LevelAbilityAction_Max)
		return _skillsRT.recSkills[action];		
	return NULL;
}

RecordID CLevelAbility::GetSkillIDRT_OnClient(LevelAbilityAction action)		
{			
	if (action<LevelAbilityAction_Max)
		return _skillsRT.idSkills[action];		
	return RecordID_Invalid;
}

LevelRecordSkill *CLevelAbility::GetSkillRecordRT(LevelAbilityAction action)
{
	_UpdateRT();

	if (action<LevelAbilityAction_Max)
		return _skillsRT.recSkills[action];
	return NULL;
}

BOOL CLevelAbility::CheckAbilityActionSkillRecord_Attack(LevelRecordSkill *rec)
{
	_UpdateRT();

	LevelAbilityAction actionsAttack[]=
	{
		LevelAbilityAction_AttackA,
		LevelAbilityAction_AttackA_Dash,
		LevelAbilityAction_AttackA_RunningDash,
		LevelAbilityAction_AttackB,
		LevelAbilityAction_AttackB_Dash,
		LevelAbilityAction_AttackC,
		LevelAbilityAction_AttackC_Dash,
		LevelAbilityAction_AttackD,
		LevelAbilityAction_AttackD_Dash,
		LevelAbilityAction_TeleLeftA,
		LevelAbilityAction_TeleRightA,
		LevelAbilityAction_TeleBackA,
		LevelAbilityAction_AttackAR,
		LevelAbilityAction_AttackAR_Dash,
		//XXXXX:More LevalAbilityAction
	};

	for (int i=0;i<ARRAY_SIZE(actionsAttack);i++)
	{
		if (_skillsRT.recSkills[actionsAttack[i]]==rec)
			return TRUE;
	}
	return FALSE;
}

BOOL CLevelAbility::CheckAbilityActionSkillRecord_Fury(LevelRecordSkill *rec)
{
	_UpdateRT();

	LevelAbilityAction actionsAttack[]=
	{
		LevelAbilityAction_FuryA,
		LevelAbilityAction_FuryB,
		LevelAbilityAction_FuryC,
		LevelAbilityAction_FuryD,
		//XXXXX:More LevalAbilityAction
	};

	for (int i=0;i<ARRAY_SIZE(actionsAttack);i++)
	{
		if (_skillsRT.recSkills[actionsAttack[i]]==rec)
			return TRUE;
	}
	return FALSE;
}




void CLevelAbility::_UpdateStackCount()
{
	//更新stack
	memset(_stacksRT,0,sizeof(_stacksRT));
	if (_owner)
	{
		extern int LevelUtil_CalcSkillStack(LevelSkillType &tpSkill,CLevelObj *owner);
		for (int i=LevelAbilityAction_First;i<LevelAbilityAction_Max;i++)
			_stacksRT[i]=LevelUtil_CalcSkillStack(GetSkillType((LevelAbilityAction)i),_owner);
	}
}

void CLevelAbility::_SaveSync_StackCount(CDataPacket &dp)
{
	dp.Data_WriteData(_stacksRT,sizeof(_stacksRT));
}

void CLevelAbility::_LoadSync_StackCount(CDataPacket &dp)
{
	dp.Data_ReadData(_stacksRT,sizeof(_stacksRT));
}

void CLevelAbility::_SetStack(LevelAbilityAction action,DWORD count)
{
	if (action<LevelAbilityAction_Max)
	{
		if (count>255)
			count=255;
		_stacksRT[action]=(BYTE)count;
	}
}


void CLevelAbility::_ClearTechs()
{
	for (int i=0;i<_entriesTech.size();i++)
	{
		CLevelTech *tech=_entriesTech[i].tech;
		if (tech)
		{
			tech->OnDestroy();
			Safe_Class_Delete(tech);
		}
		Safe_Class_Delete(_entriesTech[i].sync);
	}
	_entriesTech.clear();
}


void CLevelAbility::_AddTech(LevelTechParam *param)
{
	TechEntry entry;
	entry.param=param;
	entry.sync=(LevelTechSync *)param->GetSyncClass()->New();

	_entriesTech.push_back(entry);
}


void CLevelAbility::_ValidateTechs()
{
	for (int i=0;i<_entriesTech.size();i++)
	{
		BOOL bValid=FALSE;
		TechEntry &e=_entriesTech[i];
		if (e.param)
			bValid=e.param->bValid;

		if (bValid)
		{
			if (!e.tech)
			{
				e.tech=(CLevelTech*)e.param->GetTechClass()->New();
				e.tech->_owner=this;
				e.tech->_param=e.param;

				e.tech->OnCreate();
			}
		}
		else
		{
			if (e.tech)
			{
				e.tech->OnDestroy();
				Safe_Class_Delete(e.tech);
			}
		}
	}
}

void CLevelAbility::_UpdateTechs(LevelTick dt)
{
	_ValidateTechs();
	for (int i=0;i<_entriesTech.size();i++)
	{
		TechEntry &e=_entriesTech[i];
		if (e.tech)
			e.tech->OnUpdate(dt);
	}

}


void CLevelAbility::_SaveSync_Techs(CDataPacket &dp)
{
	for (int i=0;i<_entriesTech.size();i++)
	{
		TechEntry &e=_entriesTech[i];
		assert(e.param);
		assert(e.sync);
		if (e.tech)
		{
			dp.Data_NextByte()=1;
			e.tech->SaveSync(*e.sync);
			DWORD sz;
			void *data=e.sync->GetData(sz);
			dp.Data_WriteData(data,sz);
		}
		else
			dp.Data_NextByte()=0;
	}
	
}

void CLevelAbility::_LoadSync_Techs(CDataPacket &dp)
{
	for (int i=0;i<_entriesTech.size();i++)
	{
		TechEntry &e=_entriesTech[i];
		assert(e.param);
		assert(e.sync);
		if (dp.Data_NextByte()==1)
		{
			DWORD sz;
			void *data=e.sync->GetData(sz);
			dp.Data_ReadData(data,sz);
		}
	}
}

void CLevelAbility::_SaveSync_GradeRT(CDataPacket &dp)
{
	dp.Data_NextByte()=_grdRT;
}

void CLevelAbility::_LoadSync_GradeRT(CDataPacket &dp)
{
	_grdRT=dp.Data_NextByte();
}


void CLevelAbility::SaveSync(CDataPacket &dp)
{
	_SaveSync(dp);
	_SaveSync_GradeRT(dp);
	_SaveSync_StackCount(dp);
	_SaveSync_Techs(dp);
}

void CLevelAbility::LoadSync(CDataPacket &dp,CRecords *records)
{
	_LoadSync(dp,records);
	_LoadSync_GradeRT(dp);
	_LoadSync_StackCount(dp);
	_LoadSync_Techs(dp);

}

LevelTechSync *CLevelAbility::FindTechSync(CClass *clss)
{
	for (int i=0;i<_entriesTech.size();i++)
	{
		if (_entriesTech[i].param->GetSyncClass()==clss)
		{
			return _entriesTech[i].sync;
		}
	}
	return NULL;
}


void CLevelAbility::Update(LevelTick dt)
{
	_UpdateRT();

	_UpdateStackCount();
	_UpdateTechs(dt);
	_OnUpdate(dt);
}

void CLevelAbility::HandleEvent(LevelEvent &e)
{
	for (int i=0;i<_entriesTech.size();i++)
	{
		TechEntry &entry=_entriesTech[i];
		if (entry.tech)
		{
			entry.tech->OnEvent(e);
			if (e.bHandled)
				return;
		}
	}

	_OnEvent(e);
}

void CLevelAbility::_ClearRT()
{
	_OnClearRT();

	for (int i=0;i<_entriesTech.size();i++)
	{
		TechEntry &entry=_entriesTech[i];
		if (entry.tech)
			entry.tech->OnClearRT();
	}

	_verCache.Zero();

}


void CLevelAbility::_UpdateRT()
{
	_ownerAbilities->CollectWpnInductionInfo();

	extern LevelPlayerStates *LevelUtil_GetLPS(CLevelObj *lo);
	LevelPlayerStates *lps=LevelUtil_GetLPS(_owner);
	if (!lps)
		return;

	if (_verCache.CheckUpdateToDate(lps))
		return;//没有变化

	_ClearRT();

	_OnBuildRT();
	
	_ValidateTechs();

	for (int i=0;i<_entriesTech.size();i++)
	{
		TechEntry &entry=_entriesTech[i];
		if (entry.tech)
			entry.tech->OnBuildRT();
	}

	_verCache.SetUpdateToData(lps);

}

void CLevelAbility::HandleStartDay()
{
	_OnStartDay();
}


void CLevelAbility::HandleEndDay()
{
	for (int i=0;i<_entriesTech.size();i++)
	{
		TechEntry &entry=_entriesTech[i];
		if (entry.tech)
			entry.tech->OnEndDay();
	}

	_OnEndDay();
}

void CLevelAbility::HandleDaily()
{
	_OnDaily();
}


void CLevelAbility::BuildArtifactState(LevelItemState &stateItem)
{
	_UpdateRT();

	for (int i=0;i<_entriesTech.size();i++)
	{
		TechEntry &entry=_entriesTech[i];
		if (entry.tech)
			entry.tech->OnBuildArtifactState(stateItem);
	}

	_OnBuildArtifactState(stateItem);


}


BOOL CLevelAbility::TestStartSkill(LevelSkillType &tpSkill)
{
	if (tpSkill.tpAbility_==GetType())
		return TRUE;
	return FALSE;
}

CLevelAbilityInitial *CLevelAbility::_GetInitialUpgrade(LevelAbilityType tpAbility)
{
	if (_owner)
	{
		CLevel *level=_owner->GetLevel();
		LevelRecordUpgrade *rec=level->GetRecords()->GetInitialUpgrade(tpAbility);
		if (rec)
			return (CLevelAbilityInitial *)rec->upgrade;
	}
	return NULL;
}

CLevelAbility *CLevelAbility::_GetActiveAbility(LevelAbilityType tp)
{
	if (_ownerAbilities)
		return _ownerAbilities->GetActiveAbility(tp);
	return NULL;
}


void CLevelAbility::_ApplyAttackMods(LevelAttr_AttackMods &mods,HitEx &hitEx,int nRepeat)
{
	LevelAttr_Hit *hit=hitEx.Get();
	hit->hit+=mods.modsHit.atkAdd*nRepeat;
}

void CLevelAbility::_ApplyAttackMods(LevelAttr_AttackMods &mods,Deal_Dmg *deal,int nRepeat)
{
	if (!deal)
		return;
	LevelAttr_Damages *damages=deal->_attacks.Get();
	for (int i=0;i<DamageAttrType_Max;i++)
	{
		if (mods.modsDamage[i].atkAdd>0)
		{
			damages->damages[i].lo+=mods.modsDamage[i].atkAdd*nRepeat;
			damages->damages[i].hi+=mods.modsDamage[i].atkAdd*nRepeat;
		}
	}
}

void CLevelAbility::_ApplyDefendMods(LevelAttr_DefendMods &mods,LevelItemState &state,int nRepeat)
{
	ItemBuff buff;
	//ItemBuff_FireResist
	if (mods.modsResist[DamageAttrType_Fire].defAdd>0)
	{
		buff.Set_AddFireResist(mods.modsResist[DamageAttrType_Fire].defAdd*nRepeat);
		state.AddItemBuff(buff);
	}

	//ItemBuff_ElecResist
	if (mods.modsResist[DamageAttrType_Lightning].defAdd>0)
	{
		buff.Set_AddElecResist(mods.modsResist[DamageAttrType_Lightning].defAdd*nRepeat);
		state.AddItemBuff(buff);
	}

	//ItemBuff_ColdResist
	if (mods.modsResist[DamageAttrType_Cold].defAdd>0)
	{
		buff.Set_AddColdResist(mods.modsResist[DamageAttrType_Cold].defAdd*nRepeat);
		state.AddItemBuff(buff);
	}

	//ItemBuff_PoisonResist
	if (mods.modsResist[DamageAttrType_Poison].defAdd>0)
	{
		buff.Set_AddPoisonResist(mods.modsResist[DamageAttrType_Poison].defAdd*nRepeat);
		state.AddItemBuff(buff);
	}
	
	//ItemBuff_PhysDef
	if (mods.modsResist[DamageAttrType_Pierce].defAdd>0)
	{
		assert(mods.modsResist[DamageAttrType_Crush].defAdd==mods.modsResist[DamageAttrType_Pierce].defAdd);
		buff.Set_AddPhysDef(mods.modsResist[DamageAttrType_Pierce].defAdd*nRepeat);
		state.AddItemBuff(buff);
	}
	//ItemBuff_PhysDefRate
	if (mods.modsResist[DamageAttrType_Pierce].defRate>0.0f)
	{
		assert(mods.modsResist[DamageAttrType_Crush].defRate==mods.modsResist[DamageAttrType_Pierce].defRate);
		buff.Set_AddPhysDef_Rate((int)(mods.modsResist[DamageAttrType_Pierce].defRate*(float)nRepeat*100.0f));
		state.AddItemBuff(buff);
	}
	//ItemBuff_PhysDef_Base
	if (mods.modsResist[DamageAttrType_Pierce].defBase>0)
	{
		assert(mods.modsResist[DamageAttrType_Crush].defBase==mods.modsResist[DamageAttrType_Pierce].defBase);
		buff.Set_AddPhysDef_Base(mods.modsResist[DamageAttrType_Pierce].defBase*nRepeat);
		state.AddItemBuff(buff);
	}


	//XXXXX: more ItemBuffType

}

void CLevelAbility::_ApplyDefendMods(CLevelAbilityInitial_Armor *upgradeArmorInitial,LevelItemState &state,LevelGrade grd)
{
	_ApplyDefendMods(*upgradeArmorInitial->_baseDefend.Get(),state,1);
	_ApplyDefendMods(*upgradeArmorInitial->_upgradeDefend.Get(),state,grd);
}

float CLevelAbility::_CalcUpgradedValue(float base,float upgrade,LevelGrade grd)
{
	if (grd>1)
		return base+upgrade*(float)(grd-1);
	else
		return base;
}

float CLevelAbility::_CalcUpgradedValue(LevelUpgradableValue &v,LevelGrade grd)
{
	return _CalcUpgradedValue(v.base,v.perGrade,grd);
}


void CLevelAbility::_ApplyAttack(LevelRecordSkill *recSkill,AbilityAttackSetting &setting)
{
	if (!recSkill)
		return;

	recSkill->hit.DiscardCache();
	recSkill->hit=setting.hit;
	Deal_Dmg *dealDmg=recSkill->GetDeal<Deal_Dmg>();
	if (dealDmg)
	{
		dealDmg->_attacks.DiscardCache();
		dealDmg->_attacks=setting.dmgs;
	}
}

void CLevelAbility::_ApplyAttackMods(LevelAttr_AttackMods &mods,AbilityAttackSetting &setting)
{
	if (_grd>1)
		mods.MergeFrom(*setting.modsPerGrade.Get(),_grd-1);
	extern WORD LevelUtil_GetStrength(CLevelObj *lo);
	WORD grdStr=LevelUtil_GetStrength(_owner);
	if (grdStr>1)
		mods.MergeFrom(*setting.modsPerStr.Get(),grdStr-1);
}

void CLevelAbility::_ApplyBulletAttack(Deal_CreateEo *deal,AbilityAttackSetting &setting)
{
	if (deal)
	{
		LevelRecordEo *rec=deal->BeginModify(_level);
		if (rec)
		{
			rec->hit.DiscardCache();
			rec->hit=setting.hit;

			Deal_Dmg *dealDmg=rec->GetDeal<Deal_Dmg>();
			if (dealDmg)
			{
				dealDmg->_attacks.DiscardCache();
				dealDmg->_attacks=setting.dmgs;
			}

			deal->EndModify();
		}
	}
}


void CLevelAbility::_ApplyBulletAttack(LevelRecordSkill *recSkill,AbilityAttackSetting &setting)
{
	if (recSkill)
	{
		Deal_CreateEo *deal=recSkill->GetDeal<Deal_CreateEo>();
		_ApplyBulletAttack(deal,setting);
	}
}

void CLevelAbility::_ApplyBulletCount(LevelRecordSkill *recSkill)
{
	if (recSkill)
	{
		extern int LevelUtil_GetArrowCountAddOn(CLevelObj *lo);
		SkillParam_Zeal *param=recSkill->GetParam<SkillParam_Zeal>();
		if (param)
		{
			extern int LevelUtil_GetArrowCountAddOn(CLevelObj *lo);
			param->countCast=1+LevelUtil_GetArrowCountAddOn(_owner);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//CAbilitiesDailyHandler
void CAbilitiesDailyHandler::Init(CLevelAbilities *abilities)
{
	_abilities=abilities;
	_entries[0].tp=LevelAbilityType_MoneyBag;
	_entries[1].tp=LevelAbilityType_GemPot;
	_entries[2].tp=LevelAbilityType_HPAmulet;
	_entries[3].tp=LevelAbilityType_SPAmulet;

	for (int i=0;i<ARRAY_SIZE(_entries);i++)
		_entries[i].bHandled=TRUE;

	_tAccum=0.0f;
}

void CAbilitiesDailyHandler::SavePersist(CDataPacket &dp)
{
	dp.Data_NextByte()=1;//Ver

	dp.Data_WriteSimple(_tAccum);

	dp.Data_NextByte()=(BYTE)ARRAY_SIZE(_entries);
	for (int i=0;i<ARRAY_SIZE(_entries);i++)
	{
		dp.Data_NextShort()=_entries[i].tp;
		dp.Data_NextByte()=_entries[i].bHandled;
	}
}

void CAbilitiesDailyHandler::LoadPersist(CDataPacket &dp)
{
	DWORD ver=dp.Data_NextByte();

	dp.Data_ReadSimple(_tAccum);

	DWORD c=dp.Data_NextByte();

	for (int i=0;i<c;i++)
	{
		LevelAbilityType tp=(LevelAbilityType)dp.Data_NextShort();
		BOOL bHandled=(BOOL)dp.Data_NextByte();

		for (int j=0;j<ARRAY_SIZE(_entries);j++)
		{
			if (_entries[j].tp==tp)
			{
				_entries[j].bHandled=bHandled;
				break;
			}
		}
	}
}


void CAbilitiesDailyHandler::StartDay()
{
	for (int i=0;i<ARRAY_SIZE(_entries);i++)
		_entries[i].bHandled=FALSE;
	_tAccum=2.0f;
}

void CAbilitiesDailyHandler::Update(LevelTick dt)
{
	_tAccum-=ANIMTICK_TO_SECOND(dt);
	if (_tAccum<=0.0f)
	{
		BOOL bAnyHandled=FALSE;
		for (int i=0;i<ARRAY_SIZE(_entries);i++)
		{
			if (_entries[i].bHandled)
				continue;

			CLevelAbility *ability=_abilities->GetActiveAbility(_entries[i].tp);
			if (!ability)
				continue;
			ability->HandleDaily();
			_entries[i].bHandled=TRUE;
			bAnyHandled=TRUE;
			break;
		}
		if (bAnyHandled)
			_tAccum+=2.0f;
		else
			_tAccum+=1.0f;
	}
}

void CAbilitiesDailyHandler::OnLeaveLevel()
{
	if (_abilities)
		_abilities->SetDirty();
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbilities

IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgrade_Void);


#define ABILITY_ENTRY(clss)														\
{																									\
	clss *ability=Class_New2(clss);												\
	ability->Init(this);																\
	_abilities[ability->GetType()]=ability;									\
}

void CLevelAbilities::Init(CLevelObj *owner,CLevelPlayer *player)
{
	_player=player;
	_owner=owner;

	_handlerDaily.Init(this);

	ABILITY_ENTRY(CLevelAbility_Unarmed);
	ABILITY_ENTRY(CLevelAbility_Fire);
	ABILITY_ENTRY(CLevelAbility_UtumTide);
	ABILITY_ENTRY(CLevelAbility_FlashSwing);
	ABILITY_ENTRY(CLevelAbility_DeathCall);
	ABILITY_ENTRY(CLevelAbility_FlameBlade);
	ABILITY_ENTRY(CLevelAbility_LightningBow);
	ABILITY_ENTRY(CLevelAbility_HonorSword);
	ABILITY_ENTRY(CLevelAbility_SkullSword);
	ABILITY_ENTRY(CLevelAbility_TeleportSword);
	ABILITY_ENTRY(CLevelAbility_PhantomDagger);
	ABILITY_ENTRY(CLevelAbility_BloodTeeth);
	ABILITY_ENTRY(CLevelAbility_ObliterateBow);
	ABILITY_ENTRY(CLevelAbility_Nameless);

	ABILITY_ENTRY(CLevelAbility_WolfSkin);
	ABILITY_ENTRY(CLevelAbility_TalBless);
	ABILITY_ENTRY(CLevelAbility_AnWeep);
	ABILITY_ENTRY(CLevelAbility_BlackSteel);
	ABILITY_ENTRY(CLevelAbility_HunterPlate);
	ABILITY_ENTRY(CLevelAbility_SimCurse);
	ABILITY_ENTRY(CLevelAbility_WhirlWind);
	ABILITY_ENTRY(CLevelAbility_HonorPlate);
	ABILITY_ENTRY(CLevelAbility_7sonLeather);
	ABILITY_ENTRY(CLevelAbility_Frost);
	ABILITY_ENTRY(CLevelAbility_EliPromise);

	ABILITY_ENTRY(CLevelSpell_FireBall);

    ABILITY_ENTRY(CLevelAbility_ExplodeOil);

	ABILITY_ENTRY(CLevelAbility_WeaponInductionStone);
	ABILITY_ENTRY(CLevelAbility_ToeStone);
	ABILITY_ENTRY(CLevelAbility_SacredArrow);
	ABILITY_ENTRY(CLevelAbility_Bomb);
	ABILITY_ENTRY(CLevelAbility_MagicRing);
	ABILITY_ENTRY(CLevelAbility_MoneyBag);
	ABILITY_ENTRY(CLevelAbility_GemPot);
	ABILITY_ENTRY(CLevelAbility_HPAmulet);
	ABILITY_ENTRY(CLevelAbility_SPAmulet);
	ABILITY_ENTRY(CLevelAbility_HPPotion);
	ABILITY_ENTRY(CLevelAbility_SPPotion);
	ABILITY_ENTRY(CLevelAbility_VampireRing);
	ABILITY_ENTRY(CLevelAbility_ShieldMask);
	ABILITY_ENTRY(CLevelAbility_Whetstone);
	ABILITY_ENTRY(CLevelAbility_HonorBook);
	ABILITY_ENTRY(CLevelAbility_ShieldAmulet);
	ABILITY_ENTRY(CLevelAbility_HPFlusk);

	ABILITY_ENTRY(CLevelAbility_StormEye);

	ABILITY_ENTRY(CLevelPoem_ChartI);
	ABILITY_ENTRY(CLevelPoem_ChartII);
	ABILITY_ENTRY(CLevelPoem_ChartIII);

	ABILITY_ENTRY(CLevelBanner_Fire);
	ABILITY_ENTRY(CLevelBanner_Wolf);

	ABILITY_ENTRY(CLevelAbility_PushSlideway);
	//XXXXX:More Ability
}

void CLevelAbilities::Clear()
{
	for (int i=0;i<ARRAY_SIZE(_abilities);i++)
	{
		if (_abilities[i])
		{
			_abilities[i]->Clear();
			Safe_Class_Delete(_abilities[i]);
		}
	}

	_infoWpnIdc.verCache.Zero();
	Zero();
}

void CLevelAbilities::OnEnterLevel()
{
	_handlerDaily.OnEnterLevel();

	for (int i=0;i<ARRAY_SIZE(_abilities);i++)
	{
		if (_abilities[i])
		{
			if (_abilities[i]->IsActive())
				_abilities[i]->OnEnterLevel();
		}
	}
}

void CLevelAbilities::OnLeaveLevel()
{
	for (int i=0;i<ARRAY_SIZE(_abilities);i++)
	{
		if (_abilities[i])
		{
			if (_abilities[i]->IsActive())
				_abilities[i]->OnLeaveLevel();
		}
	}

	_handlerDaily.OnLeaveLevel();
}


void CLevelAbilities::Save(CDataPacket &dp)
{
	dp.Data_NextByte()=1;//Ver

	_handlerDaily.SavePersist(dp);
	for (int i=1;i<ARRAY_SIZE(_abilities);i++)
	{
		if (_abilities[i])
		{
			if (_abilities[i]->IsActive())
			{
				dp.Data_NextWord()=(WORD)i;
				DP_PreSafeSave(dp);
				_abilities[i]->SavePersist(dp);
				DP_PostSafeSave();
			}
		}
	}
	dp.Data_NextWord()=0;
}

void CLevelAbilities::Load(CDataPacket &dp)
{
	DWORD ver=dp.Data_NextByte();

	_handlerDaily.LoadPersist(dp);
	for (int i=0;i<ARRAY_SIZE(_abilities);i++)
	{
		if (_abilities[i])
			_abilities[i]->_bActive=FALSE;
	}
	while(1)
	{
		int idx=(int)dp.Data_NextWord();
		if (idx==0)
			break;
		DP_PreSafeLoad(dp);
		if (idx<ARRAY_SIZE(_abilities))
		{
			if (_abilities[idx])
			{
				_abilities[idx]->LoadPersist(dp);
				_abilities[idx]->_bActive=TRUE;
			}
		}
		DP_PostSafeLoad();
	}
}

CLevelAbility *CLevelAbilities::GetAbility(LevelAbilityType tp)
{
	if (((int)tp)>=LevelAbilityType_Max)
		return NULL;
	return _abilities[((int)tp)];
}

CLevelAbility *CLevelAbilities::GetActiveAbility(LevelAbilityType tp)
{
	CLevelAbility *ability=GetAbility(tp);
	if (!ability)
		return NULL;
	if (ability->IsActive())
		return ability;
	return NULL;
}



BOOL CLevelAbilities::ApplyUpgrade(CLevelAbilityUpgrade *upgrade,LevelAwardSeed &seed)
{
	CLevelAbility *ability=GetAbility(upgrade->GetAbilityType());
	if (ability)
	{
		if (upgrade->GetChannel()==CLevelAbilityUpgrade::Channel_Initial)
		{
			if (!ability->IsActive())
			{
				ability->SetActive();
				ability->OnEnterLevel();
			}
			return upgrade->Init(ability);
		}
		return ability->ApplyUpgrade(upgrade,seed);
	}
	return FALSE;
}

void CLevelAbilities::CollectWpnInductionInfo()
{
	if (!_player)
		return;
	if (!_player->GetLPS())
		return;
	if (_infoWpnIdc.verCache.CheckUpdateToDate(_player->GetLPS()))
		return;

	extern void LevelUtil_GetWeaponAbilities(CLevelObj *lo, LevelAbilityType &tpActive, LevelAbilityType&tpInactive);
	LevelUtil_GetWeaponAbilities(_owner, _infoWpnIdc.active, _infoWpnIdc.inactive);

	_infoWpnIdc.grdActive = _infoWpnIdc.grdInactive = 0;
	if (TRUE)
	{
		CLevelAbility *ability;
		ability = GetActiveAbility(_infoWpnIdc.active);
		if (ability)
			_infoWpnIdc.grdActive = ability->GetGradeRT();
		ability = GetActiveAbility(_infoWpnIdc.inactive);
		if (ability)
			_infoWpnIdc.grdInactive = ability->GetGradeRT();
	}

	_infoWpnIdc.grdInduction = 3;

	if (TRUE)
	{
		CLevelAbility *ability = GetActiveAbility(LevelAbilityType_WeaponInductionStone);
		if (ability)
			_infoWpnIdc.grdInduction = ability->GetGradeRT();
	}

	_infoWpnIdc.verCache.SetUpdateToData(_player->GetLPS());
}


void CLevelAbilities::Update()
{
	if (!_owner)
		return;

	CollectWpnInductionInfo();

	LevelTick t=_owner->GetT();
	if (_tUpdate==0)
		_tUpdate=t;
	LevelTick dt=ANIMTICK_SAFE_MINUS(t,_tUpdate);

	_handlerDaily.Update(dt);

	for (int i=1;i<LevelAbilityType_Max;i++)
	{
		if (_abilities[i])
		{
			if (_abilities[i]->IsActive())
				_abilities[i]->Update(dt);
		}
	}
	_tUpdate=t;

}

void CLevelAbilities::HandleEvent(LevelEvent &e)
{
	for (int i=1;i<LevelAbilityType_Max;i++)
	{
		if (_abilities[i])
		{
			if (_abilities[i]->IsActive())
			{
				_abilities[i]->HandleEvent(e);
				if (e.bHandled)
					return;
			}
		}
	}
}

void CLevelAbilities::HandleStartDay()
{
	_handlerDaily.StartDay();

	for (int i=1;i<LevelAbilityType_Max;i++)
	{
		if (_abilities[i])
		{
			if (_abilities[i]->IsActive())
				_abilities[i]->HandleStartDay();
		}
	}
}


void CLevelAbilities::HandleEndDay()
{
	for (int i=1;i<LevelAbilityType_Max;i++)
	{
		if (_abilities[i])
		{
			if (_abilities[i]->IsActive())
				_abilities[i]->HandleEndDay();
		}
	}
}


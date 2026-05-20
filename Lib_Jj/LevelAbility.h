#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "anim/animdefines.h"

#include "LevelDefines.h"

#include "Deal_Dmg.h"

#include "LevelUpgrade.h"

#include "LevelUpgradableValue.h"

#include "LevelTech.h"

#define DEFINE_ABILITY_UPGRADE_CLASS(clss,tpAbility)											\
	DEFINE_CLASS(clss);																					\
	virtual LevelAbilityType GetAbilityType()											\
	{																														\
		return tpAbility;																			\
	}


#define IMPLEMENT_ABILITY_UPGRADE_CLASS(clss)													\
	CClass *GetClass_##clss()														\
	{																													\
		return Class_Ptr2(clss);															\
	}

#define DEFINE_ABILITY_CLASS(clss,tpAbility)											\
	DEFINE_CLASS(clss);																					\
	virtual LevelAbilityType GetType()											\
	{																														\
		return tpAbility;																			\
	}

#define DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(clss,clssInitialUpgrade,tpAbility)											\
	DEFINE_CLASS(clss);																					\
	virtual LevelAbilityType GetType()											\
	{																														\
	return tpAbility;																			\
	}																														\
	virtual void _EquipInitialUpgrade() override																														\
	{																														\
		_upgradeInitial=_GetInitialUpgrade<clssInitialUpgrade>();																														\
	}																														\
	clssInitialUpgrade *_upgradeInitial;


struct LevelPlayerStates;
struct AbilitiesVerCache
{
	AbilitiesVerCache()
	{
		Zero();
	}
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL CheckUpdateToDate(LevelPlayerStates *lps);
	void SetUpdateToData(LevelPlayerStates *lps);
	DWORD verAbilities;//_skillRT锟斤拷应锟斤拷LPSAbilities锟斤拷ver
	DWORD verArtifacts;//_skillRT锟斤拷应锟斤拷LPSArtifacts锟斤拷ver
	DWORD verEquip;//_skillRT锟斤拷应锟斤拷LPSEquip锟斤拷ver

};

struct AbilityAttackSetting
{
	HitEx hit;
	DamagesEx dmgs;

	AttackModsEx modsPerGrade;
	AttackModsEx modsPerStr;

	BEGIN_GOBJ_PURE(AbilityAttackSetting,1);

		GELEM_OBJ(HitEx,hit);
			GELEM_EDITOBJ("命中参数","命中参数");	
		GELEM_OBJ(DamagesEx,dmgs);
			GELEM_EDITOBJ("伤害","伤害");	

		GELEM_OBJ(AttackModsEx,modsPerGrade);
			GELEM_EDITOBJ("每级增加伤害","每级增加伤害");	
		GELEM_OBJ(AttackModsEx,modsPerStr);
			GELEM_EDITOBJ("每点力量值增加伤害","每点力量值增加伤害");	
	END_GOBJ();
};

struct AbilityActionSetting
{
	LevelAbilityAction action;

	RecordID idSkill;

// 	AbilityAttackSetting attack;

	BEGIN_GOBJ_PURE(AbilityActionSetting,1);
		GELEM_VAR_INIT(LevelAbilityAction,action,LevelAbilityAction_AttackA);
			GELEM_EDITVAR("Action",GVT_U,GSem(GSem_Interger,LevelAbilityActionConstraintStr),"Action");
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
// 		GELEM_OBJ(AbilityAttackSetting,attack);
// 			GELEM_EDITOBJ("锟斤拷锟斤拷锟斤拷锟斤拷","锟斤拷锟杰的癸拷锟斤拷锟斤拷锟斤拷");	
	END_GOBJ();
};

struct AbilityActionSettings
{
	std::vector<AbilityActionSetting> settings;

	BEGIN_GOBJ_PURE(AbilityActionSettings,1);

		GELEM_OBJVECTOR(AbilityActionSetting,settings);
			GELEM_EDITOBJ("Action设置","Action设置");	
	END_GOBJ();

};


class CLevelAbility;
struct LevelItemState;
class CLevelAbilityUpgrade:public CLevelUpgrade
{
public:
	enum Channel
	{
		Channel_None,
		Channel_Initial,//锟斤拷始
		Channel_LevelUp,//锟斤拷通锟斤拷锟斤拷
	};
	virtual CLevelUpgrade::Type GetUpgradeType()	{		return CLevelUpgrade::Ability;	}

	virtual LevelAbilityType GetAbilityType()=0;
	virtual BOOL CanUpgrade(CLevelAbility *ability)=0;
	virtual void MakeSeed(CLevelAbility *ability,LevelAwardSeed &seed)=0;
	virtual void Upgrade(CLevelAbility *ability,LevelAwardSeed &seed)=0;

	virtual Channel GetChannel() {		return Channel_None;	}

	virtual RecordID GetSkillID()	{		return RecordID_Invalid;	}

	virtual BOOL Init(CLevelAbility *ability)	{		return FALSE;	}
};

class CLevelAbilityInitial:public CLevelAbilityUpgrade
{
public:
	virtual Channel GetChannel()override	{		return Channel_Initial;	} 
	virtual BOOL CanUpgrade(CLevelAbility *ability)	{		return FALSE;	}
	virtual void MakeSeed(CLevelAbility *ability,LevelAwardSeed &seed)	{	}
	virtual void Upgrade(CLevelAbility *ability,LevelAwardSeed &seed)	{	}
	virtual const char *GetDesc(LevelAwardSeed &seed,CLevelRecords *records)	{		return "";	}
};

class CLevelAbilityUpgrade_LevelUp:public CLevelAbilityUpgrade
{
public:
	virtual Channel GetChannel()override	{		return Channel_LevelUp;	} 
	virtual BOOL CanUpgrade(CLevelAbility *ability)	{		return TRUE;	}
	virtual void MakeSeed(CLevelAbility *ability,LevelAwardSeed &seed)	{	}
	virtual void Upgrade(CLevelAbility *ability,LevelAwardSeed &seed)	;
	virtual const char *GetDesc(LevelAwardSeed &seed,CLevelRecords *records)	{		return "";	}
};


#define GELEM_ABILITY_BASE()							\
	GELEM_VAR_INIT(LevelGrade,_grd,1);GELEM_UID(100)

class CLevelAbilityInitial_Armor:public CLevelAbilityInitial
{
public:
protected:
	DefendModsEx _baseDefend;
	DefendModsEx _upgradeDefend;

	friend class CLevelAbility;
};

#define GELEM_ARMOR_UPGRADE_DEFEND()													\
		GELEM_OBJ(DefendModsEx,_baseDefend);							\
			GELEM_EDITOBJ("锟斤拷锟斤拷锟斤拷锟斤拷","锟斤拷锟斤拷");										\
		GELEM_OBJ(DefendModsEx,_upgradeDefend);									\
			GELEM_EDITOBJ("锟斤拷锟斤拷锟斤拷锟斤拷(每锟斤拷)","锟斤拷锟斤拷锟斤拷锟斤拷");	

class CLevelAbilityInitial_Shield:public CLevelAbilityInitial
{
public:
protected:

	friend class CLevelAbility;
};


struct WpnIdcInfo
{
	WpnIdcInfo()
	{
		grdInduction = 0;
		active = LevelAbilityType_None;
		inactive = LevelAbilityType_None;
		grdActive = 0;
		grdInactive = 0;
	}
	LevelAbilityType active;
	LevelAbilityGrade grdActive;
	LevelAbilityType inactive;
	LevelAbilityGrade grdInactive;
	LevelGrade grdInduction;

	AbilitiesVerCache verCache;
};


class CLevelObj;
class CRecords;
struct LevelRecordSkill;
class CRecord;
class CLevelAbilities;
struct LevelAttr_AttackMods;
struct HitEx;
class Deal_Dmg;
class Deal_CreateEo;
class CLevel;
class CLevelPlayer;
struct PoemAwards;
class CLevelAbility
{
public:
	struct SkillRuntime
	{
		SkillRuntime()
		{
			Zero();
		}
		void Zero()
		{
			memset(idSkills,0,sizeof(idSkills));
			memset(recSkills,0,sizeof(recSkills));
		}
		RecordID GetSkillRecordID(LevelAbilityAction action)
		{
			if (action<LevelAbilityAction_Max)
				return idSkills[action];
			return RecordID_Invalid;
		}
		LevelRecordSkill *GetSkillRecord(LevelAbilityAction action)
		{
			if (action<LevelAbilityAction_Max)
				return recSkills[action];
			return NULL;
		}

		void Clear();
		RecordID idSkills[LevelAbilityAction_Max];
		LevelRecordSkill *recSkills[LevelAbilityAction_Max];
	};


	CLevelAbility()
	{
		Zero();
	}
	void Zero()
	{
		_ownerAbilities=NULL;
		_owner=NULL;
		_level=NULL;
		_bActive=FALSE;
		memset(_stacksRT,0,sizeof(_stacksRT));
		_tUpdate=0;
		_grd=1;
		_grdRT=0;
	}
	void Init(CLevelAbilities *abilities);
	void Clear()	
	{ 
		_ClearRT();
		_ClearTechs();

		//锟皆凤拷锟斤拷一
		_skillsRT.Clear();
		_ClearGradeRT();
		Zero();	
	}

	void OnEnterLevel(){}
	void OnLeaveLevel(){}

	void Update(LevelTick dt);
	void HandleEvent(LevelEvent &e);
	void HandleStartDay();
	void HandleEndDay();
	void HandleDaily();
	void BuildArtifactState(LevelItemState &stateItem);

	virtual BOOL TestStartSkill(LevelSkillType &tpSkill);
	virtual void NotifyStartSkill(LevelSkillType &tpSkill)	{	}

	virtual LevelAbilityType GetType()=0;

	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()=0;


	virtual void SavePersist(CDataPacket &dp)
	{
		SaveGObj(dp,GetGObj());
	}
	virtual void LoadPersist(CDataPacket &dp)
	{
		LoadGObj(dp,GetGObj(),NULL);
	}

	virtual void SaveSync(CDataPacket &dp);
	virtual void LoadSync(CDataPacket &dp,CRecords *records);

	CLevelAbilities *GetOwnerAbilities()	{	return _ownerAbilities;	}
	CLevelObj *GetOwner()	{	return _owner;	}

	LevelSkillType GetSkillType(LevelAbilityAction action)
	{
		LevelSkillType tp;
		tp.tpAbility_=GetType();
		tp.actionAbility=action;
		return tp;
	}


	BOOL ApplyUpgrade(CLevelAbilityUpgrade *upgrade,LevelAwardSeed seed)
	{
		if (!upgrade->CanUpgrade(this))
			return FALSE;
		upgrade->Upgrade(this,seed);
		_ValidateTechs();

		return TRUE;
	}

	BOOL IsActive()	{		return _bActive;	}
	
	void SetActive()
	{
		_bActive=TRUE;
	}
	void SetInactive()
	{
		_bActive=FALSE;
	}
	virtual BOOL IsSpell()	{		return FALSE;	}
	virtual BOOL IsPoem()	{		return FALSE;	}
	virtual PoemAwards *FindPoemAwards(StringID nm)	{		return NULL;	}

	virtual BOOL IsConsumable()	{		return FALSE;	}
	virtual void Consume()	{	}
	virtual BOOL IsConsumed()	{		return FALSE;	}

	//Toggle
	virtual BOOL SupportToggle()	{		return FALSE;	}
	virtual AnimTick GetToggleOnCD()	{		return 0;	}
	virtual BOOL CheckInToggleOnCD()	{		return FALSE;	}
	virtual BOOL CheckToggledOn()	{		return FALSE;	}
	virtual BOOL Toggle(BOOL bOn)	{		return FALSE;	}

	//Charge
	virtual int GetChargeCount()	{		return LEVELABILITY_INFINITE_CHARGE;	}
	virtual int GetMaxChargeCount()	{		return LEVELABILITY_INFINITE_CHARGE;	}

	//Fury
	virtual BOOL CanFury()	{		return FALSE;	}

	//Guard
	virtual BOOL CanGuard()	{		return FALSE;	}


	virtual LevelRecordSkill *GetSkillRecordRT_OnClient(LevelAbilityAction action);
	virtual RecordID GetSkillIDRT_OnClient(LevelAbilityAction action);
	virtual LevelRecordSkill *GetSkillRecordRT(LevelAbilityAction action);
	BOOL CheckAbilityActionSkillRecord_Attack(LevelRecordSkill *rec);
	BOOL CheckAbilityActionSkillRecord_Fury(LevelRecordSkill *rec);

	virtual LevelSkillGrade GetSkillGradeRT()		{			return _grdRT;		}
	LevelGrade GetGradeRT()	{		return _grdRT;	}

	virtual int GetSkillStackRT(LevelAbilityAction action)	//锟斤拷锟斤拷锟斤拷锟街伙拷锟絚lient锟较碉拷锟斤拷
	{	
		if (action<LevelAbilityAction_Max)
			return _stacksRT[(int)action];	
		return 0;
	}


	LevelTechSync *FindTechSync(CClass *clss);
	template<typename T>
	T *FindTechSync()
	{
		return (T*)FindTechSync(Class_Ptr2(T));
	}
protected:

	virtual void _EquipInitialUpgrade()	{	}

	virtual void _OnBuildRT()=0;
	virtual void _OnClearRT()=0;
	virtual void _OnUpdate(LevelTick dt)	{	}
	virtual void _OnEvent(LevelEvent &e)	{	}
	virtual void _OnStartDay()	{	}
	virtual void _OnDaily()	{	}
	virtual void _OnEndDay()	{	}
	virtual void _OnBuildArtifactState(LevelItemState &state)	{	}

	static float _CalcUpgradedValue(float base,float upgrade,LevelGrade grd);
	static float _CalcUpgradedValue(LevelUpgradableValue &v,LevelGrade grd);

	LevelAbilityGrade _grd;

	CLevel *_level;
	CLevelAbilities *_ownerAbilities;
	CLevelObj *_owner;
	BOOL _bActive;
	AnimTick _tUpdate;

	WpnIdcInfo &_GetWpnIdcInfo();

	CLevelAbilityInitial *_GetInitialUpgrade(LevelAbilityType tpAbility);
	template <typename T>
	T *_GetInitialUpgrade(LevelAbilityType tpAbility)
	{
		CLevelAbilityInitial *upgrade=_GetInitialUpgrade(tpAbility);
		if (upgrade)
		{
			if (upgrade->GetClass()->IsSameWith(Class_Ptr2(T)))
				return (T*)upgrade;
		}
		return NULL;
	}


	CLevelAbilityInitial *_GetInitialUpgrade()
	{
		return _GetInitialUpgrade(GetType());
	}
	template <typename T>
	T *_GetInitialUpgrade()
	{
		return _GetInitialUpgrade<T>(GetType());
	}

	CLevelAbility *_GetActiveAbility(LevelAbilityType tp);


	void _ClearRT();
	void _UpdateRT();
	AbilitiesVerCache _verCache;

	void _ClearSkillsRT()	{		_skillsRT.Clear();	}
	void _AddSkillRT(LevelAbilityAction action,RecordID idSkill);
	void _AddSkillRT(CRecords *recordsSkill,LevelAbilityAction action,RecordID idSkill);
	void _BuildSkillRT(AbilityActionSettings &settings);
	void _SaveSync_SkillsRT(CDataPacket &dp);
	void _LoadSync_SkillsRT(CDataPacket &dp,CRecords *recordsSkill);
	SkillRuntime _skillsRT;

	void _ClearGradeRT()	{		_grdRT=0;	}
	void _BuildGradeRT();
	void _SaveSync_GradeRT(CDataPacket &dp);
	void _LoadSync_GradeRT(CDataPacket &dp);
	LevelAbilityGrade _grdRT;

	virtual void _SaveSync(CDataPacket &dp)=0;
	virtual void _LoadSync(CDataPacket &dp,CRecords *recordsSkill)=0;

	//Stack
	virtual void _UpdateStackCount();
	void _SaveSync_StackCount(CDataPacket &dp);
	void _LoadSync_StackCount(CDataPacket &dp);
	void _SetStack(LevelAbilityAction action,DWORD count);
	BYTE _stacksRT[LevelAbilityAction_Max];

	//Techs
	virtual void _InitTechs()	{	}
	void _ClearTechs();
	void _AddTech(LevelTechParam *param);
	void _ValidateTechs();
	void _UpdateTechs(LevelTick dt);
	void _SaveSync_Techs(CDataPacket &dp);
	void _LoadSync_Techs(CDataPacket &dp);

	struct TechEntry
	{
		TechEntry()
		{
			param=NULL;
			sync=NULL;
			tech=NULL;
		}
		LevelTechParam *param;
		LevelTechSync *sync;
		CLevelTech *tech;
	};

	std::vector<TechEntry> _entriesTech;


	//Utilities
	void _ApplyAttackMods(LevelAttr_AttackMods &mods,HitEx &hit,int nRepeat);
	void _ApplyAttackMods(LevelAttr_AttackMods &mods,Deal_Dmg *deal,int nRepeat);

	void _ApplyDefendMods(LevelAttr_DefendMods &mods,LevelItemState &state,int nRepeat);
	void _ApplyDefendMods(CLevelAbilityInitial_Armor *upgradeArmorInitial,LevelItemState &state,LevelGrade grd);

	void _ApplyAttack(LevelRecordSkill *recSkill,AbilityAttackSetting &setting);
	void _ApplyAttackMods(LevelAttr_AttackMods &mods,AbilityAttackSetting &setting);

	void _ApplyBulletAttack(LevelRecordSkill *recSkill,AbilityAttackSetting &setting);
	void _ApplyBulletAttack(Deal_CreateEo *deal,AbilityAttackSetting &setting);

	void _ApplyBulletCount(LevelRecordSkill *recSkill);

	friend class CLevelAbilities;
	friend class CLevelAbilityUpgrade_LevelUp;
};

class CAbilitiesDailyHandler
{
public:
	CAbilitiesDailyHandler()
	{
	}
	void Zero()
	{
		_abilities=NULL;
		_tAccum=0.0f;
		memset(_entries,0,sizeof(_entries));
	}

	struct Entry
	{
		Entry()
		{
			tp=LevelAbilityType_None;
			bHandled=FALSE;
		}
		LevelAbilityType tp;
		BOOL bHandled;
	};

	void Init(CLevelAbilities *abilities);

	void OnEnterLevel(){}
	void OnLeaveLevel();

	void StartDay();
	void Update(LevelTick dt);

	void SavePersist(CDataPacket &dp);
	void LoadPersist(CDataPacket &dp);

public:

	CLevelAbilities *_abilities;

	Entry _entries[4];

	float _tAccum;

};

class CLevelAbilities
{
public:
	DEFINE_CLASS(CLevelAbilities);
	CLevelAbilities()
	{
		Zero();
	}
	void Zero()
	{
		_player=NULL;
		memset(_abilities,0,sizeof(_abilities));
		_owner=NULL;
		_tUpdate=0;
		_bDirty=FALSE;
		_handlerDaily.Zero();
    }
	void Init(CLevelObj *owner,CLevelPlayer *player);
	void Clear();

	void OnEnterLevel();
	void OnLeaveLevel();

	CLevelObj *GetOwner()	{		return _owner;	}

	void SetDirty()	{		_bDirty=TRUE;	}
	void ClearDirty()	{		_bDirty=FALSE;	}
	BOOL IsDirty()	{		return _bDirty;	}
	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

	void Update();

	void HandleEvent(LevelEvent &e);
	void HandleStartDay();
	void HandleEndDay();

	CLevelAbility *GetAbility(LevelAbilityType tp);
	CLevelAbility *GetActiveAbility(LevelAbilityType tp);

	BOOL ApplyUpgrade(CLevelAbilityUpgrade *upgrade,LevelAwardSeed &seed);

	void CollectWpnInductionInfo();
	WpnIdcInfo &GetWpnIdcInfo()    {        return _infoWpnIdc;    }

protected:

	CLevelPlayer *_player;

	CAbilitiesDailyHandler _handlerDaily;

	CLevelAbility *_abilities[LevelAbilityType_Max];
	CLevelObj *_owner;
	LevelTick _tUpdate;

	WpnIdcInfo _infoWpnIdc;

	BOOL _bDirty;

};


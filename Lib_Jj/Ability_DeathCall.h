#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct DeathCallIdc_BloodTeeth
{

	BEGIN_GOBJ_PURE(DeathCallIdc_BloodTeeth,1);

		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("添加的Buff",GVT_S,GSem(GSem_RecordID,"buffs"),"添加的Buff");

		GELEM_VAR_INIT(float,percentBase,0.2f);
			GELEM_EDITVAR("基础回血百分比",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"基础回血百分比");
		GELEM_VAR_INIT(float,percentPerGrade,0.05f);
			GELEM_EDITVAR("每级(BloodTeeth等级)增加回血百分比",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"每级(BloodTeeth等级)增加回血百分比");

	END_GOBJ();

	RecordID idBuff;

	float percentBase;
	float percentPerGrade;


};

struct DeathCallIdc_FlameBlade
{
	RecordID idEo;
	float durBase;
	float durPerGrade;
	float dmgBase;
	float dmgPerGrade;

	BEGIN_GOBJ_PURE(DeathCallIdc_FlameBlade,1);

		GELEM_VAR_INIT(RecordID,idEo,RecordID_Invalid);
			GELEM_EDITVAR("创建EO",GVT_S,GSem(GSem_RecordID,"eos"),"创建EO");
		GELEM_VAR_INIT(float,durBase,2.0f);
			GELEM_EDITVAR("火焰基础持续时间",GVT_F,GSem(GSem_Float,"0.0,100.0,0.01"),"火焰基础持续时间");
		GELEM_VAR_INIT(float,durPerGrade,0.5f);
			GELEM_EDITVAR("每级(FlameBlade)增加火焰持续时间",GVT_F,GSem(GSem_Float,"0.0,100.0,0.01"),"每级(FlameBlade)增加火焰持续时间");
		GELEM_VAR_INIT(float,dmgBase,20.0f);
			GELEM_EDITVAR("火焰基础伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"火焰基础伤害");
		GELEM_VAR_INIT(float,dmgPerGrade,4.0f);
			GELEM_EDITVAR("每级(FlameBlade)增加火焰基础伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"每级(FlameBlade)增加火焰基础伤害");
	END_GOBJ();

};

struct DeathCallIdc_FlashSwing
{
	RecordID idSkill;
	float durBase;
	float durPerGrade;
	float dmgBase;
	float dmgPerGrade;

	BEGIN_GOBJ_PURE(DeathCallIdc_FlashSwing,1);

		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_VAR_INIT(float,durBase,2.0f);
			GELEM_EDITVAR("刀光持续时间",GVT_F,GSem(GSem_Float,"0.0,100.0,0.01"),"刀光持续时间");
		GELEM_VAR_INIT(float,durPerGrade,0.5f);
			GELEM_EDITVAR("每级(FlashSwing)增加刀光持续时间",GVT_F,GSem(GSem_Float,"0.0,100.0,0.01"),"每级(FlashSwing)增加刀光持续时间");
		GELEM_VAR_INIT(float,dmgBase,20.0f);
			GELEM_EDITVAR("刀光基础伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"刀光基础伤害");
		GELEM_VAR_INIT(float,dmgPerGrade,4.0f);
			GELEM_EDITVAR("每级(FlashSwing)增加刀光基础伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"每级(FlameBlade)增加刀光基础伤害");
	END_GOBJ();

};

struct DeathCallIdc_LightningBow
{
	CLevelDeal *deal; 
	float durBase;
	float durPerGrade;
	float rangeBase;
	float rangePerGrade;
	float dmgBase;
	float dmgPerGrade;

	BEGIN_GOBJ_PURE(DeathCallIdc_LightningBow,1);

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_CreateEo, "结算", "选择不同的技能结算" );
			GELEM_DYNOBJPTR_CLASS_DEAL( "创建Eo", Deal_CreateEo);
		GELEM_VAR_INIT(float,durBase,5.0f);
			GELEM_EDITVAR("闪电持续时间",GVT_F,GSem(GSem_Float,"0.0,100.0,0.01"),"闪电持续时间");
		GELEM_VAR_INIT(float,durPerGrade,0.5f);
			GELEM_EDITVAR("每级(LightningBow)增加闪电持续时间",GVT_F,GSem(GSem_Float,"0.0,100.0,0.01"),"每级(LightningBow)增加闪电持续时间");
		GELEM_VAR_INIT(float,rangeBase,3.0f);
			GELEM_EDITVAR("基础闪电范围",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"基础闪电范围");
		GELEM_VAR_INIT(float,rangePerGrade,0.2f);
			GELEM_EDITVAR("每级(LightningBow)增加闪电范围",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"每级(LightningBow)增加闪电范围");
		GELEM_VAR_INIT(float,dmgBase,10.0f);
			GELEM_EDITVAR("闪电基础伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"闪电基础伤害");
		GELEM_VAR_INIT(float,dmgPerGrade,4.0f);
			GELEM_EDITVAR("每级(LightningBow)增加闪电伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"每级(LightningBow)增加闪电伤害");
	END_GOBJ();

};

struct DeathCallIdc_ObliterateBow
{
	RecordID idSummon;
	RecordID idBirth;
	RecordID idDie;
	CLevelDeal *dealShoot; 

	BEGIN_GOBJ_PURE(DeathCallIdc_ObliterateBow,1);

		GELEM_VAR_INIT(RecordID,idSummon,RecordID_Invalid);
			GELEM_EDITVAR("召唤单位ID",GVT_S,GSem(GSem_RecordID,"units"),"召唤单位ID");
		GELEM_VAR_INIT(RecordID,idBirth,RecordID_Invalid);
			GELEM_EDITVAR("召唤单位出生Buff",GVT_S,GSem(GSem_RecordID,"buffs"),"召唤单位出生Buff");
		GELEM_VAR_INIT(RecordID,idDie,RecordID_Invalid);
			GELEM_EDITVAR("召唤单位死亡Buff",GVT_S,GSem(GSem_RecordID,"buffs"),"召唤单位死亡Buff");
		GELEM_DYNOBJPTR_DEAL(CLevelDeal,dealShoot,Deal_CreateEo, "发射飞箭的Deal", "发射飞箭的Deal" );
			GELEM_DYNOBJPTR_CLASS_DEAL( "创建Eo", Deal_CreateEo);

	END_GOBJ();

};


struct DeathCallIdc_TeleportSword
{
	RecordID idEo;
	float durDizzyBase;
	float durDizzyPerGrade;
	float dmgBase;
	float dmgPerGrade;

	BEGIN_GOBJ_PURE(DeathCallIdc_TeleportSword,1);

		GELEM_VAR_INIT(RecordID,idEo,RecordID_Invalid);
			GELEM_EDITVAR("创建EO",GVT_S,GSem(GSem_RecordID,"eos"),"创建EO");
		GELEM_VAR_INIT(float,durDizzyBase,2.0f);
			GELEM_EDITVAR("基础眩晕时间",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"基础眩晕时间");
		GELEM_VAR_INIT(float,durDizzyPerGrade,0.5f);
			GELEM_EDITVAR("每级(TeleportSword)增加眩晕时间",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"每级(TeleportSword)增加眩晕时间");
		GELEM_VAR_INIT(float,dmgBase,10.0f);
			GELEM_EDITVAR("基础伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"基础震荡波伤害值");
		GELEM_VAR_INIT(float,dmgPerGrade,4.0f);
			GELEM_EDITVAR("每级(TeleportSword)增加基础伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"每级(TeleportSword)增加基础伤害");
	END_GOBJ();

};



class CUpgradeDeathCall_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeDeathCall_Init,LevelAbilityType_DeathCall);

	BEGIN_GOBJ_PURE(CUpgradeDeathCall_Init,1);
		GELEM_OBJ(AbilityActionSettings,settings);
			GELEM_EDITOBJ("Action参数","Action参数");

		GELEM_OBJ(AbilityAttackSetting,attackNormal);
			GELEM_EDITOBJ("普通攻击参数","普通攻击参数");
		GELEM_OBJ(AbilityAttackSetting,attackLethal);
			GELEM_EDITOBJ("致命攻击参数","攻击参数");
		GELEM_OBJ(LevelUpgradableValue,rateStar);
			GELEM_EDITOBJ("Star几率(术能等级)","Star几率");

		GELEM_OBJ(DeathCallIdc_BloodTeeth,idcBloodTeeth);
			GELEM_EDITOBJ("IDC(血牙剑)","武器感应参数(血牙剑)");
		GELEM_OBJ(DeathCallIdc_FlameBlade,idcFlameBlade);
			GELEM_EDITOBJ("IDC(火焰刀)","武器感应参数(火焰刀)");
		GELEM_OBJ(DeathCallIdc_FlashSwing,idcFlashSwing);
			GELEM_EDITOBJ("IDC(闪羽剑)","武器感应参数(闪羽剑)");
		GELEM_OBJ(DeathCallIdc_LightningBow,idcLightningBow);
			GELEM_EDITOBJ("IDC(闪电弓)","武器感应参数(闪电弓)");
		GELEM_OBJ(DeathCallIdc_ObliterateBow,idcObliterateBow);
			GELEM_EDITOBJ("IDC(尸爆弓)","武器感应参数(尸爆弓)");
		GELEM_OBJ(DeathCallIdc_TeleportSword,idcTeleportSword);
			GELEM_EDITOBJ("IDC(闪动剑)","武器感应参数(闪动剑)");

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	AbilityActionSettings settings;

	AbilityAttackSetting attackNormal;
	AbilityAttackSetting attackLethal;
	LevelUpgradableValue rateStar;

	DeathCallIdc_BloodTeeth idcBloodTeeth;
	DeathCallIdc_FlameBlade idcFlameBlade;
	DeathCallIdc_FlashSwing idcFlashSwing;
	DeathCallIdc_LightningBow idcLightningBow;
	DeathCallIdc_ObliterateBow idcObliterateBow;
	DeathCallIdc_TeleportSword idcTeleportSword;
};

class CUpgradeDeathCall_LevelUp:public CLevelAbilityUpgrade_LevelUp
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeDeathCall_LevelUp,LevelAbilityType_DeathCall);

	BEGIN_GOBJ_PURE(CUpgradeDeathCall_LevelUp,1);
	END_GOBJ();
};


struct LeModDamageAttr;
struct LeKill;
struct LevelRecordSkill;
struct LePreDamage;
struct LePostCreateEo;
class CLevelAbility_DeathCall:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_DeathCall,CUpgradeDeathCall_Init,LevelAbilityType_DeathCall);
	
	BEGIN_GOBJ_PURE_UID(CLevelAbility_DeathCall,1);

		GELEM_ABILITY_BASE();

		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);GELEM_UID(1)
		GELEM_VAR_INIT(RecordID,_idDefSkill,RecordID_Invalid);GELEM_UID(2)
		GELEM_VAR_INIT(int,_nStars,0);GELEM_UID(3)
		GELEM_VAR_INIT(LevelTick,_tLightningAccum,0);GELEM_UID(4)
		GELEM_VAR_INIT(int,_nLightning,0);GELEM_UID(5)

	END_GOBJ();

	int GetStarCount()	{		return _nStars;	}

public:

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;


public://Take it as protected

	virtual void _InitTechs()	 override{	}

	virtual void _OnEvent(LevelEvent &e) override;

	BOOL _BuildSkillRT_FlashSwing();

	BOOL _Update_LightningBow(AnimTick dt);
	BOOL _Update_ObliterateBow(AnimTick dt);

	BOOL _MakeModDmg_Default(BOOL bDefaultSkill,LeModDamageAttr &e,LevelOSB &osbSrc,CLevelObj *loTarget);
	BOOL _MakeDmg_Default(BOOL bDefaultSkill,LevelOSB &osbSrc,CLevelObj *loTarget,int nDmg,LevelStrike *strike,LevelOpLink &link);
	BOOL _MakeKill_BloodTeeth(BOOL bDefaultSkill,LeKill &e);
	BOOL _MakeKill_FlameBlade(BOOL bDefaultSkill,LeKill &e);
	BOOL _MakePreDmg_FlashSwing(LePreDamage &e);
	BOOL _MakePostCreateEo_FlashSwing(LePostCreateEo &e);
	BOOL _MakeKill_ObliterateBow(BOOL bDefaultSkill,LeKill &e);
	BOOL _MakeKill_TeleportSword(BOOL bDefaultSkill,LeKill &e);

	void _MakeDmg(BOOL bDefaultSkill,LevelOSB &osbSrc,CLevelObj *loTarget,int nDmg,LevelStrike *strike,LevelOpLink &link);
	void _MakeModDmg(BOOL bDefaultSkill,LeModDamageAttr &e,LevelOSB &osbSrc,CLevelObj *loTarget);
	void _MakeKill(BOOL bDefaultSkill,LeKill &e);

	RecordID _idSkill;
	RecordID _idDefSkill;

	void _DecStar();
	int _nStars;

	void _FlushFlashSwingEos();
	std::unordered_map<LevelObjID,AnimTick> _eosFlashSwing;

	AnimTick _tLightningAccum;
	int _nLightning;

	struct ObliterateBowEoEntry
	{
		ObliterateBowEoEntry()
		{
			memset(this,0,sizeof(*this));
		}
		LevelObjID id;
		int nDmgPerShot;
		int nShots;
		int nToShots;
		AnimTick tRecentShot;
	};
	std::deque<ObliterateBowEoEntry> _entriesObliterateBow;

};


#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "valueset/valueset.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

#include "math/range.h"

struct BloodTeethIdc_DeathCall
{
	RecordID idSummon;//加MaxHP的血球
	float possibilityBase;
	float possibilityUpgrade;
	float mulIdcGrd02;
	float mulIdcGrd03;

	BEGIN_GOBJ_PURE(BloodTeethIdc_DeathCall,1);

		GELEM_VAR_INIT(RecordID,idSummon,RecordID_Invalid);
			GELEM_EDITVAR("召唤单位ID",GVT_S,GSem(GSem_RecordID,"units"),"召唤单位ID");
		GELEM_VAR_INIT(float,possibilityBase,0.005f);
			GELEM_EDITVAR("基础几率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"基础几率");
		GELEM_VAR_INIT(float,possibilityUpgrade,0.002f);
			GELEM_EDITVAR("每级增加的几率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"每级增加的几率");
		GELEM_VAR_INIT(float,mulIdcGrd02,1.4f);
			GELEM_EDITVAR("感应等级2时几率倍率",GVT_F,GSem(GSem_Float,"1.0,19.0,0.01"),"感应等级2时几率倍率");
		GELEM_VAR_INIT(float,mulIdcGrd03,2.2f);
			GELEM_EDITVAR("感应等级3时几率倍率",GVT_F,GSem(GSem_Float,"1.0,19.0,0.01"),"感应等级3时几率倍率");
	END_GOBJ();

};

class Deal_BloodTeethFireBlood:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_BloodTeethFireBlood);


	BEGIN_GOBJ_PURE(Deal_BloodTeethFireBlood,1);

		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");

	END_GOBJ();

	RecordID idBuff;

	virtual void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg);

};


struct BloodTeethIdc_FlameBlade
{
	RecordID idSummon;
	float rateBase;
	float ratePerGrade;
	float durBase;
	float durPerHP;

	BEGIN_GOBJ_PURE(BloodTeethIdc_FlameBlade,1);

		GELEM_VAR_INIT(RecordID,idSummon,RecordID_Invalid);
			GELEM_EDITVAR("召唤单位ID",GVT_S,GSem(GSem_RecordID,"units"),"召唤单位ID");
		GELEM_VAR_INIT(float,rateBase,0.4f);
			GELEM_EDITVAR("基础火球几率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"基础燃烧时间");
		GELEM_VAR_INIT(float,ratePerGrade,0.1f);
			GELEM_EDITVAR("每级(FlameBlade等级)增加火球几率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"每级(FlameBlade等级)增加火球几率");
		GELEM_VAR_INIT(float,durBase,0.2f);
			GELEM_EDITVAR("基础燃烧时间",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"基础燃烧时间");
		GELEM_VAR_INIT(float,durPerHP,0.01f);
			GELEM_EDITVAR("每点HP增加燃烧时间",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"每点HP增加燃烧时间");
	END_GOBJ();

};


struct BloodTeethIdc_FlashSwing
{
	RecordID idSkill;
	float rangeBase;
	float rangePerGrade;
	int dmgBaseMin;
	int dmgBaseMax;
	int dmgPerGrade;

	BEGIN_GOBJ_PURE(BloodTeethIdc_FlashSwing,1);

		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_VAR_INIT(float,rangeBase,1.5f);
			GELEM_EDITVAR("基础攻击范围",GVT_F,GSem(GSem_Float,"0.0,100.0,0.1"),"基础范围");
		GELEM_VAR_INIT(float,rangePerGrade,0.2f);
			GELEM_EDITVAR("每级增加攻击范围",GVT_F,GSem(GSem_Float,"0.0,10.0,0.1"),"每级(FlashSwing等级)增加攻击范围");
		GELEM_VAR_INIT(int,dmgBaseMin,30);
			GELEM_EDITVAR("基础伤害(Min)",GVT_S,GSem_Interger,"基础伤害");
		GELEM_VAR_INIT(int,dmgBaseMax,35);
			GELEM_EDITVAR("基础伤害(Max)",GVT_S,GSem_Interger,"基础伤害");
		GELEM_VAR_INIT(int,dmgPerGrade,5);
			GELEM_EDITVAR("每级增加伤害",GVT_S,GSem_Interger,"每级(BloodTeeth)增加伤害");
	END_GOBJ();

};

class Deal_BloodTeethLightningBlood:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_BloodTeethLightningBlood);


	BEGIN_GOBJ_PURE(Deal_BloodTeethLightningBlood,1);

	END_GOBJ();

	virtual void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg);

};


struct BloodTeethIdc_LightingBow
{
	RecordID idSummon;
	float rateBase;
	float ratePerGrade;
	float dmgBase;
	float dmgPerHP;

	BEGIN_GOBJ_PURE(BloodTeethIdc_LightingBow,1);

		GELEM_VAR_INIT(RecordID,idSummon,RecordID_Invalid);
			GELEM_EDITVAR("召唤单位ID",GVT_S,GSem(GSem_RecordID,"units"),"召唤单位ID");
		GELEM_VAR_INIT(float,rateBase,0.4f);
			GELEM_EDITVAR("基础电球几率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"基础电球几率");
		GELEM_VAR_INIT(float,ratePerGrade,0.1f);
			GELEM_EDITVAR("每级(LightningBow等级)增加电球几率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"每级(LightningBow等级)增加电球几率");
		GELEM_VAR_INIT(float,dmgBase,10.0f);
			GELEM_EDITVAR("基础电球伤害",GVT_F,GSem(GSem_Float,"0.0,200.0,0.1"),"基础电球伤害");
		GELEM_VAR_INIT(float,dmgPerHP,1.0f);
			GELEM_EDITVAR("每点HP增加电球伤害",GVT_F,GSem(GSem_Float,"0.0,100.0,0.1"),"每点HP增加电球伤害");
	END_GOBJ();

};


struct BloodTeethIdc_ObliterateBow
{
	RecordID idSummon;
	float rateBase;
	float ratePerGrade;
	float percentBase;
	float percentPerGrade;

	BEGIN_GOBJ_PURE(BloodTeethIdc_ObliterateBow,1);

		GELEM_VAR_INIT(RecordID,idSummon,RecordID_Invalid);
			GELEM_EDITVAR("召唤单位ID",GVT_S,GSem(GSem_RecordID,"units"),"召唤单位ID");
		GELEM_VAR_INIT(float,rateBase,0.4f);
			GELEM_EDITVAR("基础体力球几率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"基础体力球几率");
		GELEM_VAR_INIT(float,ratePerGrade,0.1f);
			GELEM_EDITVAR("每级(ObliterateBow)增加体力球几率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"每级(ObliterateBow)增加体力球几率");
		GELEM_VAR_INIT(float,percentBase,10.0f);
			GELEM_EDITVAR("基础转化百分比",GVT_F,GSem(GSem_Float,"0.0,100.0,0.1"),"基础HP转SP百分比");
		GELEM_VAR_INIT(float,percentPerGrade,1.0f);
			GELEM_EDITVAR("每级(感应石等级)增加转化百分比",GVT_F,GSem(GSem_Float,"0.0,100.0,0.1"),"每级(感应石等级)增加转化百分比");
	END_GOBJ();

};

struct BloodTeethIdc_TeleportSword
{
	RecordID idEo;
	float rateBase;
	float ratePerGrade;
	float dmgBase;
	float dmgPerGrade;

	BEGIN_GOBJ_PURE(BloodTeethIdc_TeleportSword,1);

		GELEM_VAR_INIT(RecordID,idEo,RecordID_Invalid);
			GELEM_EDITVAR("创建EO",GVT_S,GSem(GSem_RecordID,"eos"),"创建EO");
		GELEM_VAR_INIT(float,rateBase,0.4f);
			GELEM_EDITVAR("基础几率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"基础震荡波几率");
		GELEM_VAR_INIT(float,ratePerGrade,0.1f);
			GELEM_EDITVAR("每级(TeleportSword)增加几率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.0001"),"每级(TeleportSword)增加震荡波几率");
		GELEM_VAR_INIT(float,dmgBase,10.0f);
			GELEM_EDITVAR("基础伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"基础震荡波伤害值");
		GELEM_VAR_INIT(float,dmgPerGrade,4.0f);
			GELEM_EDITVAR("每级(TeleportSword)增加基础伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"每级(TeleportSword)增加基础伤害");
	END_GOBJ();

};



class CUpgradeBloodTeeth_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeBloodTeeth_Init,LevelAbilityType_BloodTeeth);

	CUpgradeBloodTeeth_Init()
	{
		GConstructor();
	}
	~CUpgradeBloodTeeth_Init()
	{
		GDestructor();
	}

	BEGIN_GOBJ(CUpgradeBloodTeeth_Init,1);
		GELEM_VAR_INIT(RecordID,idDefaultSkill,RecordID_Invalid);
			GELEM_EDITVAR("缺省(左键)技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_VAR_INIT(RecordID,idSummon,RecordID_Invalid);
			GELEM_EDITVAR("召唤单位ID",GVT_S,GSem(GSem_RecordID,"units"),"召唤单位ID");
		GELEM_VAR_INIT(RecordID,idBirth,RecordID_Invalid);
			GELEM_EDITVAR("召唤单位出生Buff",GVT_S,GSem(GSem_RecordID,"buffs"),"召唤单位出生Buff");

		GELEM_OBJ(AbilityAttackSetting,attackNormal);
			GELEM_EDITOBJ("攻击参数","攻击参数");

		GELEM_OBJ(BloodTeethIdc_DeathCall,idcDeathCall);
			GELEM_EDITOBJ("IDC(死亡召唤)","武器感应参数(死亡召唤)");
		GELEM_OBJ(BloodTeethIdc_FlameBlade,idcFlameBlade);
			GELEM_EDITOBJ("IDC(火焰刀)","武器感应参数(火焰刀)");
		GELEM_OBJ(BloodTeethIdc_FlashSwing,idcFlashSwing);
			GELEM_EDITOBJ("IDC(闪羽剑)","武器感应参数(闪羽剑)");
		GELEM_OBJ(BloodTeethIdc_LightingBow,idcLightningBow);
			GELEM_EDITOBJ("IDC(闪电弓)","武器感应参数(闪电弓)");
		GELEM_OBJ(BloodTeethIdc_ObliterateBow,idcObliterateBow);
			GELEM_EDITOBJ("IDC(尸爆弓)","武器感应参数(尸爆弓)");
		GELEM_OBJ(BloodTeethIdc_TeleportSword,idcTeleportSword);
			GELEM_EDITOBJ("IDC(闪动剑)","武器感应参数(闪动剑)");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	RecordID idSkill;
	RecordID idDefaultSkill;

	//召唤的不同的血球
	RecordID idSummon;//普通血球

	RecordID idBirth;

	AbilityAttackSetting attackNormal;

	BloodTeethIdc_DeathCall idcDeathCall;
	BloodTeethIdc_FlameBlade idcFlameBlade;
	BloodTeethIdc_FlashSwing idcFlashSwing;
	BloodTeethIdc_LightingBow idcLightningBow;
	BloodTeethIdc_ObliterateBow idcObliterateBow;
	BloodTeethIdc_TeleportSword idcTeleportSword;

};


class CUpgradeBloodTeeth_LevelUp:public CLevelAbilityUpgrade_LevelUp
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeBloodTeeth_LevelUp,LevelAbilityType_BloodTeeth);

	BEGIN_GOBJ_PURE(CUpgradeBloodTeeth_LevelUp,1);
	END_GOBJ();

};



struct LevelRecordSkill;
class CLevelAbility_BloodTeeth:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_BloodTeeth,CUpgradeBloodTeeth_Init,LevelAbilityType_BloodTeeth);

	BEGIN_GOBJ_PURE_UID(CLevelAbility_BloodTeeth,1);

		GELEM_ABILITY_BASE();

	END_GOBJ();


	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;

public://Take it as protected

	virtual void _OnEvent(LevelEvent &e) override;

	BOOL _BuildSkillRT_FlashSwing();

	void _MakeBlood(LevelOSB &osbSrc,CLevelObj *loTarget,int hp,LevelStrike *strike,LevelOpLink &link);
	void _MakeDmgBlood(LevelOSB &osbSrc,CLevelObj *loTarget,int nDmg,LevelStrike *strike,LevelOpLink &link);
	void _MakeKillBlood(LevelOSB &osbSrc,CLevelObj *loTarget,LevelStrike *strike,LevelOpLink &link);
	void _MakeDmg(LevelOSB &osbSrc,CLevelObj *loTarget,int nDmg,LevelStrike *strike,LevelOpLink &link);

	BOOL _MakeBlood_DeathCall(LevelOSB &osbSrc,CLevelObj *loTarget,int hp,LevelStrike *strike,LevelOpLink &link);
	BOOL _MakeBlood_FlameBlade(LevelOSB &osbSrc,CLevelObj *loTarget,int hp,LevelStrike *strike,LevelOpLink &link);
	BOOL _MakeBlood_LightningBow(LevelOSB &osbSrc,CLevelObj *loTarget,int hp,LevelStrike *strike,LevelOpLink &link);
	BOOL _MakeBlood_ObliterateBow(LevelOSB &osbSrc,CLevelObj *loTarget,LevelStrike *strike,LevelOpLink &link);
	BOOL _MakeDmg_TeleportSword(LevelOSB &osbSrc,CLevelObj *loTarget,LevelStrike *strike,LevelOpLink &link);

	void _MakeBloodBirth(LevelOSB &osbSrc,CLevelObj *lo,LevelPos3D &pos,LevelStrike *strike,LevelOpLink &link);

	BOOL _MakeModDmg_Default(BOOL bDefaultSkill,LeModDamageAttr &e);
	void _MakeModDmg(BOOL bDefaultSkill,LeModDamageAttr &e);

	virtual void _InitTechs();

	int _RollCount(float rate);

};


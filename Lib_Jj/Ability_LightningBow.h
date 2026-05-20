#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

#include "Tech_Fury.h"

struct LightningBowIdc_BloodTeeth
{

	BEGIN_GOBJ_PURE(LightningBowIdc_BloodTeeth,1);

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,dealShoot,Deal_CreateEo, "发射血牙的Deal", "发射血牙的Deal" );
			GELEM_DYNOBJPTR_CLASS_DEAL( "创建Eo", Deal_CreateEo);

		GELEM_VAR_INIT(float,countBase,1.0f);
			GELEM_EDITVAR("血牙基础个数",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"血牙基础个数");
		GELEM_VAR_INIT(float,countPerGrade,0.5f);
			GELEM_EDITVAR("每级(BloodTeeth)增加血牙个数",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"每级(BloodTeeth)增加血牙个数");

		GELEM_VAR_INIT(float,dmgBase,10.0f);
			GELEM_EDITVAR("血牙基础伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"血牙基础伤害");
		GELEM_VAR_INIT(float,dmgPerGrade,4.0f);
			GELEM_EDITVAR("每级(BloodTeeth)增加血牙伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"每级(BloodTeeth)增加血牙伤害");

	END_GOBJ();

	CLevelDeal *dealShoot; 

	float countBase;
	float countPerGrade;

	float dmgBase;
	float dmgPerGrade;


};

struct LightningBow_SacredArrow
{

	BEGIN_GOBJ_PURE(LightningBow_SacredArrow,1);

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_CreateEo, "子弹Eo", "子弹Eo" );
			GELEM_DYNOBJPTR_CLASS_DEAL( "创建Eo", Deal_CreateEo);

		GELEM_OBJ(AbilityAttackSetting,attack);
			GELEM_EDITOBJ("攻击参数","攻击参数");

	END_GOBJ();

	CLevelDeal *deal; 

	AbilityAttackSetting attack;

};


struct LevelRecordSkill;
class CUpgradeLightningBow_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeLightningBow_Init,LevelAbilityType_LightningBow);

	BEGIN_GOBJ_PURE(CUpgradeLightningBow_Init,1);

		GELEM_OBJ(AbilityActionSettings,settings);
			GELEM_EDITOBJ("Action参数","Action参数");

		GELEM_VARVECTOR_INIT(RecordID,idsBullet,RecordID_Invalid);
			GELEM_EDITVAR("所有弓射出的箭",GVT_S,GSem(GSem_RecordID,"eos"),"所有弓射出的箭");

		GELEM_OBJ(AbilityAttackSetting,attack);
			GELEM_EDITOBJ("攻击参数","攻击参数");

		GELEM_OBJ(LightningBow_SacredArrow,infoSacredArrow);
			GELEM_EDITOBJ("Info(圣箭)","参数(圣箭)");
		GELEM_OBJ(LightningBowIdc_BloodTeeth,idcBloodTeeth);
			GELEM_EDITOBJ("IDC(血牙剑)","武器感应参数(血牙剑)");

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	AbilityActionSettings settings;

	std::vector<RecordID> idsBullet;

	AbilityAttackSetting attack;

	LightningBow_SacredArrow infoSacredArrow;
	LightningBowIdc_BloodTeeth idcBloodTeeth;

};


class CUpgradeLightningBow_LevelUp:public CLevelAbilityUpgrade_LevelUp
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeLightningBow_LevelUp,LevelAbilityType_LightningBow);

	BEGIN_GOBJ_PURE(CUpgradeLightningBow_LevelUp,1);
	END_GOBJ();

};

struct LeDamage;
class CLevelAbility_LightningBow:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_LightningBow,CUpgradeLightningBow_Init,LevelAbilityType_LightningBow);

	BEGIN_GOBJ_PURE_UID(CLevelAbility_LightningBow,1);

		GELEM_ABILITY_BASE();

	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;
	virtual void _OnUpdate(LevelTick dt) override;

	virtual void _OnEvent(LevelEvent &e) override;

public://Take it as protected

	virtual void _InitTechs();

	BOOL _MakeModDmg_Default(LeModDamageAttr &e);

	BOOL _MakeCast_BloodTeeth(LePostCreateEo &e);

	void _MakeCast(LePostCreateEo &e);

	BOOL _PreCreateBullet_SacredArrow(LePreCreateEo &e);

	void _MakeDmg(LeDamage &e);
	void _MakeModDmg(LeModDamageAttr &e);

	BOOL _PreCreateBullet(LePreCreateEo &e);

	BOOL _CheckArrow(CLevelObj *lo);

};



#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "valueset/valueset.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

#include "math/range.h"

struct LevelRecordSkill;

struct ObliterateBowIdc_BloodTeeth
{

	BEGIN_GOBJ_PURE(ObliterateBowIdc_BloodTeeth,1);

		GELEM_VAR_INIT(RecordID,idEo,RecordID_Invalid);
			GELEM_EDITVAR("创建EO",GVT_S,GSem(GSem_RecordID,"eos"),"创建EO");

		GELEM_VAR_INIT(float,countBase,3.0f);
			GELEM_EDITVAR("血牙基础个数",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"血牙基础伤害");
		GELEM_VAR_INIT(float,countPerGrade,0.5f);
			GELEM_EDITVAR("每级(BloodTeeth)增加血牙个数",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"每级(BloodTeeth)增加血牙个数");

		GELEM_VAR_INIT(float,dmgBase,10.0f);
			GELEM_EDITVAR("血牙基础伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"血牙基础伤害");
		GELEM_VAR_INIT(float,dmgPerGrade,4.0f);
			GELEM_EDITVAR("每级(BloodTeeth)增加血牙伤害",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"每级(BloodTeeth)增加血牙伤害");

	END_GOBJ();

	RecordID idEo;

	float countBase;
	float countPerGrade;

	float dmgBase;
	float dmgPerGrade;

};

struct ObliterateBowIdc_DeathCall
{

	BEGIN_GOBJ_PURE(ObliterateBowIdc_DeathCall,1);

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_CreateEo, "结算", "选择不同的技能结算" );
			GELEM_DYNOBJPTR_CLASS_DEAL( "创建Eo", Deal_CreateEo);

		GELEM_OBJ(AbilityAttackSetting,attack);
			GELEM_EDITOBJ("死亡之箭攻击参数","死亡之箭攻击参数");

		GELEM_VAR_INIT(float,rateBase,0.1f);
			GELEM_EDITVAR("死亡之箭基础几率",GVT_F,GSem(GSem_Float,"0.0,1000.0,0.01"),"基础几率");
		GELEM_VAR_INIT(float,ratePerGrade,0.03f);
			GELEM_EDITVAR("每级(DeathCall)增加死亡之箭几率",GVT_F,GSem(GSem_Float,"0.0,1,0.01"),"每级(DeathCall)增加死亡之箭几率");

	END_GOBJ();

	CLevelDeal *deal; 

	AbilityAttackSetting attack;

	float rateBase;
	float ratePerGrade;

};

struct ObliterateBowIdcGrdInfo_FlashSwing
{
	int count;
	float dur;
	float rateBaseDmg;

	BEGIN_GOBJ_PURE(ObliterateBowIdcGrdInfo_FlashSwing,1);

		GELEM_VAR_INIT(DWORD,count,3);
			GELEM_EDITVAR("攻击次数",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9"),"连续攻击的次数");
		GELEM_VAR_INIT(float,dur,1.2f);GELEM_VERSION(4)
			GELEM_EDITVAR("攻击时间",GVT_F,GSem(GSem_Float,"0,100,0.1"),"攻击持续时间");

		GELEM_VAR_INIT(float,rateBaseDmg,0.6f);
			GELEM_EDITVAR("基础伤害比率",GVT_F,GSem(GSem_Float,"0,100,0.1"),"基础伤害的比例");
	END_GOBJ();
};

struct ObliterateBowIdc_FlashSwing
{

	BEGIN_GOBJ_PURE(ObliterateBowIdc_FlashSwing,1);

		GELEM_OBJVECTOR(ObliterateBowIdcGrdInfo_FlashSwing,grdinfosIdc)
			GELEM_EDITOBJ("不同感应石等级的攻击参数","不同感应石等级的攻击参数");

		GELEM_VAR_INIT(float,ratePerGrade,0.03f);
			GELEM_EDITVAR("每级(FlashSwing)增加攻击倍率",GVT_F,GSem(GSem_Float,"0.0,10,0.01"),"每级(FlashSwing)增加攻击倍率");

	END_GOBJ();

	std::vector<ObliterateBowIdcGrdInfo_FlashSwing> grdinfosIdc;

	float ratePerGrade;

};

struct ObliterateBow_SacredArrow
{

	BEGIN_GOBJ_PURE(ObliterateBow_SacredArrow,1);

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_CreateEo, "子弹Eo", "子弹Eo" );
			GELEM_DYNOBJPTR_CLASS_DEAL( "创建Eo", Deal_CreateEo);

		GELEM_OBJ(AbilityAttackSetting,attack);
			GELEM_EDITOBJ("攻击参数","攻击参数");

	END_GOBJ();

	CLevelDeal *deal; 

	AbilityAttackSetting attack;

};



class CUpgradeObliterateBow_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeObliterateBow_Init,LevelAbilityType_ObliterateBow);

	CUpgradeObliterateBow_Init()
	{
		GConstructor();
	}
	~CUpgradeObliterateBow_Init()
	{
		GDestructor();
	}

	BEGIN_GOBJ(CUpgradeObliterateBow_Init,1);
		GELEM_VAR_INIT(RecordID,idDefaultSkill,RecordID_Invalid);
			GELEM_EDITVAR("缺省(左键)技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");


		GELEM_OBJ(AbilityAttackSetting,attackNormal);
			GELEM_EDITOBJ("攻击参数","攻击参数");

		GELEM_VARVECTOR_INIT(RecordID,idsBullet,RecordID_Invalid);
			GELEM_EDITVAR("会引爆尸体的子弹",GVT_S,GSem(GSem_RecordID,"eos"),"会引爆尸体的子弹");

		GELEM_OBJ(ObliterateBow_SacredArrow,infoSacredArrow);
			GELEM_EDITOBJ("Info(圣箭)","参数(圣箭)");
		GELEM_OBJ(ObliterateBowIdc_BloodTeeth,idcBloodTeeth);
			GELEM_EDITOBJ("IDC(血牙剑)","武器感应参数(血牙剑)");
		GELEM_OBJ(ObliterateBowIdc_DeathCall,idcDeathCall);
			GELEM_EDITOBJ("IDC(死亡召唤)","武器感应参数(死亡召唤)");
		GELEM_OBJ(ObliterateBowIdc_FlashSwing,idcFlashSwing);
			GELEM_EDITOBJ("IDC(斩羽剑)","武器感应参数(斩羽剑)");

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	RecordID idSkill;
	RecordID idDefaultSkill;
	std::vector<RecordID> idsBullet;
	AbilityAttackSetting attackNormal;

	ObliterateBow_SacredArrow infoSacredArrow;

	ObliterateBowIdc_BloodTeeth idcBloodTeeth;
	ObliterateBowIdc_DeathCall idcDeathCall;
	ObliterateBowIdc_FlashSwing idcFlashSwing;

};


class CUpgradeObliterateBow_LevelUp:public CLevelAbilityUpgrade_LevelUp
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeObliterateBow_LevelUp,LevelAbilityType_ObliterateBow);

	BEGIN_GOBJ_PURE(CUpgradeObliterateBow_LevelUp,1);
	END_GOBJ();

};


class CLevelAbility_ObliterateBow:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_ObliterateBow,CUpgradeObliterateBow_Init,LevelAbilityType_ObliterateBow);

	BEGIN_GOBJ_PURE_UID(CLevelAbility_ObliterateBow,1);

		GELEM_ABILITY_BASE();

		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);GELEM_UID(1)
		GELEM_VAR_INIT(RecordID,_idDefSkill,RecordID_Invalid);GELEM_UID(2)
	END_GOBJ();


	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;


public://Take it as protected

	virtual void _OnEvent(LevelEvent &e) override;


	virtual void _InitTechs();

	RecordID _idSkill;
	RecordID _idDefSkill;

	void _ApplyBulletDamageRate(LevelRecordSkill *rec,float rate);

	BOOL _BuildSkillRT_FlashSwing();
	BOOL _BuildSkillRT_Default();
	BOOL _MakeModDmg_Default(LeModDamageAttr &e);

	BOOL _MakeDmg_BloodTeeth(LeDamage &e);

	BOOL _PreCreateBullet_SacredArrow(LePreCreateEo &e);

	BOOL _PreCreateBullet_DeathCall(LePreCreateEo &e);
	BOOL _MakeModDmg_DeathCall(LeModDamageAttr &e);

	void _MakeDmg(LeDamage &e);
	void _MakeModDmg(LeModDamageAttr &e);

	BOOL _PreCreateBullet(LePreCreateEo &e);

	BOOL _CheckArrow(CLevelObj *lo);

};



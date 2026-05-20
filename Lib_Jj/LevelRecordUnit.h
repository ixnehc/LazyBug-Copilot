#pragma once

#include "math/range.h"

#include "class/class.h"
#include "gds/GObj.h"
#include "records/records.h"

#include "strlib/strlibdefines.h"

#include "anim/animdefines.h"

#include "unitmgr/UnitMgr.h"
#include "unitmgr/Unit3DMgr.h"

#include "LevelStrike.h"
#include "LevelSensor.h"
#include "LevelAttrs.h"
#include "LevelAttrs_Weak.h"
#include "LevelObjMiscFlags.h"

#include "LevelRecordAgent.h"

#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"

struct NakeEquips
{
	RecordID armor;
	RecordID glove;
	RecordID boot;
	RecordID head;

	BEGIN_GOBJ_PURE(NakeEquips,1);
		GELEM_VAR_INIT(RecordID,armor,RecordID_Invalid);
			GELEM_EDITVAR("身体",GVT_U,GSem(GSem_RecordID,"items"),"裸身装备--身体");
		GELEM_VAR_INIT(RecordID,glove,RecordID_Invalid);
			GELEM_EDITVAR("手部",GVT_U,GSem(GSem_RecordID,"items"),"裸身装备--手部");
		GELEM_VAR_INIT(RecordID,boot,RecordID_Invalid);
			GELEM_EDITVAR("脚部",GVT_U,GSem(GSem_RecordID,"items"),"裸身装备--脚部");
		GELEM_VAR_INIT(RecordID,head,RecordID_Invalid);
			GELEM_EDITVAR("头部",GVT_U,GSem(GSem_RecordID,"items"),"裸身装备--头部");
	END_GOBJ();
};

struct UnitActions
{
	std::vector<unsigned __int64> stuns;
	unsigned __int64 burning;
	unsigned __int64 cold;

	BEGIN_GOBJ_PURE(UnitActions,1);

		GELEM_VARVECTOR_INIT(unsigned __int64,stuns,0);
			GELEM_EDITVAR("受击打动作",GVT_Bx8,GSem_ProtoPath,"各种受击打的动作");
		GELEM_VAR_INIT(unsigned __int64,burning,0);
			GELEM_EDITVAR("燃烧效果",GVT_Bx8,GSem_ProtoPath,"燃烧效果");
		GELEM_VAR_INIT(unsigned __int64,cold,0);
			GELEM_EDITVAR("冰冻效果",GVT_Bx8,GSem_ProtoPath,"冰冻效果");

	END_GOBJ();

};

struct UnitBuffs
{
	BEGIN_GOBJ_PURE(UnitBuffs,1);

		GELEM_VAR_INIT(RecordID,dead,RecordID_Invalid);
			GELEM_EDITVAR("死亡Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位死亡时的Buff");
		GELEM_VAR_INIT(RecordID,stun,RecordID_Invalid);
			GELEM_EDITVAR("硬直Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位硬直时的Buff");
		GELEM_VAR_INIT(RecordID,kb,RecordID_Invalid);
			GELEM_EDITVAR("击退Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位击退时的Buff");
		GELEM_VAR_INIT(RecordID,bleed,RecordID_Invalid);
			GELEM_EDITVAR("流血Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位流血时的Buff");
		GELEM_VAR_INIT(RecordID,ash,RecordID_Invalid);
			GELEM_EDITVAR("化为灰烬Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位化为灰烬时的Buff");
		GELEM_VAR_INIT( StringID,jink,StringID_Invalid);	
			GELEM_EDITVAR( "快闪Buff添加器", GVT_U, GSem(GSem_StringID,"行为图中继名称"), "一个行为图中继的名称,用来执行一段逻辑添加快闪Buff" );
		GELEM_VAR_INIT( StringID,skillstun,StringID_Invalid);	
			GELEM_EDITVAR( "Skill硬直Buff添加器", GVT_U, GSem(GSem_StringID,"行为图中继名称"), "一个行为图中继的名称,用来执行一段逻辑添加Skill硬直" );

	END_GOBJ();

	RecordID dead;//死亡的Buff
	RecordID stun;//Stun的Buff
	RecordID kb;//KB的Buff
	RecordID bleed;//流血的Buff
	RecordID ash;//化为灰烬的Buff
	StringID jink;//快闪的Buff添加器
	StringID skillstun;//SkillStun的Buff添加器
	

};


struct DropRate
{
	float rateGold;
	float rateGem;
	float rateSoul;
	i_math::rangei amntSoul;
	i_math::rangei amntMP;
	float rateCrystal;
	i_math::rangei amntCrystal;
	float rateCrystalOrb;
	int amntHnr;

	BEGIN_GOBJ_PURE(DropRate,1);

		GELEM_VAR_INIT(float ,rateGold,0.05f);
			GELEM_EDITVAR("金粒掉落率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.005"),"金粒的掉落几率");
		GELEM_VAR_INIT(float ,rateGem,0.02f);
			GELEM_EDITVAR("宝石掉落率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.005"),"宝石的掉落几率");
		GELEM_VAR_INIT(float ,rateSoul,0.07f);
			GELEM_EDITVAR("魔血掉落率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.005"),"魔血的掉落几率");
		GELEM_VAR_INIT( i_math::rangei,amntSoul,i_math::rangei(1,2));
			GELEM_EDITVAR( "魔血掉落数量", GVT_Sx2,GSem_Range,"魔血掉落数量");
		GELEM_VAR_INIT(float ,rateCrystal,0.00f);
			GELEM_EDITVAR("魔晶掉落率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.00001"),"魔晶掉落率");
		GELEM_VAR_INIT( i_math::rangei,amntMP,i_math::rangei(1,5));
			GELEM_EDITVAR( "MP掉落数量", GVT_Sx2,GSem_Range,"MP掉落数量");
		GELEM_VAR_INIT( i_math::rangei,amntCrystal,i_math::rangei(0,0));GELEM_VERSION(2)
			GELEM_EDITVAR( "魔晶掉落数量(obsolete)", GVT_Sx2,GSem_Range,"魔晶掉落数量");
		GELEM_VAR_INIT(float ,rateCrystalOrb,0.00f);
			GELEM_EDITVAR("魔晶球掉落率(obsolete)",GVT_F,GSem(GSem_Float,"0.0,1.0,0.00001"),"魔晶球掉落率");
		GELEM_VAR_INIT(int,amntHnr,1);
			GELEM_EDITVAR("掉落Honor",GVT_S,GSem_Interger,"被玩家杀死后,玩家得到多少Honor");

	END_GOBJ();
};

//根据等级对各种属性的加成
struct GradeAddOn
{
	int HP;
	int SP;
	int Attack;
	int Armor;
	int Hit;
	int Dodge;

	float rateGold;
	float rateGem;

	BEGIN_GOBJ_PURE(GradeAddOn,1);

		GELEM_VAR_INIT(int,HP,100);
			GELEM_EDITVAR("最大生命值",GVT_S,GSem_Interger,"单位的最大生命值");
		GELEM_VAR_INIT(int,SP,1000);
			GELEM_EDITVAR("最大灵气值",GVT_S,GSem_Interger,"单位的最大灵气值");
		GELEM_VAR_INIT(int,Attack,10);
			GELEM_EDITVAR("攻击力",GVT_S,GSem_Interger,"单位的攻击力");
		GELEM_VAR_INIT(int,Armor,10);
			GELEM_EDITVAR("防御力",GVT_S,GSem_Interger,"单位的防御力");

		GELEM_VAR_INIT(int,Hit,10);
			GELEM_EDITVAR("命中",GVT_S,GSem_Interger,"单位的命中能力");
		GELEM_VAR_INIT(int,Dodge,10);
			GELEM_EDITVAR("闪避",GVT_S,GSem_Interger,"单位的闪避能力");

		GELEM_VAR_INIT(float ,rateGold,0.05f);
			GELEM_EDITVAR("金粒掉落率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.005"),"金粒的掉落几率");
		GELEM_VAR_INIT(float ,rateGem,0.02f);
			GELEM_EDITVAR("宝石掉落率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.005"),"宝石的掉落几率");

	END_GOBJ();


};

struct UnitAIs
{
	BOOL bSync;
	StringID nmBg;//野外时的AI

	BEGIN_GOBJ_PURE(UnitAIs,1);

		GELEM_VAR_INIT(BOOL,bSync,0);GELEM_UID(1);
			GELEM_EDITVAR("是否同步",GVT_S,GSem_Boolean,"Server端是否要将AI的状态变化同步给Client端");
		GELEM_VAR_INIT( StringID,nmBg,StringID_Invalid);	GELEM_UID(2);
			GELEM_EDITVAR( "单位AI", GVT_U, GSem(GSem_StringID,"行为图名称"), "作为野外单位时的AI" );

	END_GOBJ();

};

//单位在各种姿势下的缺省攻击技能,注意,顺序和LevelPostureType中的类型保持一致
struct UnitDefAttacks
{
	RecordID Unarmed;//徒手
	RecordID Shield;//盾
	RecordID ShortWpnWithShield;//短兵器+盾
	RecordID ShortWpn;//短兵器
	RecordID LongWpnWithShield;//长兵器+盾
	RecordID LongWpn;//长兵器
	RecordID TwoHand;//双手武器
	RecordID Bow;//弓
	RecordID Crossbow;//弩

	BEGIN_GOBJ_PURE(UnitDefAttacks,1);

		GELEM_VAR_INIT(RecordID,Unarmed,RecordID_Invalid);
			GELEM_EDITVAR("徒手",GVT_U,GSem(GSem_RecordID,"skills"),"徒手攻击技能");
		GELEM_VAR_INIT(RecordID,Shield,RecordID_Invalid);
			GELEM_EDITVAR("单手盾",GVT_U,GSem(GSem_RecordID,"skills"),"单手盾攻击技能");
		GELEM_VAR_INIT(RecordID,ShortWpnWithShield,RecordID_Invalid);
			GELEM_EDITVAR("短兵器+盾",GVT_U,GSem(GSem_RecordID,"skills"),"短兵器+盾 攻击技能");
		GELEM_VAR_INIT(RecordID,ShortWpn,RecordID_Invalid);
			GELEM_EDITVAR("短兵器",GVT_U,GSem(GSem_RecordID,"skills"),"短兵器攻击技能");
		GELEM_VAR_INIT(RecordID,LongWpnWithShield,RecordID_Invalid);
			GELEM_EDITVAR("长兵器+盾",GVT_U,GSem(GSem_RecordID,"skills"),"长兵器+盾 攻击技能");
		GELEM_VAR_INIT(RecordID,LongWpn,RecordID_Invalid);
			GELEM_EDITVAR("长兵器",GVT_U,GSem(GSem_RecordID,"skills"),"长兵器攻击技能");
		GELEM_VAR_INIT(RecordID,TwoHand,RecordID_Invalid);
			GELEM_EDITVAR("双手武器",GVT_U,GSem(GSem_RecordID,"skills"),"双手武器攻击技能");
		GELEM_VAR_INIT(RecordID,Bow,RecordID_Invalid);
			GELEM_EDITVAR("弓",GVT_U,GSem(GSem_RecordID,"skills"),"弓攻击技能");
		GELEM_VAR_INIT(RecordID,Crossbow,RecordID_Invalid);
			GELEM_EDITVAR("弩",GVT_U,GSem(GSem_RecordID,"skills"),"弩攻击技能");

	END_GOBJ();
	//XXXXX:more LevelPostureType

};

struct LevelCoSkillInfo
{
	RecordID idSkill;
	DWORD nFullCharge;
	BEGIN_GOBJ_PURE(LevelCoSkillInfo,1);
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("响应的技能",GVT_U,GSem(GSem_RecordID,"skills"),"响应的主角施放技能");
		GELEM_VAR_INIT(int,nFullCharge,1);
			GELEM_EDITVAR("满值",GVT_S,GSem_Interger,"Charge的满值,施放多少次技能达到协同技能的施放条件");
	END_GOBJ();
};

struct LevelUnitFly:public UnitFly
{
	BEGIN_GOBJ_PURE(LevelUnitFly,1);
		GELEM_VAR_INIT(BOOL,bEnable,FALSE);
			GELEM_EDITVAR("可用",GVT_S,GSem(GSem_Boolean,"飞行速度,飞行速度Blend,飞行转身速度,飞行高度,飞行高度浮动"),"是否可用");
		GELEM_VAR_INIT(float,speed,15.0f);
			GELEM_EDITVAR("飞行速度",GVT_F,GSem(GSem_Float,"0,100,0"),"单位的飞行速度");
		GELEM_VAR_INIT(float,blendSpeed,0.1f);
			GELEM_EDITVAR("飞行速度Blend",GVT_F,GSem(GSem_Float,"0,1,0.01"),"单位的飞行速度的平滑参数");
		GELEM_VAR_INIT(float,speedFace,180.0f);
			GELEM_EDITVAR("飞行转身速度",GVT_F,GSem(GSem_Float,"0,10000,0"),"单位的飞行转身速度(度/秒");
		GELEM_VAR_INIT(float,hang,8.0f);
			GELEM_EDITVAR("飞行高度",GVT_F,GSem(GSem_Float,"0,100,0"),"单位在什么高度飞行");
		GELEM_VAR_INIT(float,hangVary,1.0f);
			GELEM_EDITVAR("飞行高度浮动",GVT_F,GSem(GSem_Float,"0,100,0"),"单位飞行高度的浮动值");
	END_GOBJ();

	BOOL bEnable;

};

struct LevelUnitPace:public UnitPace
{
	LevelUnitPace()
	{
		GConstructor();
		startFw.ResetFloat(0.0f);
		startRot.ResetFloat(0.0f);
		stop.ResetFloat(0.0f);
	}
	~LevelUnitPace()
	{
		GDestructor();
	}
	BEGIN_GOBJ(LevelUnitPace,1);
		GELEM_VAR_INIT(BOOL,bEnable,FALSE);
			GELEM_EDITVAR("可用",GVT_S,GSem(GSem_Boolean,"允许起步控制,允许停步控制,移动旋转速度,Avoid旋转速度,原地转身时长,原地转身停顿时间,起步控制(前方),起步控制(转身),触发起步转身角度,停步控制,BigTurn角度"),"是否可用");

		GELEM_VAR_INIT(BOOL,bSupportStart,FALSE);
			GELEM_EDITVAR("允许起步控制",GVT_S,GSem(GSem_Boolean,"起步脚步(前方),起步脚步(左转),起步脚步(右转),起步控制(前方),起步控制(转身),触发起步转身角度"),"是否可用");
		GELEM_VAR_INIT(float,angleStartRot,45.0f);
			GELEM_EDITVAR("触发起步转身角度",GVT_F,GSem(GSem_Float,"0.0,180.0.0,0.05"),"起步时夹角大于多少触发转身起步");
		GELEM_OBJVAR( ValueSet,startFw);
			GELEM_EDITOBJ_EX( "起步控制(前方)", "起步控制(前方)",GSem( GSem_Unknown, "0,0,-1,-1") );
		GELEM_OBJVAR( ValueSet,startRot);
			GELEM_EDITOBJ_EX( "起步控制(转身)", "起步控制(转身)",GSem( GSem_Unknown, "0,0,-1,-1") );
		GELEM_VAR_INIT(BOOL,bSupportStop,FALSE);
			GELEM_EDITVAR("允许停步控制",GVT_S,GSem(GSem_Boolean,"停步控制"),"是否可用");
		GELEM_OBJVAR( ValueSet,stop);
			GELEM_EDITOBJ_EX( "停步控制", "停步控制",GSem( GSem_Unknown, "0,0,-1,-1") );

		GELEM_VAR_INIT(float,durROS,1.0f);
			GELEM_EDITVAR("原地转身时长",GVT_F,GSem(GSem_Float,"0.0,100.0,0.05"),"原地转身持续时间,0表示瞬间转身");
		GELEM_VAR_INIT(float,durROSWait,0.0f);
			GELEM_EDITVAR("原地转身停顿时间",GVT_F,GSem(GSem_Float,"0.0,100.0,0.05"),"原地转身结束后的停顿时间");

		GELEM_VAR_INIT(float,rotlimit,0.0f);
			GELEM_EDITVAR("移动旋转速度",GVT_F,GSem(GSem_Float,"0.0,10000.0,0.05"),"移动旋转速度,0表示无限大");
		GELEM_VAR_INIT(float,rotlimitAvoid,0.0f);
			GELEM_EDITVAR("Avoid旋转速度",GVT_F,GSem(GSem_Float,"0.0,10000.0,0.05"),"Avoid时的旋转速度,0表示无限大");
		GELEM_VAR_INIT(float,rotlimitBigTurn,0.0f);
			GELEM_EDITVAR("BigTurn角度",GVT_F,GSem(GSem_Float,"0.0,36000.0,0.05"),"一秒内转动角度大于多少度要停止移动,0表示不考虑");
	END_GOBJ();

	BOOL bEnable;
};


struct LevelSensorParamEx:public LevelSensorParam
{
	BOOL bEnable;

	BEGIN_GOBJ_PURE(LevelSensorParamEx,1);
		GELEM_VAR_INIT(BOOL,bEnable,FALSE);
			GELEM_EDITVAR("可用",GVT_S,GSem(GSem_Boolean,"感应范围,施放对象标志,施放对象的特定需求,侦测权重,使用主人的视野"),"是否可用");
		GELEM_VAR_INIT(float,range,10.0f);
			GELEM_EDITVAR("感应范围",GVT_F,GSem(GSem_Float,"0.0,10000.0,0.05"),"感应范围");
		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("施放对象标志",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"侦测什么类型的单位施放");
		GELEM_VARVECTOR_INIT(LevelObjRequire,requires,LevelObjRequire_Attackable);
			GELEM_EDITVAR("施放对象的特定需求",GVT_S,GSem(GSem_Interger,LevelObjRequire_SemConstraint),"施放对象的特定需求");
		GELEM_OBJ(LevelDetectWeights,weightsDetect)
			GELEM_EDITOBJ("侦测权重","侦测权重");
		GELEM_VAR_INIT(BOOL,bUseOwnerSight,TRUE);
			GELEM_EDITVAR("使用主人的视野",GVT_S,GSem_Boolean,"使用主人的视野");
		GELEM_VAR_INIT(BOOL,bDisableWhenCastingSkill,TRUE);
			GELEM_EDITVAR("施放技能时停止工作",GVT_S,GSem_Boolean,"施放技能时停止更新Threat");
	END_GOBJ();
};

//EyeOfQueen控制要求
struct LevelEoqDemand
{
	BOOL bAllowEoqControl;
	LevelEoqPower mean;
	LevelEoqPower deviation;

	BEGIN_GOBJ_PURE(LevelEoqDemand,1);
		GELEM_VAR_INIT(BOOL,bAllowEoqControl,FALSE);
			GELEM_EDITVAR("可被母虫之眼控制",GVT_S,GSem(GSem_Boolean,"均数,标准差"),"是否可用");
		GELEM_VAR_INIT(float,mean,10.0f);
			GELEM_EDITVAR("均数",GVT_F,GSem(GSem_Float,"0.0,10000.0,0.05"),"正态分布均数");
		GELEM_VAR_INIT(float,deviation,5.0f);
			GELEM_EDITVAR("标准差",GVT_F,GSem(GSem_Float,"0.0,10000.0,0.05"),"正态分布标准差");
	END_GOBJ();

};

struct UnitEquipSet
{
	std::vector<RecordID>equips;//装备 
	float weight;

	BEGIN_GOBJ_PURE(UnitEquipSet,1);
		GELEM_VAR_INIT(float,weight,1.0f);
			GELEM_EDITVAR("权重",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"权重");
		GELEM_VARVECTOR_INIT(RecordID,equips,RecordID_Invalid);
			GELEM_EDITVAR("装备",GVT_U,GSem(GSem_RecordID,"items"),"装备");
	END_GOBJ();
};

struct UnitEquipSets
{
	std::vector<UnitEquipSet> sets;

	BEGIN_GOBJ_PURE(UnitEquipSets,1);
		GELEM_OBJVECTOR(UnitEquipSet,sets);
			GELEM_EDITOBJ("装备集","装备集");
	END_GOBJ();

};

struct UnitPainInfo
{
	BOOL bEnable;
	int full;
	AnimTick durKeep;
	float speedDrop;
	float rateHPDmg;

	BEGIN_GOBJ_PURE(UnitPainInfo,1);
		GELEM_VAR_INIT(BOOL,bEnable,FALSE);
			GELEM_EDITVAR("是否有效",GVT_S,GSem_Boolean,"是否有效");
		GELEM_VAR_INIT(int,full,50);
			GELEM_EDITVAR("满值",GVT_S,GSem_Interger,"最大疼痛值");
		GELEM_VAR_INIT(AnimTick,durKeep,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("保持时间",GVT_U,GSem(GSem_AnimTick,"0,20,0.1"),"疼痛值上升后保持多久再开始下降");
		GELEM_VAR_INIT(float,speedDrop,10.0f);
			GELEM_EDITVAR("疼痛值下降速率",GVT_F,GSem(GSem_Float,"0.1,10000.0,0.5"),"疼痛值下降速率");
		GELEM_VAR_INIT(float,rateHPDmg,0.3f);
			GELEM_EDITVAR("扣血比例",GVT_F,GSem(GSem_Float,"0.01,1.0,0.01"),"疼痛值满后扣多少比例的HP");
	END_GOBJ();

};


struct LevelRecordUnit:public CRecord
{
	DEFINE_CLASS(LevelRecordUnit);

	std::string Name;
	unsigned __int64 Avtr;
	std::string ImpostorMesh;

	float score;

	int HP;
	int SP;
	float ExhaustedSP;
	AnimTick ExhaustedCountDown;
	UnitPainInfo pain;

	EvadeEx evade;
	ResistsEx resists;
	WeaksEx weaks;
// 	int Dmg;
// 	int Def;
// 	int Accu;
// 	int Evade;
// 
// 	int Stun;
// 	int StunResist;
// 	int CanStun;
// 	int LethalResist;

	LoUnitMiscFlags flagsMisc;

	float DetectRange;
	NakeEquips nake;
	float Speed;
	float Radius;
	float Height;
	DWORD layorCollide;
	float AimHeight;
	float CastHeight;
	float htMount;
	float scaleModel;
	AnimTick RaiseUpDur;
	BOOL bAdvWalkable;
	BOOL bCollar;
	BOOL bAggressive;
	BOOL bAllowKilledKB;
	BOOL bSpawnInvisibly;
	BOOL bSight;
	BOOL bKeepCorpseOnClient;
	BOOL bCanMove;
	std::vector<RecordID>Equips;//装备 
	UnitEquipSets EquipSets;//装备集
	std::vector<RecordID>Skills;
	UnitBuffs buffs;
	RecordID buffDead;//死亡的Buff
	UnitActions actions;
	LevelUnitFly fly;
	LevelUnitPace pace;
	LevelSensorParamEx sensor;
	LevelEoqDemand demandEoq;
	UnitAIs ais;
	BOOL bTalks;
	std::string iconTalk;

	std::string iconRtnu;//随从头像

	DropRate rateDrop;

	GradeAddOn addonGrade;
	RecordID attksDef[LevelPosture_Max];
	unsigned __int64 effectsStrike[16];

	std::vector<LevelDialogInfo> dlgs;

	std::vector<LevelCoSkillInfo> coskills;

	std::string pathMI;//Mini Icon
	std::string pathHilightMI;//Hilight Mini Icon

	LevelRtnuRank rankRtnu;
	int amntRtnu;//XXXXX:临时

	BEGIN_GOBJ_PURE(LevelRecordUnit,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"单位的名称");

		GELEM_VAR_INIT(unsigned __int64,Avtr,0);
			GELEM_EDITVAR("AvatarID",GVT_Bx8,GSem_ProtoPath,"单位的骨架系统的Proto路径");

		GELEM_STRING_INIT(ImpostorMesh,"");
			GELEM_EDITVAR("Impostor模型",GVT_String,GSem_MeshPath,"Impostor模型");

		GELEM_VAR_INIT(float,score,50.0f);
			GELEM_EDITVAR("实力分数",GVT_F,GSem(GSem_Float,"0.1,1000.0,1"),"代表单位实力强弱的分数");
		GELEM_VAR_INIT(int,HP,100);
			GELEM_EDITVAR("最大生命值",GVT_S,GSem_Interger,"单位的最大生命值");
		GELEM_VAR_INIT(int,SP,1000);
			GELEM_EDITVAR("最大精力值",GVT_S,GSem_Interger,"单位的最大精力值");
		GELEM_OBJ(UnitPainInfo,pain);
			GELEM_EDITOBJ("疼痛值设置","疼痛值设置");
		GELEM_VAR_INIT(float,ExhaustedSP,0);
			GELEM_EDITVAR("疲惫的精力值",GVT_S,GSem(GSem_Float,"0,1000000,0.1"),"精力值小于多少进入疲惫状态");
		GELEM_VAR_INIT(AnimTick,ExhaustedCountDown,ANIMTICK_FROM_SECOND(60.0f));
			GELEM_EDITVAR("疲惫存活时间",GVT_U,GSem(GSem_AnimTick,"0,1000,1"),"疲惫存活时间");

		GELEM_OBJ(EvadeEx,evade);
			GELEM_EDITOBJ("闪避","闪避");
		GELEM_OBJ(ResistsEx,resists);
			GELEM_EDITOBJ("抵抗属性","各种抵抗属性");
		GELEM_OBJ(WeaksEx,weaks);
			GELEM_EDITOBJ("弱点","各种弱点");

		GELEM_OBJ(LoUnitMiscFlags,flagsMisc);
			GELEM_EDITOBJ("杂七杂八的标志","杂七杂八的标志");

		GELEM_VAR_INIT(float,DetectRange,12.0f);
			GELEM_EDITVAR("侦测范围",GVT_F,GSem(GSem_Float,"0,100,0"),"侦测范围");

		GELEM_VAR_INIT(float,Radius,0.5f);
			GELEM_EDITVAR("半径",GVT_F,GSem(GSem_Float,"0,100,0"),"单位半径");
		GELEM_VAR_INIT(float,Speed,6.0f);
			GELEM_EDITVAR("速度",GVT_F,GSem(GSem_Float,"0,100,0"),"单位的移动速度");
		GELEM_VAR_INIT(float,Height,1.5f);
			GELEM_EDITVAR("身高",GVT_F,GSem(GSem_Float,"0,100,0"),"单位的身高");
		GELEM_VAR_INIT(DWORD,layorCollide,3);//底层/中层
			GELEM_EDITVAR("碰撞高度层",GVT_U,GSem(GSem_Flags,"底层:1,中层:2,高层:4"),"在哪些高度层上有碰撞 ");
		GELEM_VAR_INIT(float,AimHeight,1.0f);
			GELEM_EDITVAR("瞄准点高度",GVT_F,GSem(GSem_Float,"0,100,0"),"单位的身上的瞄准点的高度");
		GELEM_VAR_INIT(float,CastHeight,1.0f);
			GELEM_EDITVAR("发射点高度",GVT_F,GSem(GSem_Float,"0,100,0"),"单位的发射点的高度");
		GELEM_VAR_INIT(float,htMount,1.0f);
			GELEM_EDITVAR("被骑高度",GVT_F,GSem(GSem_Float,"0,100,0"),"被骑的位置的高度");

		GELEM_VAR_INIT(float,scaleModel,1.0f);
			GELEM_EDITVAR("大小缩放",GVT_F,GSem(GSem_Float,"0,100,0"),"单位模型的大小缩放");

		GELEM_VAR_INIT(AnimTick,RaiseUpDur,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("击倒后站起时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"击倒后站起时间,单位为秒");

		GELEM_VAR_INIT(BOOL,bAdvWalkable,FALSE);
			GELEM_EDITVAR("是否可以走到高级可走区域",GVT_S,GSem_Boolean,"单位是否可以走到高级可走区域");

		GELEM_VAR_INIT(BOOL,bAggressive,TRUE);
			GELEM_EDITVAR("是否具有攻击性",GVT_S,GSem_Boolean,"单位是否具有攻击性");
		GELEM_VAR_INIT(BOOL,bCollar,0);
			GELEM_EDITVAR("有否袖标",GVT_S,GSem_Boolean,"单位身上是否有袖标");
		GELEM_VAR_INIT(BOOL,bAllowKilledKB,TRUE);GELEM_VERSION(2);
			GELEM_EDITVAR("杀死后允许击退",GVT_S,GSem(GSem_Interger,"无法击退,可以击退"),"单位被杀死后是否可以被击退");
		GELEM_VAR_INIT(BOOL,bSpawnInvisibly,FALSE);
			GELEM_EDITVAR("Spawn出来时隐身",GVT_S,GSem_Boolean,"Spawn出来时隐身");
		GELEM_VAR_INIT(BOOL,bSight,0);
			GELEM_EDITVAR("是否要有视野",GVT_S,GSem(GSem_Boolean,"有视野,没有视野"),"最为Retinue时是否有视野");
		GELEM_VAR_INIT(BOOL,bKeepCorpseOnClient,1);
			GELEM_EDITVAR("是否在Client端保留尸体",GVT_S,GSem_Boolean,"是否在Client端保留尸体");
		GELEM_VAR_INIT(BOOL,bCanMove,1);
			GELEM_EDITVAR("是否可以移动",GVT_S,GSem_Boolean,"是否可以移动");
		GELEM_OBJ(NakeEquips,nake)
			GELEM_EDITOBJ("基本装备","基本装备");

		GELEM_VARVECTOR_INIT(RecordID,Equips,RecordID_Invalid);
			GELEM_EDITVAR("附加装备",GVT_U,GSem(GSem_RecordID,"items"),"附加的装备(比如武器,头盔,等)");
		GELEM_OBJ(UnitEquipSets,EquipSets)
			GELEM_EDITOBJ("附加装备集","附加装备集");

		GELEM_VARVECTOR_INIT(RecordID,Skills,RecordID_Invalid);
			GELEM_EDITVAR("初始技能",GVT_U,GSem(GSem_RecordID,"skills"),"单位的技能");

		GELEM_OBJVECTOR(LevelCoSkillInfo,coskills);
			GELEM_EDITOBJ("协同技能信息","协同技能信息");

		GELEM_VAR_INIT(RecordID,buffDead,RecordID_Invalid);
			GELEM_EDITVAR("死亡Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位死亡时的Buff");

		GELEM_OBJ(UnitBuffs,buffs)
			GELEM_EDITOBJ("各种Buff","各种Buff");

		GELEM_OBJ(UnitActions,actions)
			GELEM_EDITOBJ("各种动作","各种动作");

		GELEM_OBJ(LevelUnitFly,fly)
			GELEM_EDITOBJ("飞行控制","飞行控制");

		GELEM_OBJ(LevelUnitPace,pace)
			GELEM_EDITOBJ("步伐控制","步伐控制");

		GELEM_OBJ(LevelSensorParamEx,sensor)
			GELEM_EDITOBJ("感应器","感应器");

		GELEM_VAR_INIT(BOOL,bTalks,0);
			GELEM_EDITVAR("是否可以对话",GVT_S,GSem_Boolean,"单位是否可以和主角发生对话");

		GELEM_OBJ(DropRate,rateDrop)
			GELEM_EDITOBJ("掉落率","各种掉落率的设定");

		GELEM_OBJ(GradeAddOn,addonGrade)
			GELEM_EDITOBJ("升级加成","每升一级的加成属性");

		GELEM_VARARRAY_INIT(RecordID,attksDef,RecordID_Invalid); 
			GELEM_VERSION(3);
			GELEM_EDITVAR("缺省攻击技能",GVT_U,GSem(GSem_RecordID,
				"$Lable{//n/a,徒手,单手盾,单手短兵器+盾,单手短兵器,单手长兵器+盾,单手长兵器,双手剑,弓,弩,双手矛,双手斧,双手杖,双手拖刀,双持剑,单手魔法物件+盾,单手魔法物件}skills"),
				"各种姿势下的缺省攻击技能");
		//XXXXX:more LevelPostureType

		GELEM_VARARRAY_INIT(unsigned __int64,effectsStrike,0); 
			GELEM_EDITVAR("受击效果",GVT_Bx8,GSem(GSem_ProtoPath,
				"$Lable{穿刺,重击,火,电,冰,毒,爆炸,砸碎}"),"受到各种击打的表现效果"); GELEM_VERSION(3)//XXXXX: More DamageAttrType

		GELEM_STRING_INIT(iconTalk,"");
			GELEM_EDITVAR("头像图片",GVT_String,GSem_TexturePartPath,"头像图片");

		GELEM_STRING_INIT(iconRtnu,"");
			GELEM_EDITVAR("随从头像图片",GVT_String,GSem_TexturePartPath,"随从头像图片");

		GELEM_STRING_INIT(pathMI,"");
			GELEM_EDITVAR("小地图图标",GVT_String,GSem_TexturePartPath,"小地图上的图标");
		GELEM_STRING_INIT(pathHilightMI,"");
			GELEM_EDITVAR("小地图图标(高亮)",GVT_String,GSem_TexturePartPath,"小地图上的图标(高亮)");

		GELEM_OBJ(UnitAIs,ais)
			GELEM_EDITOBJ("AI","各种AI的行为图");

		GELEM_VAR_INIT(int,amntRtnu,5);
			GELEM_EDITVAR("随从个数",GVT_S,GSem_Interger,"作为随从最多可以召唤几个");

		GELEM_VAR_INIT(LevelRtnuRank,rankRtnu,LevelRtnuRank_None);
			GELEM_EDITVAR("随从职级",GVT_U,GSem(GSem_Interger,LevelRtnuRank_SemConstraint),"当作为随从时为何种职级");

		GELEM_OBJVECTOR(LevelDialogInfo,dlgs)
			GELEM_EDITOBJ("对话框","对话框信息");

		GELEM_OBJ(LevelEoqDemand,demandEoq);
			GELEM_EDITOBJ("母虫之眼控制要求","母虫之眼控制要求");

	END_GOBJ();

	//Some caches
	LevelAttr_Resists *GetResists()
	{
		return resists.Get();
	}
	LevelAttr_Evade *GetEvade()
	{
		return evade.Get();
	}
	LevelAttr_Weaks *GetWeaks()
	{
		return weaks.Get();
	}

};

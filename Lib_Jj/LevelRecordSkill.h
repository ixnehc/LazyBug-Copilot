#pragma once

#include "class/class.h"

#include "gds/GObj.h"
#include "gds/GObjEx.h"

#include "stringparser/stringparser.h"

#include "records/records.h"

#include "strlib/strlibdefines.h"

#include "ref/ref.h"

#include "LevelSkill.h"
#include "LevelDeal.h"
#include "LevelCost.h"

#include "LevelAttrs_DamageAttr.h"

#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"


#define GELEM_DYNOBJPTR_SKILLPARAM(type,name0,initclss,editname,editdesc)											\
	{																												\
		GElem_DynObjPtr<type>*p=new GElem_DynObjPtr<type>;										\
		p->off=(DWORD)((BYTE*)&ptr->name0-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name0);																	\
		p->elemname=#name0;																		\
		extern CClass*GetClass_##initclss();														\
		p->init=GetClass_##initclss();																\
		p->sem=GSem(GSem_Unknown,"DynObjPtr");									\
		p->name=editname;																			\
		p->desc=editdesc;																				\
		p->bEditable=TRUE;																			\
		_ELEM_LINK;																						\
	}

#define GELEM_DYNOBJPTR_CLASS_SKILLPARAM(name,clss)																					\
	extern CClass*GetClass_##clss();														\
	((GElem_DynObjPtrBase*)curelem)->classes[std::string(#clss)]=GetClass_##clss();						\
	((GElem_DynObjPtrBase*)curelem)->names[std::string(#clss)]=std::string(name);



class CClass;
struct GObjBase;
class CLevelSkill;
struct LevelSkillParam;

struct LevelSkillIcons
{
	std::string normal;
	std::string disable;
	std::string active;

	BEGIN_GOBJ_PURE(LevelSkillIcons,1);
		GELEM_STRING_INIT(normal,"");
			GELEM_EDITVAR("正常图标",GVT_String,GSem_TexturePartPath,"正常图标");
		GELEM_STRING_INIT(disable,"");
			GELEM_EDITVAR("禁用图标",GVT_String,GSem_TexturePartPath,"禁用图标");
		GELEM_STRING_INIT(active,"");
			GELEM_EDITVAR("激活图标",GVT_String,GSem_TexturePartPath,"激活图标");
	END_GOBJ();

};

struct LevelRecordSkill:public CRecord
{
	DEFINE_CLASS(LevelRecordSkill);

	LevelRecordSkill()
	{
		GConstructor();
		requires.push_back(LevelObjRequire_Attackable);
	}
	~LevelRecordSkill()
	{
		GDestructor();
	}

	std::string Name;
	LevelSkillIcons icons;
	float CastRange;
	float CastRangeTolerance;
	BOOL bClosestFollow;
	BOOL b3DFollowObj;
	float rangeFace;
	float tolFace;
	AnimTick CoolDown;
	AnimTick CastTime;
	AnimTick HitDelay;
	int Continuous;
	int bNoFollow;

	std::vector<unsigned __int64> casts;
	unsigned __int64 effectStrike;
	std::vector<unsigned __int64> effectsKilling;
	int SPCost;
	std::vector<LevelDetectTargetFlag> flagsDetect;
	std::vector<LevelObjRequire> requires;
	LevelDetectWeights weightsDetect;

	LevelCost cost;
	HitEx hit;
	CLevelDeal *deal;
	std::vector<DealEntry> deals;
	LevelSkillParam *param;

	template <typename T>
	T *GetDeal()
	{
		if (deal->GetClass()->IsSameWith(Class_Ptr2(T)))
			return (T*)deal;
		for (int i=0;i<deals.size();i++)
		{
			if (deals[i].deal->GetClass()->IsSameWith(Class_Ptr2(T)))
				return (T*)deals[i].deal;
		}

		return NULL;
	}

	template <typename T>
	T *GetParam()
	{
		if (param->GetClass()->IsSameWith(Class_Ptr2(T)))
			return (T*)param;
		return NULL;
	}

	DWORD postures;

	StringID desc;//功能描述

	RecordID idSummon;//XXXXX,临时

	BEGIN_GOBJ(LevelRecordSkill,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"技能的名称");
		GELEM_VAR_INIT( StringID,desc,StringID_Invalid);	
			GELEM_EDITVAR( "技能描述", GVT_U, GSem(GSem_StringID,"技能描述"), "技能的描述文字" );
		GELEM_OBJ(LevelSkillIcons,icons)
			GELEM_EDITOBJ("图标","图标贴图路径");
		GELEM_VAR_INIT(float,CastRange,1.0f);
			GELEM_EDITVAR("施放范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"技能的施放范围");
		GELEM_VAR_INIT(float,CastRangeTolerance,0.2f);
			GELEM_EDITVAR("施放范围冗余",GVT_F,GSem(GSem_Float,"0,100,0.1"),"在开始释放技能时,如果目标大于施放范围,但小于施放范围+冗余,仍可施放");
		GELEM_VAR_INIT(BOOL,bClosestFollow,TRUE);
			GELEM_EDITVAR("跟随时尽量紧贴",GVT_S,GSem_Boolean,"跟随目标时是否要尽量靠近目标");
		GELEM_VAR_INIT(BOOL,b3DFollowObj,TRUE);
			GELEM_EDITVAR("跟随目标的3D位置(仅当自己为飞行单位)",GVT_S,GSem_Boolean,"跟随目标的3D位置(仅当自己为飞行单位)");
		GELEM_VAR_INIT(float,rangeFace,10.0f);
			GELEM_EDITVAR("朝向范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"面对目标时目标必须在多大的范围内,单位为米");
		GELEM_VAR_INIT(float,tolFace,30.0f);
			GELEM_EDITVAR("朝向容忍度",GVT_F,GSem(GSem_Float,"0,180,0.1"),"面对目标时目标必须在多大的角度范围内,单位为度");
		GELEM_VAR_INIT(BOOL,Continuous,0);
			GELEM_EDITVAR("是否会连续释放",GVT_S,GSem_Boolean,"开始施放后,是否要持续施放");
		GELEM_VAR_INIT(BOOL,bNoFollow,0);
			GELEM_EDITVAR("是否能够跟随释放",GVT_S,GSem(GSem_Interger,"可以跟随,不能跟随"),"是否能够跟随对象释放");
		GELEM_VAR_INIT(int,SPCost,100);
			GELEM_EDITVAR("灵气消耗",GVT_S,GSem_Interger,"技能消耗的灵气点数");
		GELEM_VAR_INIT(DWORD,postures,0);
			GELEM_EDITVAR("姿势要求",GVT_U,GSem(GSem_Flags,LevelPostureFlagsStr),"本技能在哪些姿势下可以使用");
		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("施放对象标志",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"侦测什么类型的单位施放");
		GELEM_VARVECTOR_INIT(LevelObjRequire,requires,LevelObjRequire_Attackable);
			GELEM_EDITVAR("施放对象的特定需求",GVT_S,GSem(GSem_Interger,LevelObjRequire_SemConstraint),"施放对象的特定需求");
		GELEM_OBJ(LevelDetectWeights,weightsDetect)
			GELEM_EDITOBJ("侦测权重","侦测权重");

		GELEM_VAR_INIT(AnimTick,CoolDown,ANIMTICK_FROM_SECOND(1.5f));
			GELEM_EDITVAR("冷却时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),
				"CoolDown时间(以秒为单位),\n这个值必须大于施放时间");
		GELEM_VAR_INIT(AnimTick,CastTime,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("施放时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),
				"开始施放技能后多长时间后才能自由移动,以秒为单位,(这个数值主要用在服务器端),\n这个值必须大于HitDelay");
		GELEM_VAR_INIT(AnimTick,HitDelay,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("命中延迟",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),
				"开始施放到命中的间隔时间,以秒为单位,这个数值由Server使用");
		GELEM_VARVECTOR_INIT(unsigned __int64,casts,0);
			GELEM_EDITVAR("技能释放效果",GVT_Bx8,GSem_ProtoPath,"技能释放效果");
		GELEM_VAR_INIT(unsigned __int64,effectStrike,0);
			GELEM_EDITVAR("技能命中效果",GVT_Bx8,GSem_ProtoPath,"技能命中效果");
		GELEM_VARVECTOR_INIT(unsigned __int64,effectsKilling,0);
			GELEM_EDITVAR("技能杀死效果",GVT_Bx8,GSem_ProtoPath,"技能杀死效果");

		GELEM_OBJ(LevelCost,cost)
			GELEM_EDITOBJ("消耗","消耗");

		GELEM_VAR_INIT(RecordID,idSummon,RecordID_Invalid);
			GELEM_EDITVAR("召唤的单位",GVT_U,GSem(GSem_RecordID,"units"),"召唤的单位");

		GELEM_OBJ(HitEx,hit);
			GELEM_EDITOBJ("命中","命中");
		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_PhysDmg, "技能的结算", "选择不同的技能结算" );
			GELEMS_LEVELDEAL_CANDIDATES();
		GELEM_OBJVECTOR(DealEntry,deals);
			GELEM_EDITOBJ("结算列表","多个结算");

		GELEM_DYNOBJPTR_SKILLPARAM(LevelSkillParam,param,SkillParam_MeleeAttack, "技能模式", "选择不同的技能模式" );
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "00a.通用", SkillParam_General);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "00b.通用(客户端驱动)", SkillParam_GeneralC);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "00c.通用(服务器端驱动)_obsolete", SkillParam_GeneralS);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "00d.通用(服务器端驱动)", SkillParam_GeneralAdvS);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "01.通用(Gesture)", SkillParam_Gesture);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "02.近战攻击", SkillParam_MeleeAttack);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "04.触发功能单位", SkillParam_InvokeAgent);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "05.扇形攻击", SkillParam_Sweep);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "08.灵气飞行", SkillParam_MissileSP);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "09.拾取道具", SkillParam_InvokeItem);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "10.进入功能单位", SkillParam_Reside);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "11.开始飞行", SkillParam_FlyUp);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "12.俯冲", SkillParam_Dive);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "14.多重近战攻击", SkillParam_Zeal);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "15.火鸦召唤", SkillParam_RavenSummon);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "16.火鸦发射子弹", SkillParam_RavenBullet);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "17.喷射攻击", SkillParam_SpitAttack);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "18.投掷Eo", SkillParam_ThrowEo);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "19.召唤", SkillParam_Summon);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "20.施放信号", SkillParam_CastSignal);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "21.连锁召唤", SkillParam_ChainSummon);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "22.连锁喷发", SkillParam_ChainSpurt);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "23.喷洒", SkillParam_Spray);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "24.滚翻放置Eo", SkillParam_Roll);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "25.猛扑Mount", SkillParam_PounceMount);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "26.吸取", SkillParam_MeleeAbsorb);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "27.乌图姆召唤", SkillParam_UtumSummon);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "28.全身投入", SkillParam_Plunge);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "29.跳跃攻击", SkillParam_JumpAttack);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "30.冲刺", SkillParam_Charge);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "31.传送攻击", SkillParam_TeleportAttack);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "32.幻影攻击", SkillParam_PhantomAttack);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "33.守卫", SkillParam_Guard);
			GELEM_DYNOBJPTR_CLASS_SKILLPARAM( "34.推动滑槽", SkillParam_PushSlideway);

			
//XXXXX:More LevelSkill

	END_GOBJ();


};

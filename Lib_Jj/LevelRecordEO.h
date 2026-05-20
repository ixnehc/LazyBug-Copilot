#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GObjEx.h"
#include "anim/animdefines.h"
#include "records/records.h"

#include "stringparser/stringparser.h"

#include "LevelDeal.h"
#include "LevelDetectTargetFlags.h"
#include "LevelAttrs_DamageAttr.h"

#include "LoEffectObj.h"


#define GELEM_DYNOBJPTR_EOPARAM(type,name0,initclss,editname,editdesc)											\
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

#define GELEM_DYNOBJPTR_CLASS_EOPARAM(name,clss)																					\
	extern CClass*GetClass_##clss();														\
	((GElem_DynObjPtrBase*)curelem)->classes[std::string(#clss)]=GetClass_##clss();						\
	((GElem_DynObjPtrBase*)curelem)->names[std::string(#clss)]=std::string(name);

enum EoClientInitialXfmBinding
{
	EoClientInitialXfmBinding_None=0,
	EoClientInitialXfmBinding_NoBinding=1,
	EoClientInitialXfmBinding_OwnerFoot=2,
	EoClientInitialXfmBinding_HostAimPos=3,
	EoClientInitialXfmBinding_OwnerFootXZ=4,

	EoClientInitialXfmBinding_ForceDword=0xffffffff,
};



struct LevelEoParam;
struct LevelRecordEo:public CRecord
{
	DEFINE_CLASS(LevelRecordEo);

	LevelRecordEo()
	{
		GConstructor();
		hit.bValid=0;
	}
	~LevelRecordEo()
	{
		GDestructor();
	}

	std::string Name;
	LevelEoParam *param;
	unsigned __int64 idEffect;

	EoClientInitialXfmBinding bindingClientInitialXfm;

	std::vector<LevelDetectTargetFlag> detects;
	std::vector<LevelObjRequire> requires;

	unsigned __int64 effectStrike;

	HitEx hit;
	CLevelDeal *deal;
	std::vector<DealEntry> deals;

	StringID idBG;

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
	DealEntry *GetDealEntry()
	{
		for (int i=0;i<deals.size();i++)
		{
			if (deals[i].deal->GetClass()->IsSameWith(Class_Ptr2(T)))
				return &deals[i];
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


	BEGIN_GOBJ_UID(LevelRecordEo,1);

		GELEM_STRING_INIT(Name,""); GELEM_UID(1);
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"Eo的名称");

		GELEM_VAR_INIT(unsigned __int64,idEffect,0); GELEM_UID(2);
			GELEM_EDITVAR("效果Proto",GVT_Bx8,GSem_ProtoPath,"效果的Proto");

		GELEM_VAR_INIT(DWORD,bindingClientInitialXfm,EoClientInitialXfmBinding_NoBinding); GELEM_UID(10);
			GELEM_EDITVAR("Client端起始Xfm绑定",GVT_U,GSem(GSem_Interger,"无绑定:1,绑定Owner脚下位置:2,绑定Host瞄准点:3,绑定Owner脚下位置(XZ):4"),"Client端起始Xfm绑定");

		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,detects,LevelDetectTargetFlag_Default); GELEM_UID(3);
			GELEM_EDITVAR("对象类型",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"作用于什么类型的单位");

		GELEM_VARVECTOR_INIT(LevelObjRequire,requires,LevelObjRequire_Attackable); GELEM_UID(4);
			GELEM_EDITVAR("对象的特定需求",GVT_S,GSem(GSem_Interger,LevelObjRequire_SemConstraint),"对象的特定需求");

		GELEM_VAR_INIT(unsigned __int64,effectStrike,0); GELEM_UID(5);
			GELEM_EDITVAR("命中效果",GVT_Bx8,GSem_ProtoPath,"命中效果");

		GELEM_OBJ(HitEx,hit);GELEM_VERSION(2); GELEM_UID(6);
			GELEM_EDITOBJ("命中","命中");
		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_PhysDmg, "结算(obsolete)", "选择不同的技能命中效果" ); GELEM_UID(7);
			GELEMS_LEVELDEAL_CANDIDATES();
		GELEM_OBJVECTOR(DealEntry,deals); GELEM_UID(8);
			GELEM_EDITOBJ("结算列表","多个结算");

		GELEM_VAR_INIT( StringID,idBG,StringID_Invalid);	
			GELEM_EDITVAR( "行为图", GVT_U, GSem(GSem_StringID,"行为图名称"), "行为图" );

		GELEM_DYNOBJPTR_EOPARAM(LevelEoParam,param,EoParamAreaDmg, "EO类型", "选择不同的EO类型" ); GELEM_UID(9);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "01.区域伤害", EoParamAreaDmg);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "02.炸弹", EoParamBomb);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "03.波", EoParamWave);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "04.吸取", EoParamAbsorb);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "05.区域持续影响", EoParamArea);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "06.陷阱", EoParamTrap);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "07.条带", EoParamStripe);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "08.挥舞", EoParamSwing);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "09.挥舞子弹", EoParamSwingBullet);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "10.引力球", EoParamMagnetBall);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "11.爆炸子弹", EoParamBulletBomb);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "12.推压(Obsolete)", EoParamPush);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "12.1.推压", EoParamPushAdv);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "13.发射子弹", EoParamShootBullet);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "14.子弹", EoParamBullet);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "15.发送信号", EoParamSignal);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "16.导弹", EoParamMissile);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "17.连锁", EoParamChain);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "18.单位爆裂", EoParamObliterater);
			GELEM_DYNOBJPTR_CLASS_EOPARAM( "19.龙卷风", EoParamTornado);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("20.脚尖石地面爆裂", EoParamToeStoneThrusts);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("21.链接", EoParamLink);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("22.战斗环境", EoParamEnv);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("23.OwnerDeal(Obsolete)", EoParamOwnerDeal);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("23.1.ObjDeal", EoParamObjDeal);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("24.空中炸弹", EoParamAirBomb);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("25.地丝", EoParamLichen);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("26.锁链锤", EoParamChainedHammer);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("27.步进", EoParamStep);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("28.地丝蔓延", EoParamLichenSpread);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("29.乌图姆攻击", EoParamUtumAttack);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("30.乌图姆修桥", EoParamUtumRepairBridge);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("31.Invoke", EoParamInvoke);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("32.灯塔路线", EoParamLanternRoute);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("33.圣光球", EoParamHolyOrb);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("34.魔血", EoParamDemonBlood);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("35.MassSpline", EoParamMassSpline);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("35.1.Spline子弹", EoParamSplineBullet);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("36.乌鸦", EoParamRavens);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("37.Shards", EoParamShards);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("38.BellyEgg", EoParamBellyEgg);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("39.BellyEelString", EoParamBellyEelString);
			GELEM_DYNOBJPTR_CLASS_EOPARAM("40.MagicCircuitRailAbsorb", EoParamMagicCircuitRailAbsorb);

//XXXXX:More CLoEffectObj

	END_GOBJ();


};

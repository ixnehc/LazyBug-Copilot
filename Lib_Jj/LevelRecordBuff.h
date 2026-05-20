#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GObjEx.h"
#include "anim/animdefines.h"
#include "records/records.h"

#include "stringparser/stringparser.h"

#include "LevelReactor.h"

#include "LevelBuff.h"


#define GELEM_DYNOBJPTR_BUFFPARAM(type,name0,initclss,editname,editdesc)											\
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

#define GELEM_DYNOBJPTR_CLASS_BUFFPARAM(name,clss)																					\
	extern CClass*GetClass_##clss();														\
	((GElem_DynObjPtrBase*)curelem)->classes[std::string(#clss)]=GetClass_##clss();						\
	((GElem_DynObjPtrBase*)curelem)->names[std::string(#clss)]=std::string(name);




struct LevelBuffParam;
struct LevelRecordBuff:public CRecord
{
	DEFINE_CLASS(LevelRecordBuff);

	LevelRecordBuff()
	{
		GConstructor();
		hit.bValid=0;
	}
	~LevelRecordBuff()
	{
		GDestructor();
	}

	std::string Name;
	LevelBuffParam *param;
	unsigned __int64 idEffect;
	AnimTick Dur;
	BOOL bCanTeleport;
	BOOL bDestroyOnDie;

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
	T *GetParam()
	{
		if (param->GetClass()->IsSameWith(Class_Ptr2(T)))
			return (T*)param;
		return NULL;
	}

	std::vector<LevelDetectTargetFlag> detects;
	std::vector<LevelObjRequire> requires;

	HitEx hit;
	CLevelDeal *deal;
	std::vector<DealEntry> deals;

	std::vector<LevelReactorParamEntry> paramsReactor;

	BEGIN_GOBJ(LevelRecordBuff,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"Buff的名称");

		GELEM_VAR_INIT(unsigned __int64,idEffect,0);
			GELEM_EDITVAR("效果Proto",GVT_Bx8,GSem_ProtoPath,"效果的Proto");

		GELEM_VAR_INIT(AnimTick,Dur,0);
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,1000,0.1"),
				"Buff的持续时间(以秒为单位),\n如果为0,表示使用缺省时间");

		GELEM_VAR_INIT(BOOL,bCanTeleport,FALSE)
			GELEM_EDITVAR("可以传送",GVT_S,GSem_Boolean,"传送后是否要保留这个Buff");
		GELEM_VAR_INIT(BOOL,bDestroyOnDie,FALSE)
			GELEM_EDITVAR("死后消失",GVT_S,GSem_Boolean,"死后消失");

		GELEM_VAR_INIT( StringID,idBG,StringID_Invalid);	
			GELEM_EDITVAR( "行为图", GVT_U, GSem(GSem_StringID,"行为图名称"), "行为图" );

		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,detects,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("对象类型",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"作用于什么类型的单位");
		GELEM_VARVECTOR_INIT(LevelObjRequire,requires,LevelObjRequire_Attackable);
			GELEM_EDITVAR("对象的特定需求",GVT_S,GSem(GSem_Interger,LevelObjRequire_SemConstraint),"对象的特定需求");

		GELEM_OBJVECTOR(LevelReactorParamEntry,paramsReactor);
			GELEM_EDITOBJ("Reactor参数","Reactor参数")

		GELEM_OBJ(HitEx,hit); GELEM_VERSION(2);
			GELEM_EDITOBJ("命中","命中");

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_Dmg, "技能的命中效果", "选择不同的技能命中效果" );
			GELEMS_LEVELDEAL_CANDIDATES();
		GELEM_OBJVECTOR(DealEntry,deals);
			GELEM_EDITOBJ("结算列表","多个结算");


		GELEM_DYNOBJPTR_BUFFPARAM(LevelBuffParam,param,BuffParam_Birth, "Buff类型", "选择不同的Buff类型" );
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "01.出生", BuffParam_Birth);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "02.冷冻减速", BuffParam_Cold);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "03.死亡", BuffParam_Dead);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "04.冻结", BuffParam_Frozen);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "05.击退", BuffParam_KB);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "06.击倒", BuffParam_KD);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "07.硬直", BuffParam_Stun);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "07a.Skill硬直", BuffParam_SkillStun);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "08.进洞", BuffParam_ResideHole);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "09.进岗楼", BuffParam_ResideWT);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "10.休眠", BuffParam_Dormant);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "11.飞行出生", BuffParam_FlyBirth);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "12.石化", BuffParam_Petrify);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "13.石化凝固", BuffParam_Petrified);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "14.包围姿态(Siege)", BuffParam_Siege);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "15.隐身", BuffParam_Invisible);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "16.通用", BuffParam_General);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "17.乌图姆(战斗)出生", BuffParam_UtumBirth);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "18.免疫", BuffParam_Immune);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "19.Deal", BuffParam_Deal);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "20.燃烧", BuffParam_Burning);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "21.推开", BuffParam_PB);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "22.加速", BuffParam_IMS);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "23.区域影响", BuffParam_Area);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "24.处于石板迷宫内", BuffParam_InSlates);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "25.传送", BuffParam_Teleport);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "27.流血", BuffParam_Bleeding);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "28.发射", BuffParam_Shoot);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "29.弱点", BuffParam_Weaks);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "30.格挡", BuffParam_Blocking);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "31.快闪", BuffParam_Jink);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "32.飞虫群", BuffParam_Flies);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "33.不可攻击", BuffParam_NotAttackable);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "34.1.蜈蚣节点_匍匐", BuffParam_CentipedeNode_Crouch);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "34.2.蜈蚣节点_移动", BuffParam_CentipedeNode_Move);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "34.3.蜈蚣节点_触须", BuffParam_CentipedeNode_Tendrils);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "34.4.蜈蚣之囊", BuffParam_CentipedeCyst);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "35.没有碰撞", BuffParam_GhostCollide);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "36.附体", BuffParam_Possess);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "37.舌虫", BuffParam_TongueFly);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "38.旗帜", BuffParam_Banner);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "39.火蝇", BuffParam_FireFly);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "40.锁定SP", BuffParam_LockSP);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "41.1.力量Aura", BuffParam_AuraStrength);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "41.2.光明Aura", BuffParam_AuraLight);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "41.3.自然Aura", BuffParam_AuraNature);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "41.4.智慧Aura", BuffParam_AuraWisdom);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "41.5.神秘Aura", BuffParam_AuraMystery);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "41.6.戏谑Aura", BuffParam_AuraJoker);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "42.Slime", BuffParam_Slime);
			GELEM_DYNOBJPTR_CLASS_BUFFPARAM( "43.SacredOrb", BuffParam_SacredOrb);
            //XXXXX:More LevelBuff


	END_GOBJ();


	

};

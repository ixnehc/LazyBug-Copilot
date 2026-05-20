#pragma once

#include "class/class.h"

#include "LevelDefines.h"

#include "gds/GObjEx.h"
#include "gds/GObjUID.h"


#define GELEM_DYNOBJPTR_DEAL(type,name0,initclss,editname,editdesc)											\
	{																												\
		GElem_DynObjPtr<type>*p=new GElem_DynObjPtr<type>;										\
		p->off=(DWORD)((BYTE*)&ptr->name0-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name0);																	\
		p->elemname=#name0;																		\
		extern CClass*GetDealClass_##initclss();														\
		p->init=GetDealClass_##initclss();																\
		p->sem=GSem(GSem_Unknown,"DynObjPtr");									\
		p->name=editname;																			\
		p->desc=editdesc;																				\
		p->bEditable=TRUE;																			\
		_ELEM_LINK;																						\
	}

#define GELEM_DYNOBJPTR_CLASS_DEAL(name,clss)																					\
	extern CClass*GetDealClass_##clss();														\
	((GElem_DynObjPtrBase*)curelem)->classes[std::string(#clss)]=GetDealClass_##clss();						\
	((GElem_DynObjPtrBase*)curelem)->names[std::string(#clss)]=std::string(name);

#define GELEMS_LEVELDEAL_CANDIDATES() \
GELEM_DYNOBJPTR_CLASS_DEAL( "00.n/a", Deal_Null);							\
GELEM_DYNOBJPTR_CLASS_DEAL( "01.物理伤害", Deal_PhysDmg);						\
GELEM_DYNOBJPTR_CLASS_DEAL( "02.石化", Deal_Petrify);									\
GELEM_DYNOBJPTR_CLASS_DEAL( "03.伤害", Deal_Dmg);						\
GELEM_DYNOBJPTR_CLASS_DEAL( "04.创建Eo", Deal_CreateEo);								\
GELEM_DYNOBJPTR_CLASS_DEAL( "05.魔血回复", Deal_SoulRecover);								\
GELEM_DYNOBJPTR_CLASS_DEAL( "06.1.HP回复", Deal_CureHP);									\
GELEM_DYNOBJPTR_CLASS_DEAL( "06.2.SP回复", Deal_CureSP);									\
GELEM_DYNOBJPTR_CLASS_DEAL( "07.召唤Unit", Deal_SummonUnit);						\
GELEM_DYNOBJPTR_CLASS_DEAL( "09.改变资源", Deal_ModRes);								\
GELEM_DYNOBJPTR_CLASS_DEAL( "10.清除所有OwnerBuff", Deal_CleanOwnerBuff);		\
GELEM_DYNOBJPTR_CLASS_DEAL( "12.1.添加Buff", Deal_MakeBuff);							\
GELEM_DYNOBJPTR_CLASS_DEAL( "12.2.去除Buff", Deal_RemoveBuff);							\
GELEM_DYNOBJPTR_CLASS_DEAL( "13.1.击倒", Deal_KnockDown);								\
GELEM_DYNOBJPTR_CLASS_DEAL( "13.2.击退", Deal_KnockBack);								\
GELEM_DYNOBJPTR_CLASS_DEAL( "13.3.眩晕", Deal_Dizzy);								\
GELEM_DYNOBJPTR_CLASS_DEAL( "14.改变资源(石板迷宫)", Deal_ModSlatesRes);	\
GELEM_DYNOBJPTR_CLASS_DEAL( "15.1血牙剑火球伤害", Deal_BloodTeethFireBlood);\
GELEM_DYNOBJPTR_CLASS_DEAL( "15.2血牙剑电球伤害", Deal_BloodTeethLightningBlood);\
GELEM_DYNOBJPTR_CLASS_DEAL( "16.吸取", Deal_Suck);					\
GELEM_DYNOBJPTR_CLASS_DEAL( "17.快闪", Deal_Jink);					\
GELEM_DYNOBJPTR_CLASS_DEAL( "18.飞虫群操作", Deal_FliesOp);	\
GELEM_DYNOBJPTR_CLASS_DEAL( "19.打断蜈蚣节点", Deal_BreakCentipedeNode);\
GELEM_DYNOBJPTR_CLASS_DEAL( "20.创建孢子", Deal_CreateSpore);\
GELEM_DYNOBJPTR_CLASS_DEAL( "21.增加MP", Deal_IncMP);	\
GELEM_DYNOBJPTR_CLASS_DEAL( "22.引爆圣光球", Deal_IgniteHolyOrb);\
GELEM_DYNOBJPTR_CLASS_DEAL( "23.触发Pain掉落", Deal_MakePainDrop);\
GELEM_DYNOBJPTR_CLASS_DEAL( "24.Teleport", Deal_Teleport);\
GELEM_DYNOBJPTR_CLASS_DEAL( "25.踩踏BellyEgg", Deal_StompBellyEgg);\
GELEM_DYNOBJPTR_CLASS_DEAL( "26.调用行为树中继", Deal_StartBhvRelay);




#define BIND_DEAL(clssDeal)													\
	CClass *GetDealClass_##clssDeal()														\
	{																													\
		return Class_Ptr2(clssDeal);															\
	}

// struct LevelAttackAddOn_Deal:public LevelAttackAddOn
// {
// 	BEGIN_GOBJ_PURE_UID(LevelAttackAddOn_Deal,1)
// 		GELEM_VAR_INIT(short,atkDelta,0);
// 			GELEM_EDITVAR("攻击增量",GVT_SS,GSem_Interger,"攻击增量");
// 		GELEM_VAR_INIT(float,atkMultiply,1.0f);
// 			GELEM_EDITVAR("攻击倍率",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"攻击倍率");
// 		GELEM_VAR_INIT(short,accuDelta,0);
// 			GELEM_EDITVAR("命中增量",GVT_SS,GSem_Interger,"命中增量");
// 		GELEM_VAR_INIT(float,accuMultiply,1.0f);
// 			GELEM_EDITVAR("命中倍率",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"命中倍率");
// 		GELEM_VAR_INIT(short,stunDelta,0);
// 			GELEM_EDITVAR("硬直增量",GVT_SS,GSem_Interger,"硬直增量");
// 		GELEM_VAR_INIT(float,stunMultiply,1.0f);
// 			GELEM_EDITVAR("硬直倍率",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"硬直倍率");
// 	END_GOBJ();
// };

struct LevelObliterateArg;
struct DealArg
{
	DealArg()
	{
		grd=0;
		amount=0;
		argObliterate=NULL;
		idTarget=LevelObjID_Invalid;
		multiply=1.0f;
		hpInitial=-1;
	}
	LevelGrade grd;
	LevelPos3D dir;
	LevelOpLink link;
	DWORD amount;
	LevelObjID idTarget;
	LevelObliterateArg *argObliterate;
	float multiply;
	int hpInitial;
};

struct DealResult
{
	DealResult()
	{
		idSummoned=LevelObjID_Invalid;
	}
	LevelObjID idSummoned;
};


struct LevelOSB;
class CLevelObj;
struct CLevelDeal
{
	CLevelDeal()
	{
	}
	virtual CClass *GetClass()=0;
	virtual GObjBase*GetGObj()=0;

	virtual BOOL IsNull()	{		return FALSE;	}

	virtual void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)	{	}
	virtual void Make(LevelOSB &osbSrc,LevelPos3D &pos,DealArg&arg,DealResult *result)	{	}

	virtual float GetFireDmg()	{		return 0.0f;	}

	CLevelDeal *Clone();

	template <typename T>
	T *ToPtr()
	{
		if (GetClass()->IsSameWith(Class_Ptr2(T)))
			return (T*)this;
		return NULL;
	}
};




class Deal_Null:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_Null);

	BEGIN_GOBJ_PURE(Deal_Null,1);
	END_GOBJ();

	BOOL IsNull()override	{		return TRUE;	}

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override	{	}

};

struct DealRequire
{
};

struct DealEntry
{
	float chance;
	StringID nmValidatorSrc;
	CLevelDeal *deal;

	BEGIN_GOBJ_PURE_UID(DealEntry,1);
		GELEM_VAR_INIT(float,chance,1.0f); GELEM_UID(1)
			GELEM_EDITVAR("机率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.01"),"多大机率Apply这个Deal");
		GELEM_VAR_INIT( StringID,nmValidatorSrc,StringID_Invalid);	GELEM_UID(3);
			GELEM_EDITVAR( "源对象判断器", GVT_U, GSem(GSem_StringID,"行为图中继名称"), "一个行为图中继的名称,用来执行一段更新逻辑" );
		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_Null, "命中效果", "选择不同的命中效果" );GELEM_UID(2)
			GELEMS_LEVELDEAL_CANDIDATES();
	END_GOBJ();

};

extern void MakeDeals(std::vector<DealEntry> &deals,LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result);
extern void MakeDeals(std::vector<DealEntry> &deals,LevelOSB &osbSrc,LevelPos3D &pos,DealArg&arg,DealResult *result);

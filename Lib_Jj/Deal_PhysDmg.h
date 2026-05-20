#pragma once
#include "LevelDefines.h"

#include "LevelDeal.h"

struct LevelAttackAddOn
{
	LevelAttackAddOn()
	{
		atkDelta=0;
		atkMultiply=1.0f;
		accuDelta=0;
		accuMultiply=1.0f;
	}
	short atkDelta;
	float atkMultiply;
	short accuDelta;
	float accuMultiply;
	short stunDelta;
	float stunMultiply;
};


struct LevelAttackAddOn_Deal:public LevelAttackAddOn
{
	BEGIN_GOBJ_PURE_UID(LevelAttackAddOn_Deal,1)
		GELEM_VAR_INIT(short,atkDelta,0);
			GELEM_EDITVAR("攻击增量",GVT_SS,GSem_Interger,"攻击增量");
		GELEM_VAR_INIT(float,atkMultiply,1.0f);
			GELEM_EDITVAR("攻击倍率",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"攻击倍率");
		GELEM_VAR_INIT(short,accuDelta,0);
			GELEM_EDITVAR("命中增量",GVT_SS,GSem_Interger,"命中增量");
		GELEM_VAR_INIT(float,accuMultiply,1.0f);
			GELEM_EDITVAR("命中倍率",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"命中倍率");
		GELEM_VAR_INIT(short,stunDelta,0);
			GELEM_EDITVAR("硬直增量",GVT_SS,GSem_Interger,"硬直增量");
		GELEM_VAR_INIT(float,stunMultiply,1.0f);
			GELEM_EDITVAR("硬直倍率",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"硬直倍率");
	END_GOBJ();
};


class Deal_PhysDmg:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_PhysDmg);


	BEGIN_GOBJ_PURE(Deal_PhysDmg,1);

		GELEM_OBJ(LevelAttackAddOn_Deal,_addon);
			GELEM_EDITOBJ("攻击附加参数","攻击附加参数");
		GELEM_VAR_INIT(BOOL,_bMelee,TRUE);
			GELEM_EDITVAR("肉搏伤害",GVT_S,GSem_Boolean,"是否为肉搏伤害(还是远程伤害)");

		GELEM_VAR_INIT(float,_strKill,0.0f);
			GELEM_EDITVAR("击飞力量",GVT_F,GSem(GSem_Float,"0,100,0.1"),"杀死敌人后用多大力量击飞敌人");

		GELEM_VAR_INIT(float,_rate,1.0f);
			GELEM_EDITVAR("伤害比率",GVT_F,GSem(GSem_Float,"0,100,0.01"),"造成的伤害相对于基本伤害的比率");
		GELEM_VAR_INIT(BOOL,_bKB,0);
			GELEM_EDITVAR("是否会击退敌人",GVT_S,GSem_Boolean,"是否会击退敌人");

	END_GOBJ();

	LevelAttackAddOn_Deal _addon;
	BOOL _bMelee;
	float _rate;
	float _strKill;//杀死敌人后,用多大的力量击飞敌人
	BOOL _bKB;

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

#pragma once

#include "LevelDeal.h"

#include "LevelAttrs_DamageAttr.h"
#include "LevelAttrs_Weak.h"

// struct DmgEntry
// {
// 	BEGIN_GOBJ_PURE(DmgEntry,1);
// 		GELEM_OBJ(DamagesEx,attacks); GELEM_UID(1);
// 			GELEM_EDITOBJ("伤害参数","伤害参数");
// 		GELEM_OBJ(OppressesEx,opresses); GELEM_UID(3);
// 			GELEM_EDITOBJ("弱点","伤害参数");
// 		GELEM_VAR_INIT(float,strKill,0.0f);GELEM_UID(2);
// 			GELEM_EDITVAR("击飞力量",GVT_F,GSem(GSem_Float,"0,100,0.1"),"杀死敌人后用多大力量击飞敌人");
// 	END_GOBJ();
// 
// 	DamagesEx attacks_;
// 	OppressesEx opresses;
// 	float strKill;//杀死敌人后,用多大的力量击飞敌人
// };


class Deal_Dmg:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_Dmg);


	BEGIN_GOBJ_PURE(Deal_Dmg,1);

		GELEM_OBJ(DamagesEx,_attacks); GELEM_UID(1);
			GELEM_EDITOBJ("伤害参数","伤害参数");
		GELEM_VAR_INIT(float,_strKill,0.0f);GELEM_UID(2);
			GELEM_EDITVAR("击飞力量",GVT_F,GSem(GSem_Float,"0,100,0.1"),"杀死敌人后用多大力量击飞敌人");
		GELEM_VAR_INIT(DmgBlockType,_tpBlock,DmgBlockType_NotBlockable);GELEM_UID(4);
			GELEM_EDITVAR("格挡类型",GVT_S,GSem(GSem_Interger,GSemConstraint_DmgBlockType),"格挡类型");

// 		GELEM_VAR_INIT(int,_grdEdit,0);GELEM_UID(3);
// 		GELEM_EDITVAR("等级",GVT_S,GSem(GSem_Interger,
// 			"等级0"		"$伤害参数(等级0),"
// 			"等级1"		"$伤害参数(等级1),"
// 			"等级2"		"$伤害参数(等级2),"
// 			"等级3"		"$伤害参数(等级3),"
// 			"等级4"		"$伤害参数(等级4),"
// 			"等级5"		"$伤害参数(等级5),"
// 			"等级6"		"$伤害参数(等级6),"
// 			"等级7"		"$伤害参数(等级7),"
// 			"等级8"		"$伤害参数(等级8),"
// 			"等级9"		"$伤害参数(等级9),"
// 			),"编辑哪个等级");
// 		GELEM_OBJ(DmgEntry,_entries[0]); GELEM_UID(11);
// 			GELEM_EDITOBJ("伤害参数(等级0)","伤害参数");
// 		GELEM_OBJ(DmgEntry,_entries[1]); GELEM_UID(12);
// 			GELEM_EDITOBJ("伤害参数(等级1)","伤害参数");
// 		GELEM_OBJ(DmgEntry,_entries[2]); GELEM_UID(13);
// 			GELEM_EDITOBJ("伤害参数(等级2)","伤害参数");
// 		GELEM_OBJ(DmgEntry,_entries[3]); GELEM_UID(14);
// 			GELEM_EDITOBJ("伤害参数(等级3)","伤害参数");
// 		GELEM_OBJ(DmgEntry,_entries[4]); GELEM_UID(15);
// 			GELEM_EDITOBJ("伤害参数(等级4)","伤害参数");
// 		GELEM_OBJ(DmgEntry,_entries[5]); GELEM_UID(16);
// 			GELEM_EDITOBJ("伤害参数(等级5)","伤害参数");
// 		GELEM_OBJ(DmgEntry,_entries[6]); GELEM_UID(17);
// 			GELEM_EDITOBJ("伤害参数(等级6)","伤害参数");
// 		GELEM_OBJ(DmgEntry,_entries[7]); GELEM_UID(18);
// 			GELEM_EDITOBJ("伤害参数(等级7)","伤害参数");
// 		GELEM_OBJ(DmgEntry,_entries[8]); GELEM_UID(19);
// 			GELEM_EDITOBJ("伤害参数(等级8)","伤害参数");
// 		GELEM_OBJ(DmgEntry,_entries[9]); GELEM_UID(20);
// 			GELEM_EDITOBJ("伤害参数(等级9)","伤害参数");

	END_GOBJ();

	DamagesEx _attacks;
	float _strKill;//杀死敌人后,用多大的力量击飞敌人
	DmgBlockType _tpBlock;

// 	DmgEntry _entries[10];
// 	int _grdEdit;

	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};

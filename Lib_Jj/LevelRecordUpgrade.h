#pragma once

#include "class/class.h"

#include "gds/GObj.h"
#include "gds/GObjEx.h"

#include "stringparser/stringparser.h"

#include "records/records.h"

#include "strlib/strlibdefines.h"

#include "LevelAbility.h"


#define GELEM_DYNOBJPTR_UPGRADE(type,name0,initclss,editname,editdesc)											\
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

#define GELEM_DYNOBJPTR_CLASS_UPGRADE(name,clss)																					\
	extern CClass*GetClass_##clss();														\
	((GElem_DynObjPtrBase*)curelem)->classes[std::string(#clss)]=GetClass_##clss();						\
	((GElem_DynObjPtrBase*)curelem)->names[std::string(#clss)]=std::string(name);



class CClass;
struct GObjBase;


struct LevelRecordUpgrade:public CRecord
{
	DEFINE_CLASS(LevelRecordUpgrade);

	LevelRecordUpgrade()
	{
		GConstructor();
	}
	~LevelRecordUpgrade()
	{
		GDestructor();
	}

	std::string Name;
	StringID desc;
	CLevelUpgrade *upgrade;

	template <typename T>
	T *GetUpgrade()
	{
		if (param->GetClass()->IsSameWith(Class_Ptr2(T)))
			return (T*)upgrade;
		return NULL;
	}

	BEGIN_GOBJ(LevelRecordUpgrade,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"能力升级的名称");
		GELEM_VAR_INIT( StringID,desc,StringID_Invalid);	
			GELEM_EDITVAR( "描述", GVT_U, GSem(GSem_StringID,"能力升级描述"), "能力升级的描述文字" );

		GELEM_DYNOBJPTR_UPGRADE(CLevelUpgrade,upgrade,CUpgradeFire_Init, "能力升级类型", "选择不同的能力升级类型" );
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "000.N/A", CUpgrade_Void);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "001.[徒手攻击]基本设置", CUpgradeUnarmed_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "002.[火焰]基本设置", CUpgradeFire_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "003.[火焰]提升伤害", CUpgradeFire_IncDmg);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "004.[火焰]减少火晶消耗", CUpgradeFire_DecCost);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "005.[火焰]增加火球个数", CUpgradeFire_IncBullet);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "006.[乌图姆潮水]基本设置", CUpgradeUtumTide_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "007.1 [飞速斩]基本设置", CUpgradeFlashSwing_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "007.2 [飞速斩]武器升级", CUpgradeFlashSwing_LevelUp);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "008.1 [死亡召唤]基本设置", CUpgradeDeathCall_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "008.2 [死亡召唤]武器升级", CUpgradeDeathCall_LevelUp);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "009.[活力之甲]基本设置", CUpgradeArmorOfVigor_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "010.1[火球术]基本设置", CUpgradeFireBall_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "011.2[火球术]升级", CUpgradeFireBall_LevelUp);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "012.[活力之盾]基本设置", CUpgradeShieldOfVigor_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "013.1 [火焰刀]基本设置", CUpgradeFlameBlade_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "013.2 [火焰刀]武器升级", CUpgradeFlameBlade_LevelUp);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "014.1 [闪电弓]基本设置", CUpgradeLightningBow_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "014.2 [闪电弓]武器升级", CUpgradeLightningBow_LevelUp);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "015.1 [荣耀之剑]基本设置", CUpgradeHonorSword_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "015.2 [荣耀之剑]武器升级", CUpgradeHonorSword_LevelUp);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "016.1 [骷髅剑]基本设置", CUpgradeSkullSword_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "016.2 [骷髅剑]武器升级", CUpgradeSkullSword_LevelUp);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "017.1 [闪动剑]基本设置", CUpgradeTeleportSword_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "017.2 [闪动剑]武器升级", CUpgradeTeleportSword_LevelUp);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "018.1 [幻影匕首]基本设置", CUpgradePhantomDagger_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "018.2 [幻影匕首]武器升级", CUpgradePhantomDagger_LevelUp);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "019.1 [血牙剑]基本设置", CUpgradeBloodTeeth_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "019.2 [血牙剑]武器升级", CUpgradeBloodTeeth_LevelUp);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "020.1 [尸爆弓]基本设置", CUpgradeObliterateBow_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "020.2 [尸爆弓]武器升级", CUpgradeObliterateBow_LevelUp);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "020x.1 [无名之剑]基本设置", CUpgradeNameless_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "020x.2 [无名之剑]武器升级", CUpgradeNameless_LevelUp);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "021.[野狼皮]基本设置", CUpgradeWolfSkin_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "022.[塔尔的护佑]基本设置", CUpgradeTalBless_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "023.[安的啜泣]基本设置", CUpgradeAnWeep_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "024.[黑钢甲]基本设置", CUpgradeBlackSteel_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "025.[猎人护甲]基本设置", CUpgradeHunterPlate_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "026.[西姆的诅咒]基本设置", CUpgradeSimCurse_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "027.[旋风甲]基本设置", CUpgradeWhirlWind_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "028.[荣耀战甲]基本设置", CUpgradeHonorPlate_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "029.[七子甲]基本设置", CUpgrade7sonLeather_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "030.[寒霜甲]基本设置", CUpgradeFrost_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE( "031.[艾利的守护]基本设置", CUpgradeEliPromise_Init);
            GELEM_DYNOBJPTR_CLASS_UPGRADE("032.ExplodeOil", CUpgradeExplodeOil_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("033.[武器感应石]基本设置", CUpgradeWeaponInductionStone_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("034.[脚尖石]基本设置", CUpgradeToeStone_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("035.[圣箭]基本设置", CUpgradeSacredArrow_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("036.[炸弹]基本设置", CUpgradeBomb_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("037.[魔法戒指]基本设置", CUpgradeMagicRing_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("038.[钱包]基本设置", CUpgradeMoneyBag_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("039.[宝石罐]基本设置", CUpgradeGemPot_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("040.[HP项链]基本设置", CUpgradeHPAmulet_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("041.[SP项链]基本设置", CUpgradeSPAmulet_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("042.[HP药水]基本设置", CUpgradeHPPotion_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("043.[SP药水]基本设置", CUpgradeSPPotion_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("044.[吸血戒指]基本设置", CUpgradeVampireRing_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("045.[魔盾面具]基本设置", CUpgradeShieldMask_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("046.[磨刀石]基本设置", CUpgradeWhetstone_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("047.[荣耀之书]基本设置", CUpgradeHonorBook_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("048.[魔盾护身符]基本设置", CUpgradeShieldAmulet_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("049.[风暴之眼]基本设置", CUpgradeStormEye_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("050.[生命宝瓶]基本设置", CUpgradeHPFlusk_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("051.[圣诗卷轴I]基本设置", CUpgradePoemChartI_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("052.[圣诗卷轴II]基本设置", CUpgradePoemChartII_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("053.[圣诗卷轴III]基本设置", CUpgradePoemChartIII_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("054.[圣旗-火焰]基本设置", CUpgradeBannerFire_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("055.[圣旗-生命之狼]基本设置", CUpgradeBannerWolf_Init);
			GELEM_DYNOBJPTR_CLASS_UPGRADE("056.[特殊-推动滑槽]基本设置", CUpgradePushSlideway_Init);

//XXXXX:More Ability
//XXXXX:More LevelSkill
	END_GOBJ();


};

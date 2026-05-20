#pragma once

#include "class/class.h"

#include "gds/GObjUID.h"

#include "anim/animdefines.h"

#include "LevelDefines.h"

enum DamageModFilter
{
	DamageModFilter_RootOwner=1,
};

enum DamageAttrType
{
	DamageAttrType_Pierce=0,	
	DamageAttrType_Crush,
	DamageAttrType_Fire,
	DamageAttrType_Lightning,
	DamageAttrType_Cold,
	DamageAttrType_Poison,
	DamageAttrType_CryticalBlocking,
	DamageAttrType_SpecialA,
	DamageAttrType_Explosion,
	DamageAttrType_Smash,
	//注意新加的类型必须加在最下面
	//不能超过15个
	//XXXXX: More DamageAttrType

	DamageAttrType_Max,
};
typedef DWORD DamageAttrMask;

#define DamageAttrMask_SemConstraint_Weaks "穿刺:1,重击:2,火:4,电:8,冰:16,毒:32,爆炸:256,Crytical格挡:64,特殊A:128"


enum DmgBlockType
{
	DmgBlockType_NotBlockable,//无法格挡
	DmgBlockType_Reflectible,//可以格挡并反弹伤害
	DmgBlockType_Blockable,//可以格挡(不能反弹伤害)

	DmgBlockType_ForceDword=0xffffffff
};

#define GSemConstraint_DmgBlockType "无法格挡,可以格挡并反弹伤害,可以格挡不能反弹伤害"



//////////////////////////////////////////////////////////////////////////
//DefendMod

struct DefendMod
{
	DefendMod()
	{
		memset(this,0,sizeof(*this));
	}

	short bImmune;
	short defBase;
	short defAdd;
	float defRate;
};

struct LevelAttr_DefendMods
{
	DEFINE_CLASS(LevelAttr_DefendMods);
	LevelAttr_DefendMods()
	{
		Zero();
	}
	void Zero()
	{
		memset(modsResist,0,sizeof(modsResist));
		memset(&modEvade,0,sizeof(modEvade));
	}

	void CopyFrom(LevelAttr_DefendMods &src)
	{
		memcpy(&modsResist[0], &src.modsResist[0], sizeof(modsResist));
		modEvade=src.modEvade;
	}

	DefendMod modEvade;
	DefendMod modsResist[DamageAttrType_Max];

};

struct DefendModEx
{
	void ToDefendMod(DefendMod &mod)
	{
		mod.bImmune=bImmune;
		mod.defBase=defBase;
		mod.defAdd=defAdd;
		mod.defRate=defRate;
	}

	BOOL bImmune;
	int defBase;
	int defAdd;
	float defRate;

	BEGIN_GOBJ_PURE_UID(DefendModEx,1);
		GELEM_VAR_INIT(BOOL,bImmune,0);
			GELEM_EDITVAR("免疫与否",GVT_S,GSem(GSem_Interger,
				"免疫:1"		"|基础值&增加值&增加比率,"
				"不免疫:0"	""
				),"是否免疫");
		GELEM_VAR_INIT(int,defBase,0);
			GELEM_EDITVAR("基础值",GVT_S,GSem_Interger,"基础值");
		GELEM_VAR_INIT(int,defAdd,10);
			GELEM_EDITVAR("增加值",GVT_S,GSem_Interger,"增加值");
		GELEM_VAR_INIT(float,defRate,0.0f);
			GELEM_EDITVAR("增加比率",GVT_F,GSem(GSem_Float,"0,5,0.01"),"增加比率");
	END_GOBJ();
};



struct DefendModsEx
{
	void ToDefendMods(LevelAttr_DefendMods &mods)
	{
		mods.Zero();

		if (flags&0x8000)
			evade.ToDefendMod(mods.modEvade);

		if (flags&(1<<DamageAttrType_Pierce))
		{
			physdmg.ToDefendMod(mods.modsResist[DamageAttrType_Pierce]);
			physdmg.ToDefendMod(mods.modsResist[DamageAttrType_Crush]);
		}
		if (flags&(1<<DamageAttrType_Fire))
			fire.ToDefendMod(mods.modsResist[DamageAttrType_Fire]);
		if (flags&(1<<DamageAttrType_Lightning))
			lightning.ToDefendMod(mods.modsResist[DamageAttrType_Lightning]);
		if (flags&(1<<DamageAttrType_Cold))
			cold.ToDefendMod(mods.modsResist[DamageAttrType_Cold]);
		if (flags&(1<<DamageAttrType_Poison))
			poison.ToDefendMod(mods.modsResist[DamageAttrType_Poison]);
		if (flags&(1<<DamageAttrType_Explosion))
			explosion.ToDefendMod(mods.modsResist[DamageAttrType_Explosion]);
		if (flags&(1<<DamageAttrType_Smash))
			smash.ToDefendMod(mods.modsResist[DamageAttrType_Smash]);
		//XXXXX: More DamageAttrType
	}

	DWORD flags;

	DefendModEx evade;
	DefendModEx physdmg;
	DefendModEx fire;
	DefendModEx lightning;
	DefendModEx cold;
	DefendModEx poison;
	DefendModEx explosion;
	DefendModEx smash;
	//XXXXX: More DamageAttrType


	BEGIN_GOBJ_PURE_UID(DefendModsEx,1);

		GELEM_VAR_INIT(DWORD,flags,0)
			GELEM_EDITVAR("伤害类型",GVT_U,GSem(GSem_Flags,
			"闪避|闪避:32768,"//0x8000
			"物理伤害|物理伤害:1,"
			"火系伤害|火系伤害:4,"
			"电系伤害|电系伤害:8,"
			"冰冻伤害|冰冻伤害:16,"
			"毒伤害|毒伤害:32,"
			"爆炸伤害|爆炸伤害:256,"
			"砸碎|砸碎:1024"
			),"伤害类型");
		//XXXXX: More DamageAttrType

		GELEM_OBJ(DefendModEx,evade);
			GELEM_EDITOBJ("闪避","闪避");		
		GELEM_OBJ(DefendModEx,physdmg);
			GELEM_EDITOBJ("物理伤害","物理伤害");		
		GELEM_OBJ(DefendModEx,fire);
			GELEM_EDITOBJ("火系伤害","火系伤害");		
		GELEM_OBJ(DefendModEx,lightning);
			GELEM_EDITOBJ("电系伤害","电系伤害");		
		GELEM_OBJ(DefendModEx,cold);
			GELEM_EDITOBJ("冰冻伤害","冰冻伤害");		
		GELEM_OBJ(DefendModEx,poison);
			GELEM_EDITOBJ("毒伤害","毒伤害");		
		GELEM_OBJ(DefendModEx,explosion);
			GELEM_EDITOBJ("爆炸伤害","爆炸伤害");		
		GELEM_OBJ(DefendModEx,smash);
			GELEM_EDITOBJ("砸碎","砸碎");		
	END_GOBJ();

	//XXXXX: More DamageAttrType
	LevelAttr_DefendMods *Get()
	{
		if (!cache.bValid)
		{
			ToDefendMods(cache);
			cache.bValid=TRUE;
		}
		return &cache;
	}

	struct Cache:public LevelAttr_DefendMods
	{
		Cache()
		{
			bValid=FALSE;
		}
		BOOL bValid;
	};

	Cache cache;
};


//////////////////////////////////////////////////////////////////////////
//AttackMod

struct AttackMod
{
	AttackMod()
	{
		memset(this,0,sizeof(*this));
	}

	void MergeFrom(AttackMod &other,int nRepeat=1)
	{
		atkAdd+=other.atkAdd*nRepeat;
		atkRate+=other.atkRate*(float)nRepeat;
	}

	short atkAdd;
	float atkRate;//0.1表示10%
};



struct LevelAttr_AttackMods
{
	DEFINE_CLASS(LevelAttr_AttackMods);
	LevelAttr_AttackMods()
	{
		Zero();
	}
	void Zero()
	{
		memset(modsDamage,0,sizeof(modsDamage));
		memset(&modsHit,0,sizeof(modsHit));
	}

	void CopyFrom(LevelAttr_AttackMods &src)
	{
		memcpy(&modsDamage[0], &src.modsDamage[0], sizeof(modsDamage));
		modsHit=src.modsHit;
	}

	void MergeFrom(LevelAttr_AttackMods &src,int nRepeat=1)
	{
		for (int i=0;i<DamageAttrType_Max;i++)
			modsDamage[i].MergeFrom(src.modsDamage[i],nRepeat);

		modsHit.MergeFrom(src.modsHit,nRepeat);
	}

	AttackMod modsHit;
	AttackMod modsDamage[DamageAttrType_Max];
};

struct AttackModEx
{
	void ToAttackMod(AttackMod &mod)
	{
		mod.atkAdd=atkAdd;
		mod.atkRate=atkRate;
	}

	int atkAdd;
	float atkRate;

	BEGIN_GOBJ_PURE_UID(AttackModEx,1);
		GELEM_VAR_INIT(int,atkAdd,10);
			GELEM_EDITVAR("增加值",GVT_S,GSem_Interger,"增加值");
		GELEM_VAR_INIT(float,atkRate,0.0f);
			GELEM_EDITVAR("增加比率",GVT_F,GSem(GSem_Float,"0,5,0.01"),"增加比率");
	END_GOBJ();
};

struct AttackModsEx
{
	void ToAttackMods(LevelAttr_AttackMods &mods)
	{
		mods.Zero();

		if (flags&0x8000)
			hit.ToAttackMod(mods.modsHit);

		if (flags&(1<<DamageAttrType_Pierce))
			pierce.ToAttackMod(mods.modsDamage[DamageAttrType_Pierce]);
		if (flags&(1<<DamageAttrType_Crush))
			crush.ToAttackMod(mods.modsDamage[DamageAttrType_Crush]);
		if (flags&(1<<DamageAttrType_Fire))
			fire.ToAttackMod(mods.modsDamage[DamageAttrType_Fire]);
		if (flags&(1<<DamageAttrType_Lightning))
			lightning.ToAttackMod(mods.modsDamage[DamageAttrType_Lightning]);
		if (flags&(1<<DamageAttrType_Cold))
			cold.ToAttackMod(mods.modsDamage[DamageAttrType_Cold]);
		if (flags&(1<<DamageAttrType_Poison))
			poison.ToAttackMod(mods.modsDamage[DamageAttrType_Poison]);
		if (flags&(1<<DamageAttrType_Explosion))
			explosion.ToAttackMod(mods.modsDamage[DamageAttrType_Explosion]);
		if (flags&(1<<DamageAttrType_Smash))
			smash.ToAttackMod(mods.modsDamage[DamageAttrType_Smash]);
		//XXXXX: More DamageAttrType
	}

	DWORD flags;

	AttackModEx hit;
	AttackModEx pierce;
	AttackModEx crush;
	AttackModEx fire;
	AttackModEx lightning;
	AttackModEx cold;
	AttackModEx poison;
	AttackModEx explosion;
	AttackModEx smash;
	//XXXXX: More DamageAttrType

	BEGIN_GOBJ_PURE_UID(AttackModsEx,1);

		GELEM_VAR_INIT(DWORD,flags,0)
			GELEM_EDITVAR("伤害类型",GVT_U,GSem(GSem_Flags,
			"命中|命中:32768,"//0x8000
			"穿刺伤害|穿刺伤害:1,"
			"重击伤害|重击伤害:2,"
			"火系伤害|火系伤害:4,"
			"电系伤害|电系伤害:8,"
			"冰冻伤害|冰冻伤害:16,"
			"毒伤害|毒伤害:32,"
			"爆炸伤害|爆炸伤害:256,"
			"砸碎|砸碎:1024"
			),"伤害类型");
		//XXXXX: More DamageAttrType

		GELEM_OBJ(AttackModEx,hit);
			GELEM_EDITOBJ("命中","命中");		
		GELEM_OBJ(AttackModEx,pierce);
			GELEM_EDITOBJ("穿刺伤害","穿刺伤害");		
		GELEM_OBJ(AttackModEx,crush);
			GELEM_EDITOBJ("重击伤害","重击伤害");		
		GELEM_OBJ(AttackModEx,fire);
			GELEM_EDITOBJ("火系伤害","火系伤害");		
		GELEM_OBJ(AttackModEx,lightning);
			GELEM_EDITOBJ("电系伤害","电系伤害");		
		GELEM_OBJ(AttackModEx,cold);
			GELEM_EDITOBJ("冰冻伤害","冰冻伤害");		
		GELEM_OBJ(AttackModEx,poison);
			GELEM_EDITOBJ("毒伤害","毒伤害");		
		GELEM_OBJ(AttackModEx,explosion);
			GELEM_EDITOBJ("爆炸伤害","爆炸伤害");		
		GELEM_OBJ(AttackModEx,smash);
			GELEM_EDITOBJ("砸碎","砸碎");		
	END_GOBJ();
	//XXXXX: More DamageAttrType

	LevelAttr_AttackMods *Get()
	{
		if (!cache.bValid)
		{
			ToAttackMods(cache);
			cache.bValid=TRUE;
		}
		return &cache;
	}

	struct Cache:public LevelAttr_AttackMods
	{
		Cache()
		{
			bValid=FALSE;
		}
		BOOL bValid;
	};

	Cache cache;
};




//////////////////////////////////////////////////////////////////////////
//Resist

struct Resist
{
	Resist()
	{
		memset(this,0,sizeof(*this));
	}

	WORD def:15;
	WORD bImmune:1;
};

struct LevelAttr_Resists
{
	Resist resists[DamageAttrType_Max];

	Resist Resolve(LevelAttr_DefendMods *mods,DamageAttrType tp)
	{
		if (tp>=DamageAttrType_Max)
			return Resist();//empty
		if (!mods)
			return resists[tp];

		Resist &resist=resists[tp];
		DefendMod &mod=mods->modsResist[tp];

		Resist ret;
		ret=resist;
		ret.def=(WORD)((1.0f+mod.defRate)*(float)(resist.def+mod.defBase)+(float)mod.defAdd);
		if (mod.bImmune)
			ret.bImmune=1;

		return ret;
	}
};

struct ResistEx
{
	void ToResist(Resist &resist)
	{
		resist.bImmune=bImmune;
		resist.def=def;
	}

	WORD def;
	BOOL bImmune;

	BEGIN_GOBJ_PURE_UID(ResistEx,1);
		GELEM_VAR_INIT(BOOL,bImmune,0);
			GELEM_EDITVAR("免疫与否",GVT_S,GSem(GSem_Interger,
				"免疫:1"		"|防,"
				"不免疫:0"	""
				),"是否免疫");
		GELEM_VAR_INIT(WORD,def,10);
			GELEM_EDITVAR("防",GVT_SU,GSem_Interger,"抵抗值");
	END_GOBJ();
};

struct ResistsEx
{
	void ToResists(LevelAttr_Resists &resists)
	{
		pierce.ToResist(resists.resists[DamageAttrType_Pierce]);
		crush.ToResist(resists.resists[DamageAttrType_Crush]);
		fire.ToResist(resists.resists[DamageAttrType_Fire]);
		lightning.ToResist(resists.resists[DamageAttrType_Lightning]);
		cold.ToResist(resists.resists[DamageAttrType_Cold]);
		poison.ToResist(resists.resists[DamageAttrType_Poison]);
		explosion.ToResist(resists.resists[DamageAttrType_Explosion]);
		smash.ToResist(resists.resists[DamageAttrType_Smash]);
		//XXXXX: More DamageAttrType
	}
	ResistEx pierce;
	ResistEx crush;
	ResistEx fire;
	ResistEx lightning;
	ResistEx cold;
	ResistEx poison;
	ResistEx explosion;
	ResistEx smash;
	//XXXXX: More DamageAttrType

	BEGIN_GOBJ_PURE_UID(ResistsEx,1);
		GELEM_OBJ(ResistEx,pierce);
			GELEM_EDITOBJ("穿刺伤害","穿刺伤害");		
		GELEM_OBJ(ResistEx,crush);
			GELEM_EDITOBJ("重击伤害","重击伤害");		
		GELEM_OBJ(ResistEx,fire);
			GELEM_EDITOBJ("火系伤害","火系伤害");		
		GELEM_OBJ(ResistEx,lightning);
			GELEM_EDITOBJ("电系伤害","电系伤害");		
		GELEM_OBJ(ResistEx,cold);
			GELEM_EDITOBJ("冰冻伤害","冰冻伤害");		
		GELEM_OBJ(ResistEx,poison);
			GELEM_EDITOBJ("毒伤害","毒伤害");		
		GELEM_OBJ(ResistEx,explosion);
			GELEM_EDITOBJ("爆炸伤害","爆炸伤害");		
		GELEM_OBJ(ResistEx,smash);
			GELEM_EDITOBJ("砸碎","砸碎");		
	END_GOBJ();
	//XXXXX: More DamageAttrType

	LevelAttr_Resists *Get()
	{
		if (!cache.bValid)
		{
			ToResists(cache);
			cache.bValid=TRUE;
		}
		return &cache;
	}

	struct Cache:public LevelAttr_Resists
	{
		Cache()
		{
			bValid=FALSE;
		}
		BOOL bValid;
	};

	Cache cache;
};



//////////////////////////////////////////////////////////////////////////
//Attack Attrs

//////////////////////////////////////////////////////////////////////////
//Damage
struct Damage
{
	Damage()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL IsZero()
	{
		return (lo==0) && (hi==0);
	}

	WORD lo;
	WORD hi;
};

struct LevelAttr_Damages
{
	DWORD filtersMod;
	Damage damages[DamageAttrType_Max];

	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	
	Damage Resolve(LevelAttr_AttackMods *mods,DamageAttrType tp)
	{
		if (tp>=DamageAttrType_Max)
			return Damage();//empty
		if (!mods)
			return damages[tp];

		Damage &dmg=damages[tp];
		AttackMod &mod=mods->modsDamage[tp];

		Damage ret;
		ret.lo=(WORD)((1.0f+mod.atkRate)*(float)(dmg.lo+mod.atkAdd));
		ret.hi=(WORD)((1.0f+mod.atkRate)*(float)(dmg.hi+mod.atkAdd));

		return ret;
	}

};


struct DamageEx
{
	void ToDamage(Damage &atk)
	{
		atk.lo=lo;
		atk.hi=hi;
	}

	void ResetFloat(float v)
	{
		lo=hi=FloatToNearestInt(v);
	}

	void Scale(float r)
	{
		lo=FloatToNearestInt(r*(float)lo);
		hi=FloatToNearestInt(r*(float)hi);
	}

	WORD lo;
	WORD hi;

	BEGIN_GOBJ_PURE_UID(DamageEx,1);
		GELEM_VAR_INIT(WORD,lo,10);
			GELEM_EDITVAR("最小值",GVT_SU,GSem_Interger,"最小值");
		GELEM_VAR_INIT(WORD,hi,10);
			GELEM_EDITVAR("最大值",GVT_SU,GSem_Interger,"最大值");
	END_GOBJ();
};

struct DamagesEx
{
	void ToDamages(LevelAttr_Damages &dmgs)
	{
		dmgs.Zero();

		dmgs.filtersMod=filtersMod;

		if (flags&(1<<DamageAttrType_Pierce))
			pierce.ToDamage(dmgs.damages[DamageAttrType_Pierce]);
		if (flags&(1<<DamageAttrType_Crush))
			crush.ToDamage(dmgs.damages[DamageAttrType_Crush]);
		if (flags&(1<<DamageAttrType_Fire))
			fire.ToDamage(dmgs.damages[DamageAttrType_Fire]);
		if (flags&(1<<DamageAttrType_Lightning))
			lightning.ToDamage(dmgs.damages[DamageAttrType_Lightning]);
		if (flags&(1<<DamageAttrType_Cold))
			cold.ToDamage(dmgs.damages[DamageAttrType_Cold]);
		if (flags&(1<<DamageAttrType_Poison))
			poison.ToDamage(dmgs.damages[DamageAttrType_Poison]);
		if (flags&(1<<DamageAttrType_Explosion))
			explosion.ToDamage(dmgs.damages[DamageAttrType_Explosion]);
		if (flags&(1<<DamageAttrType_Smash))
			smash.ToDamage(dmgs.damages[DamageAttrType_Smash]);
		if (flags&(1<<DamageAttrType_CryticalBlocking))
			cryticalblocking.ToDamage(dmgs.damages[DamageAttrType_CryticalBlocking]);
		//XXXXX: More DamageAttrType
	}

	DWORD flags;
	DamageEx pierce;
	DamageEx crush;
	DamageEx fire;
	DamageEx lightning;
	DamageEx cold;
	DamageEx poison;
	DamageEx explosion;
	DamageEx cryticalblocking;
	DamageEx smash;
	//XXXXX: More DamageAttrType

	DWORD filtersMod;

	BEGIN_GOBJ_PURE_UID(DamagesEx,1);

		GELEM_VAR_INIT(DWORD,filtersMod,0xffffffff)
			GELEM_EDITVAR("允许哪些类型的mod",GVT_U,GSem(GSem_Flags,
			"来自RootOwner的Mode:1"
			),"允许哪些类型的mod");


		GELEM_VAR_INIT(DWORD,flags,0)
			GELEM_EDITVAR("伤害类型",GVT_U,GSem(GSem_Flags,
			"穿刺伤害|穿刺伤害:1,"
			"重击伤害|重击伤害:2,"
			"火系伤害|火系伤害:4,"
			"电系伤害|电系伤害:8,"
			"冰冻伤害|冰冻伤害:16,"
			"毒伤害|毒伤害:32,"
			"爆炸伤害|爆炸伤害:256,"
			"砸碎|砸碎:1024,"
			"CryticalBlocking|CryticalBlocking:64"
			),"伤害类型");
		//XXXXX: More DamageAttrType

		GELEM_OBJ(DamageEx,pierce);
			GELEM_EDITOBJ("穿刺伤害","穿刺伤害");		
		GELEM_OBJ(DamageEx,crush);
			GELEM_EDITOBJ("重击伤害","重击伤害");		
		GELEM_OBJ(DamageEx,fire);
			GELEM_EDITOBJ("火系伤害","火系伤害");		
		GELEM_OBJ(DamageEx,lightning);
			GELEM_EDITOBJ("电系伤害","电系伤害");		
		GELEM_OBJ(DamageEx,cold);
			GELEM_EDITOBJ("冰冻伤害","冰冻伤害");		
		GELEM_OBJ(DamageEx,poison);
			GELEM_EDITOBJ("毒伤害","毒伤害");		
		GELEM_OBJ(DamageEx,explosion);
			GELEM_EDITOBJ("爆炸伤害","爆炸伤害");		
		GELEM_OBJ(DamageEx,smash);
			GELEM_EDITOBJ("砸碎","砸碎");		
		GELEM_OBJ(DamageEx,explosion);
			GELEM_EDITOBJ("爆炸伤害","爆炸伤害");		
		GELEM_OBJ(DamageEx,cryticalblocking);
			GELEM_EDITOBJ("CryticalBlocking","CryticalBlocking");		
	END_GOBJ();
	//XXXXX: More DamageAttrType

	void DiscardCache()
	{
		cache.bValid=FALSE;
	}


	LevelAttr_Damages *Get()
	{
		if (!cache.bValid)
		{
			ToDamages(cache);
			cache.bValid=TRUE;
		}
		return &cache;
	}

	struct Cache:public LevelAttr_Damages
	{
		Cache()
		{
			bValid=FALSE;
		}
		BOOL bValid;
	};

	Cache cache;
};


struct LevelAttr_Evade
{
	DWORD evade:31;
	DWORD bImmune:1;

	void Zero()
	{
		memset(this,0,sizeof(*this));
	}

	LevelAttr_Evade Resolve(LevelAttr_DefendMods *mods)
	{
		if (!mods)
			return (*this);

		DefendMod &mod=mods->modEvade;

		LevelAttr_Evade ret;
		ret=(*this);
		ret.evade=(DWORD)((1.0f+mod.defRate)*(float)(evade+mod.defAdd));
		if (mod.bImmune)
			ret.bImmune=1;

		return ret;
	}

};

struct EvadeEx
{
	DWORD evade;
	BOOL bImmune;

	void ToEvade(LevelAttr_Evade &evade_)
	{
		evade_.evade=evade;
		evade_.bImmune=bImmune;
	}

	BEGIN_GOBJ_PURE_UID(EvadeEx,1);
		GELEM_VAR_INIT(BOOL,bImmune,0);
			GELEM_EDITVAR("免疫与否",GVT_S,GSem(GSem_Interger,
				"免疫:1"		"|闪避值,"
				"不免疫:0"	""
				),"是否免疫");
		GELEM_VAR_INIT(DWORD,evade,10);
			GELEM_EDITVAR("闪避值",GVT_U,GSem_Interger,"闪避值");
	END_GOBJ();

	LevelAttr_Evade *Get()
	{
		if (!cache.bValid)
		{
			ToEvade(cache);
			cache.bValid=TRUE;
		}
		return &cache;
	}

	struct Cache:public LevelAttr_Evade
	{
		Cache()
		{
			bValid=FALSE;
		}
		BOOL bValid;
	};

	Cache cache;
};

struct LevelAttr_Hit
{
	DWORD bValid:1;
	DWORD bIgnoreEvade:1;
	DWORD hit:30;

	void Zero()
	{
		memset(this,0,sizeof(*this));
	}

	LevelAttr_Hit Resolve(LevelAttr_AttackMods *mods)
	{
		if (!mods)
			return (*this);

		AttackMod &mod=mods->modsHit;

		LevelAttr_Hit ret;
		ret=(*this);
		if (ret.bValid)
			ret.hit=(DWORD)((1.0f+mod.atkRate)*(float)(hit+mod.atkAdd));

		return ret;
	}

};

struct HitEx
{
	BOOL bValid;
	BOOL bIgnoreEvade;
	DWORD hit;

	void ToHit(LevelAttr_Hit&hit_)
	{
		hit_.hit=hit;
		hit_.bIgnoreEvade=bIgnoreEvade;
		hit_.bValid=bValid;
	}
	BEGIN_GOBJ_PURE_UID(HitEx,1);
		GELEM_VAR_INIT(BOOL,bValid,1);
			GELEM_EDITVAR("是否有效",GVT_S,GSem(GSem_Interger,
				"是:1"		","
				"否:0"	"|是否必中&命中值"
				),"是否必中");
		GELEM_VAR_INIT(BOOL,bIgnoreEvade,0);
			GELEM_EDITVAR("是否必中",GVT_S,GSem(GSem_Interger,
				"是:1"		"|命中值,"
				"否:0"	""
				),"是否必中");
		GELEM_VAR_INIT(DWORD,hit,10);
			GELEM_EDITVAR("命中值",GVT_U,GSem_Interger,"命中值");
	END_GOBJ();

	void DiscardCache()
	{
		cache.bValid=FALSE;
	}

	LevelAttr_Hit *Get()
	{
		if (!cache.bValid)
		{
			ToHit(cache);
			cache.bValid=TRUE;
		}
		return &cache;
	}

	struct Cache:public LevelAttr_Hit
	{
		Cache()
		{
			bValid=FALSE;
		}
		BOOL bValid;
	};

	Cache cache;
};

struct LevelAttr_Blocking
{
	LevelAttr_Blocking()
	{
		Zero();
	}
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	float converts[DamageAttrType_Max];
};

struct BlockingConvertsEx
{
	BEGIN_GOBJ_PURE_UID(BlockingConvertsEx,1);

		GELEM_VAR_INIT(DWORD,flags,0)
			GELEM_EDITVAR("伤害类型",GVT_U,GSem(GSem_Flags,
			"穿刺伤害|穿刺伤害转化率:1,"
			"重击伤害|重击伤害转化率:2,"
			"火系伤害|火系伤害转化率:4,"
			"电系伤害|电系伤害转化率:8,"
			"冰冻伤害|冰冻伤害转化率:16,"
			"毒伤害|毒伤害转化率:32,"
			"爆炸伤害|爆炸伤害转化率:256,"
			"砸碎伤害|砸碎伤害转化率:2048"
			),"伤害类型");
		//XXXXX: More DamageAttrType

		GELEM_VAR_INIT(float,pierce,0.0f);
			GELEM_EDITVAR("穿刺伤害转化率",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"穿刺伤害转化率");
		GELEM_VAR_INIT(float,crush,0.0f);
			GELEM_EDITVAR("重击伤害转化率",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"重击伤害转化率");
		GELEM_VAR_INIT(float,fire,0.0f);
			GELEM_EDITVAR("火系伤害转化率",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"火系伤害转化率");
		GELEM_VAR_INIT(float,lightning,0.0f);
			GELEM_EDITVAR("电系伤害转化率",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"电系伤害转化率");
		GELEM_VAR_INIT(float,cold,0.0f);
			GELEM_EDITVAR("冰冻伤害转化率",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"冰冻伤害转化率");
		GELEM_VAR_INIT(float,poison,0.0f);
			GELEM_EDITVAR("毒伤害转化率",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"毒伤害转化率");
		GELEM_VAR_INIT(float,explosion,0.0f);
			GELEM_EDITVAR("爆炸伤害转化率",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"爆炸伤害转化率");
		GELEM_VAR_INIT(float,smash,0.0f);
			GELEM_EDITVAR("砸碎伤害转化率",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"砸碎伤害转化率");
	END_GOBJ();
	//XXXXX: More DamageAttrType

	DWORD flags;
	float pierce;
	float crush;
	float fire;
	float lightning;
	float cold;
	float poison;
	float explosion;
	float smash;
	//XXXXX: More DamageAttrType

	void Apply(LevelAttr_Blocking &attrBlocking)
	{
		if (flags&(1<<DamageAttrType_Pierce))
			attrBlocking.converts[DamageAttrType_Pierce]=pierce;
		if (flags&(1<<DamageAttrType_Crush))
			attrBlocking.converts[DamageAttrType_Crush]=crush;
		if (flags&(1<<DamageAttrType_Fire))
			attrBlocking.converts[DamageAttrType_Fire]=fire;
		if (flags&(1<<DamageAttrType_Lightning))
			attrBlocking.converts[DamageAttrType_Lightning]=lightning;
		if (flags&(1<<DamageAttrType_Cold))
			attrBlocking.converts[DamageAttrType_Cold]=cold;
		if (flags&(1<<DamageAttrType_Poison))
			attrBlocking.converts[DamageAttrType_Poison]=poison;
		if (flags&(1<<DamageAttrType_Explosion))
			attrBlocking.converts[DamageAttrType_Explosion]=explosion;
		if (flags&(1<<DamageAttrType_Smash))
			attrBlocking.converts[DamageAttrType_Smash]=smash;
		//XXXXX: More DamageAttrType
	}
};

struct BlockingEx
{
	void ToBlocking(LevelAttr_Blocking &blocking)
	{
		blocking.Zero();

		converts.Apply(blocking);
	}

	BlockingConvertsEx converts;


	BEGIN_GOBJ_PURE_UID(BlockingEx,1);
		GELEM_OBJ(BlockingConvertsEx,converts);
			GELEM_EDITOBJ("伤害转化率","将伤害值转换为体力消耗的转化率");
	END_GOBJ();
	//XXXXX: More DamageAttrType

	void DiscardCache()
	{
		cache.bValid=FALSE;
	}


	LevelAttr_Blocking *Get()
	{
		if (!cache.bValid)
		{
			ToBlocking(cache);
			cache.bValid=TRUE;
		}
		return &cache;
	}

	struct Cache:public LevelAttr_Blocking
	{
		Cache()
		{
			bValid=FALSE;
		}
		BOOL bValid;
	};

	Cache cache;
};

struct DamageResult
{
	DamageResult()
	{
		Zero();
	}
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL IsEmpty()	{		return maskHP==0&&sp<=0.0f;	}
	float GetHPTotal()
	{
		float total=0.0f;
		for (int i=0;i<DamageAttrType_Max;i++)
		{
			if (maskHP&(DamageAttrMask)(1<<i))
				total+=bufHP[i];
		}
		return total;
	}
	int GetHPTotal_Int()
	{
		return FloatToNearestInt(GetHPTotal());
	}
	float GetSPTotal()
	{
		return sp;
	}

	void ApplyHPScale(float scale)
	{
		for (int i=0;i<DamageAttrType_Max;i++)
		{
			if (maskHP&(DamageAttrMask)(1<<i))
				bufHP[i]*=scale;
		}
	}

	void ApplyHPOverride(float vOverride)
	{
		ApplyHPScale(vOverride/GetHPTotal());
	}

	void ApplHPDelta(float delta)
	{
		float total=GetHPTotal();
		ApplyHPScale((total+delta)/total);
	}


	//HP
	DamageAttrMask maskHP;
	float bufHP[DamageAttrType_Max];

	//SP
	float sp;
};
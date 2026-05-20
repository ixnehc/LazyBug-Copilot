#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelRecordSkill;
class CLevelAbility_Fire:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_Fire,LevelAbilityType_Fire);

	CLevelAbility_Fire()
	{
		GConstructor();
	}

	~CLevelAbility_Fire()
	{
		GDestructor();
	}

	BEGIN_GOBJ_UID(CLevelAbility_Fire,1);

		GELEM_ABILITY_BASE();

		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);GELEM_UID(1)
		GELEM_VAR_INIT(int,_cost,0);GELEM_UID(2)
		GELEM_VAR_INIT(int,_attackFire,0);GELEM_UID(3)
		GELEM_VAR_INIT(int,_nBullets,1);GELEM_UID(4)

	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *recordsSkill) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;

public://Take it as protected
	virtual void _InitTechs()	 override{	}


	RecordID _idSkill;
	int _cost;
	int _attackFire;
	int _nBullets;

};


class CUpgradeFire_Init:public CLevelAbilityInitial
{
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeFire_Init,LevelAbilityType_Fire);

	BEGIN_GOBJ_PURE(CUpgradeFire_Init,1);
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_VAR_INIT(int,cost,0);
			GELEM_EDITVAR("火晶消耗",GVT_S,GSem_Interger,"火晶消耗");
		GELEM_VAR_INIT(int,attackFire,0);
			GELEM_EDITVAR("火伤害",GVT_S,GSem_Interger,"火伤害");
	END_GOBJ();

	virtual BOOL Init(CLevelAbility *ability);

	RecordID idSkill;
	int cost;
	int attackFire;

};


class CUpgradeFire_IncDmg:public CLevelAbilityUpgrade
{
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeFire_IncDmg,LevelAbilityType_Fire);

	BEGIN_GOBJ_PURE(CUpgradeFire_IncDmg,1);
		GELEM_VAR_INIT(int,dmgMin,1);
			GELEM_EDITVAR("最小值",GVT_S,GSem_Interger,"最小值");
		GELEM_VAR_INIT(int,dmgMax,1);
			GELEM_EDITVAR("最大值",GVT_S,GSem_Interger,"最大值");
	END_GOBJ();

	int dmgMin;
	int dmgMax;

	virtual BOOL CanUpgrade(CLevelAbility *ability)
	{
		return TRUE;
	}
	virtual void MakeSeed(CLevelAbility *ability,LevelAwardSeed &seed)
	{
		seed.v1=CSysRandom::RandRangeInt(dmgMin,dmgMax);
	}

	virtual void Upgrade(CLevelAbility *ability0,LevelAwardSeed &seed)
	{
		CLevelAbility_Fire *ability=(CLevelAbility_Fire *)ability0;
		ability->_attackFire+=seed.v1;

	}
	virtual const char *GetDesc(LevelAwardSeed &seed,CLevelRecords *records)
	{
		static std::string s;
		FormatString(s,"火伤害+%d",seed.v1);
		return s.c_str();
	}

protected:


};

class CUpgradeFire_DecCost:public CLevelAbilityUpgrade
{
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeFire_DecCost,LevelAbilityType_Fire);

	BEGIN_GOBJ_PURE(CUpgradeFire_DecCost,1);

		GELEM_VAR_INIT(int,costMin,1);
			GELEM_EDITVAR("最小值",GVT_S,GSem_Interger,"最小值");
		GELEM_VAR_INIT(int,costMax,1);
			GELEM_EDITVAR("最大值",GVT_S,GSem_Interger,"最大值");

	END_GOBJ();

	int costMin;
	int costMax;


	virtual BOOL CanUpgrade(CLevelAbility *ability0)
	{
		CLevelAbility_Fire *ability=(CLevelAbility_Fire *)ability0;

		if (ability->_cost<=0)
			return FALSE;

		return TRUE;
	}
	virtual void Upgrade(CLevelAbility *ability0,LevelAwardSeed &seed)
	{
		CLevelAbility_Fire *ability=(CLevelAbility_Fire *)ability0;
		if (ability->_cost>seed.v1)
			ability->_cost-=seed.v1;
		else
			ability->_cost=0;
	}

	virtual void MakeSeed(CLevelAbility *ability0,LevelAwardSeed &seed)
	{
		CLevelAbility_Fire *ability=(CLevelAbility_Fire *)ability0;

		seed.v1=CSysRandom::RandRangeInt(costMin,costMax);
		if (seed.v1>ability->_cost)
		{
			seed.v1=ability->_cost;
		}
	}

	virtual const char *GetDesc(LevelAwardSeed &seed,CLevelRecords *records)
	{
		static std::string s;
		FormatString(s,"火晶消耗-%d",seed.v1);
		return s.c_str();
	}


};


class CUpgradeFire_IncBullet:public CLevelAbilityUpgrade
{
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeFire_IncBullet,LevelAbilityType_Fire);

	BEGIN_GOBJ_PURE(CUpgradeFire_IncBullet,1);


	END_GOBJ();



	virtual BOOL CanUpgrade(CLevelAbility *ability0)
	{
		CLevelAbility_Fire *ability=(CLevelAbility_Fire *)ability0;

		return TRUE;
	}
	virtual void Upgrade(CLevelAbility *ability0,LevelAwardSeed &seed)
	{
		CLevelAbility_Fire *ability=(CLevelAbility_Fire *)ability0;
		ability->_nBullets++;
	}

	virtual void MakeSeed(CLevelAbility *ability0,LevelAwardSeed &seed)
	{
	}

	virtual const char *GetDesc(LevelAwardSeed &seed,CLevelRecords *records)
	{
		static std::string s;
		FormatString(s,"增加一个火球");
		return s.c_str();
	}


};

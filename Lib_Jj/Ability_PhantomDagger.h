#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "valueset/valueset.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

#include "Tech_Fury.h"


class CUpgradePhantomDagger_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradePhantomDagger_Init,LevelAbilityType_PhantomDagger);

	CUpgradePhantomDagger_Init()
	{
		GConstructor();
	}
	~CUpgradePhantomDagger_Init()
	{
		GDestructor();
	}

	BEGIN_GOBJ(CUpgradePhantomDagger_Init,1);
		GELEM_OBJ(AbilityActionSettings,settings);
			GELEM_EDITOBJ("Action参数","Action参数");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	AbilityActionSettings settings;

};


class CUpgradePhantomDagger_LevelUp:public CLevelAbilityUpgrade_LevelUp
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradePhantomDagger_LevelUp,LevelAbilityType_PhantomDagger);

	BEGIN_GOBJ_PURE(CUpgradePhantomDagger_LevelUp,1);
	END_GOBJ();

};


struct LevelRecordSkill;
class CLevelAbility_PhantomDagger:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_PhantomDagger,CUpgradePhantomDagger_Init,LevelAbilityType_PhantomDagger);

	BEGIN_GOBJ_PURE_UID(CLevelAbility_PhantomDagger,1);

		GELEM_ABILITY_BASE();

		GELEM_VAR_INIT(float,_energy,0.0f);GELEM_UID(3)
	END_GOBJ();

	virtual BOOL TestStartSkill(LevelSkillType &tpSkill) override;
	virtual void NotifyStartSkill(LevelSkillType &idSkill)	override;


	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;

	BOOL IsFullEnergy()
	{
		return _energy>=1.0f;
	}

public://Take it as protected

	virtual void _OnEvent(LevelEvent &e) override;

	virtual void _InitTechs();

	float _energy;

};


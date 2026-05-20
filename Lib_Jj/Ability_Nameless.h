#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

#include "Tech_Fury.h"


struct LevelRecordSkill;
class CUpgradeNameless_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeNameless_Init,LevelAbilityType_Nameless);

	BEGIN_GOBJ_PURE(CUpgradeNameless_Init,1);
		GELEM_OBJ(AbilityActionSettings,settings);
			GELEM_EDITOBJ("Action参数","Action参数");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	AbilityActionSettings settings;

};


class CUpgradeNameless_LevelUp:public CLevelAbilityUpgrade_LevelUp
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeNameless_LevelUp,LevelAbilityType_Nameless);
	BEGIN_GOBJ_PURE(CUpgradeNameless_LevelUp,1);
	END_GOBJ();

};



class CLevelAbility_Nameless:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_Nameless,CUpgradeNameless_Init,LevelAbilityType_Nameless);

	BEGIN_GOBJ_PURE_UID2(CLevelAbility_Nameless,471,1);

		GELEM_ABILITY_BASE();

	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnUpdate(LevelTick dt) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

public://Take it as protected

	virtual void _InitTechs();


};



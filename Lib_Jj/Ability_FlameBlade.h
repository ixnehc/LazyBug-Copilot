#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

#include "Tech_Fury.h"



class CUpgradeFlameBlade_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeFlameBlade_Init,LevelAbilityType_FlameBlade);

	BEGIN_GOBJ_PURE(CUpgradeFlameBlade_Init,1);
		GELEM_OBJ(AbilityActionSettings,settings);
			GELEM_EDITOBJ("Action参数","Action参数");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	AbilityActionSettings settings;
};

class CUpgradeFlameBlade_LevelUp:public CLevelAbilityUpgrade_LevelUp
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeFlameBlade_LevelUp,LevelAbilityType_FlameBlade);
	BEGIN_GOBJ_PURE(CUpgradeFlameBlade_LevelUp,1);
	END_GOBJ();
};



struct LevelRecordSkill;
class CLevelAbility_FlameBlade:public CLevelAbility
{
public:
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_FlameBlade,CUpgradeFlameBlade_Init,LevelAbilityType_FlameBlade);

	BEGIN_GOBJ_PURE_UID(CLevelAbility_FlameBlade,1);
		GELEM_ABILITY_BASE();

		GELEM_OBJ(TechParam_Fury,_tcpFury);

	END_GOBJ();

	virtual BOOL CanFury();


protected:

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;
	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;


public://Take it as protected

	virtual void _InitTechs();

	TechParam_Fury _tcpFury;

};


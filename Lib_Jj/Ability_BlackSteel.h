#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelRecordSkill;
class CLevelAbility_BlackSteel:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_BlackSteel,LevelAbilityType_BlackSteel);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_BlackSteel,1);

		GELEM_ABILITY_BASE();


	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override
	{
	}
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override
	{
	}

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;
	virtual void _OnBuildArtifactState(LevelItemState &state)	 override;


public://Take it as protected
	virtual void _InitTechs()	 override{	}


};


class CUpgradeBlackSteel_Init:public CLevelAbilityInitial_Armor
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeBlackSteel_Init,LevelAbilityType_BlackSteel);

	BEGIN_GOBJ_PURE(CUpgradeBlackSteel_Init,1);

		GELEM_ARMOR_UPGRADE_DEFEND();

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:
};



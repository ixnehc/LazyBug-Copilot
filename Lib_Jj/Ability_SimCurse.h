#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelRecordSkill;
class CLevelAbility_SimCurse:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_SimCurse,LevelAbilityType_SimCurse);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_SimCurse,1);

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


class CUpgradeSimCurse_Init:public CLevelAbilityInitial_Armor
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeSimCurse_Init,LevelAbilityType_SimCurse);

	BEGIN_GOBJ_PURE(CUpgradeSimCurse_Init,1);

		GELEM_ARMOR_UPGRADE_DEFEND();

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:

};



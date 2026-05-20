#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelRecordSkill;
class CLevelAbility_TalBless:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_TalBless,LevelAbilityType_TalBless);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_TalBless,1);

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

	virtual void _OnEndDay() override;

};


class CUpgradeTalBless_Init:public CLevelAbilityInitial_Armor
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeTalBless_Init,LevelAbilityType_TalBless);

	BEGIN_GOBJ_PURE(CUpgradeTalBless_Init,1);

		GELEM_ARMOR_UPGRADE_DEFEND();

		GELEM_VAR_INIT(int,_deltaMaxHPPerDay,100);
			GELEM_EDITVAR("每日生命值增加",GVT_S,GSem_Interger,"每日生命值增加");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:
	int _deltaMaxHPPerDay;

	friend class CLevelAbility_TalBless;
};



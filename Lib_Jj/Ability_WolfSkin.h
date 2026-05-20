#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelRecordSkill;
class CLevelAbility_WolfSkin:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_WolfSkin,LevelAbilityType_WolfSkin);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_WolfSkin,1);

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


class CUpgradeWolfSkin_Init:public CLevelAbilityInitial_Armor
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeWolfSkin_Init,LevelAbilityType_WolfSkin);

	BEGIN_GOBJ_PURE(CUpgradeWolfSkin_Init,1);

		GELEM_ARMOR_UPGRADE_DEFEND();

		GELEM_VAR_INIT(int,_deltaFullSP,100);
			GELEM_EDITVAR("精力增加",GVT_S,GSem_Interger,"精力增加");
		GELEM_VAR_INIT(int,_deltaMaxHP,100);
			GELEM_EDITVAR("生命值增加",GVT_S,GSem_Interger,"生命值增加");
		GELEM_VAR_INIT(int,_ims,10);
			GELEM_EDITVAR("速度增加百分比",GVT_S,GSem_Interger,"速度增加百分比");

	END_GOBJ();

protected:
	int _deltaFullSP;
	int _deltaMaxHP;
	int _ims;

	DefendModsEx baseDefend;
	DefendModsEx upgradeDefend;

	friend class CLevelAbility_WolfSkin;

};



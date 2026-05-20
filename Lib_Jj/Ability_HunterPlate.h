#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelRecordSkill;
class CLevelAbility_HunterPlate:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_HunterPlate,LevelAbilityType_HunterPlate);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_HunterPlate,1);

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
	virtual void _OnEvent(LevelEvent &e) override;

	virtual void _OnBuildArtifactState(LevelItemState &state)	 override;

public://Take it as protected

	virtual void _InitTechs()	 override{	}


};


class CUpgradeHunterPlate_Init:public CLevelAbilityInitial_Armor
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeHunterPlate_Init,LevelAbilityType_HunterPlate);

	BEGIN_GOBJ_PURE(CUpgradeHunterPlate_Init,1);

		GELEM_ARMOR_UPGRADE_DEFEND();

		GELEM_VAR_INIT(int,_deltaPhysDef,100);
			GELEM_EDITVAR("防御增加",GVT_S,GSem_Interger,"防御增加");
		GELEM_VAR_INIT(int,_ims,10);
			GELEM_EDITVAR("速度增加百分比",GVT_S,GSem_Interger,"速度增加百分比");
		GELEM_VAR_INIT(int,_iasBow,10);
			GELEM_EDITVAR("弓攻击速度增加百分比",GVT_S,GSem_Interger,"弓攻击速度增加百分比");
		GELEM_VAR_INIT(int,_deltaPhysDmgRateBow,10);
			GELEM_EDITVAR("弓物理攻击增加百分比",GVT_S,GSem_Interger,"弓物理攻击增加百分比");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:
	int _deltaPhysDef;
	int _ims;
	int _iasBow;
	int _deltaPhysDmgRateBow;

	friend class CLevelAbility_HunterPlate;
};



#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelRecordSkill;
class CLevelAbility_Frost:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_Frost,LevelAbilityType_Frost);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_Frost,1);

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


class CUpgradeFrost_Init:public CLevelAbilityInitial_Armor
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeFrost_Init,LevelAbilityType_Frost);

	BEGIN_GOBJ_PURE(CUpgradeFrost_Init,1);

		GELEM_ARMOR_UPGRADE_DEFEND();

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,_dealFrostExplode,Deal_CreateEo, "寒冰爆炸的Deal", "寒冰爆炸的Deal" );
			GELEMS_LEVELDEAL_CANDIDATES();

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:

	CLevelDeal *_dealFrostExplode;

};



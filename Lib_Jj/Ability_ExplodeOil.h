#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelRecordSkill;
class CLevelAbility_ExplodeOil:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_ExplodeOil,LevelAbilityType_ExplodeOil);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_ExplodeOil,1);

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


class CUpgradeExplodeOil_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeExplodeOil_Init,LevelAbilityType_ExplodeOil);

	BEGIN_GOBJ_PURE(CUpgradeExplodeOil_Init,1);

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,_dealWhenDamaged,Deal_CreateEo, "被Damage时的Deal", "被Damage时的Deal" );
			GELEMS_LEVELDEAL_CANDIDATES();

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:
	CLevelDeal *_dealWhenDamaged;

	friend class CLevelAbility_ExplodeOil;

};



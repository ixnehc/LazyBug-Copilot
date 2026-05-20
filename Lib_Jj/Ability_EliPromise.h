#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelRecordSkill;
class CLevelAbility_EliPromise:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_EliPromise,LevelAbilityType_EliPromise);

	CLevelAbility_EliPromise()
	{
		GConstructor();
		_dealCureRT=NULL;
		_dealShockwaveRT=NULL;
	}

	~CLevelAbility_EliPromise()
	{
		GDestructor();
		Safe_Class_Delete(_dealCureRT);
		Safe_Class_Delete(_dealShockwaveRT);
	}


	BEGIN_GOBJ_UID(CLevelAbility_EliPromise,1);

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

	CLevelDeal *_dealCureRT;
	CLevelDeal *_dealShockwaveRT;



};


class CUpgradeEliPromise_Init:public CLevelAbilityInitial_Armor
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeEliPromise_Init,LevelAbilityType_EliPromise);

	BEGIN_GOBJ_PURE(CUpgradeEliPromise_Init,1);

		GELEM_ARMOR_UPGRADE_DEFEND();

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,_dealCure,Deal_CureHP, "恢复HP的Deal", "恢复HP的Deal" );
			GELEM_DYNOBJPTR_CLASS_DEAL( "06.HP回复", Deal_CureHP);

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,_dealShockwave,Deal_CreateEo, "震荡波的Deal", "震荡波的Deal" );
			GELEMS_LEVELDEAL_CANDIDATES();

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:

	CLevelDeal *_dealCure;
	CLevelDeal *_dealShockwave;

	friend class CLevelAbility_EliPromise;

};



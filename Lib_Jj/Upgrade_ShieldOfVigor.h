#pragma once

#include "LevelArtifactUpgrade.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

struct LevelRecordSkill;

class CUpgradeShieldOfVigor_Init:public CLevelArtifactUpgrade_Initial
{
public:
	DEFINE_ARTIFACT_UPGRADE_CLASS(CUpgradeShieldOfVigor_Init,LevelArtifact_ShieldOfVigor);

	BEGIN_GOBJ_PURE(CUpgradeShieldOfVigor_Init,1);
		GELEM_VAR_INIT(int,nDeltaMaxHP,0);
			GELEM_EDITVAR("MaxHP增量",GVT_S,GSem_Interger,"MaxHP增量");
	END_GOBJ();

	virtual BOOL Init(LevelItemState *state);

	int nDeltaMaxHP;
};



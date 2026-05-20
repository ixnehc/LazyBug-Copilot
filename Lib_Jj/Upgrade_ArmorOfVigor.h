#pragma once

#include "LevelArtifactUpgrade.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

struct LevelRecordSkill;

class CUpgradeArmorOfVigor_Init:public CLevelArtifactUpgrade_Initial
{
public:
	DEFINE_ARTIFACT_UPGRADE_CLASS(CUpgradeArmorOfVigor_Init,LevelArtifact_ArmorOfVigor);

	BEGIN_GOBJ_PURE(CUpgradeArmorOfVigor_Init,1);
		GELEM_VAR_INIT(int,nDeltaMaxHP,0);
			GELEM_EDITVAR("MaxHP增量",GVT_S,GSem_Interger,"MaxHP增量");
	END_GOBJ();

	virtual BOOL Init(LevelItemState *state);

	int nDeltaMaxHP;
};



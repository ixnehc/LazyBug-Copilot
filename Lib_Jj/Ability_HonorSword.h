#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

#include "Tech_Fury.h"

struct LevelRecordSkill;
class CLevelAbility_HonorSword:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_HonorSword,LevelAbilityType_HonorSword);

	BEGIN_GOBJ_PURE_UID(CLevelAbility_HonorSword,1);

		GELEM_ABILITY_BASE();

		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);GELEM_UID(1)
		GELEM_VAR_INIT(RecordID,_idDefSkill,RecordID_Invalid);GELEM_UID(2)

	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnUpdate(LevelTick dt) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

public://Take it as protected

	virtual void _InitTechs();

	RecordID _idSkill;
	RecordID _idDefSkill;

};


class CUpgradeHonorSword_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeHonorSword_Init,LevelAbilityType_HonorSword);

	BEGIN_GOBJ_PURE(CUpgradeHonorSword_Init,1);
		GELEM_VAR_INIT(RecordID,idDefaultSkill,RecordID_Invalid);
			GELEM_EDITVAR("缺省(左键)技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	RecordID idSkill;
	RecordID idDefaultSkill;

};


class CUpgradeHonorSword_LevelUp:public CLevelAbilityUpgrade_LevelUp
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeHonorSword_LevelUp,LevelAbilityType_HonorSword);
	BEGIN_GOBJ_PURE(CUpgradeHonorSword_LevelUp,1);
	END_GOBJ();

};

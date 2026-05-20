#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelRecordSkill;
class CLevelAbility_SkullSword:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_SkullSword,LevelAbilityType_SkullSword);

	BEGIN_GOBJ_PURE_UID(CLevelAbility_SkullSword,1);

		GELEM_ABILITY_BASE();

		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);GELEM_UID(1)
		GELEM_VAR_INIT(RecordID,_idDefSkill,RecordID_Invalid);GELEM_UID(2)
		GELEM_VAR_INIT(RecordID,_idSummon,RecordID_Invalid);GELEM_UID(3)

	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;

public://Take it as protected

	virtual void _InitTechs() override;

	virtual void _OnEvent(LevelEvent &e) override;

	RecordID _idSkill;
	RecordID _idDefSkill;

	RecordID _idSummon;

};


class CUpgradeSkullSword_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeSkullSword_Init,LevelAbilityType_SkullSword);

	BEGIN_GOBJ_PURE(CUpgradeSkullSword_Init,1);
		GELEM_VAR_INIT(RecordID,idDefaultSkill,RecordID_Invalid);
			GELEM_EDITVAR("缺省(左键)技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_VAR_INIT(RecordID,idSummon,RecordID_Invalid);
			GELEM_EDITVAR("召唤单位ID",GVT_S,GSem(GSem_RecordID,"units"),"召唤单位ID");
		GELEM_VAR_INIT(RecordID,idBirth,RecordID_Invalid);
			GELEM_EDITVAR("召唤单位出生Buff",GVT_S,GSem(GSem_RecordID,"buffs"),"召唤单位出生Buff");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	RecordID idSkill;
	RecordID idDefaultSkill;
	RecordID idSummon;
	RecordID idBirth;

};


class CUpgradeSkullSword_LevelUp:public CLevelAbilityUpgrade_LevelUp
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeSkullSword_LevelUp,LevelAbilityType_SkullSword);

	BEGIN_GOBJ_PURE(CUpgradeSkullSword_LevelUp,1);
	END_GOBJ();

};

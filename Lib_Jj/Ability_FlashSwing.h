#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelRecordSkill;
class CLevelAbility_FlashSwing:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_FlashSwing,LevelAbilityType_FlashSwing);

	BEGIN_GOBJ_PURE_UID(CLevelAbility_FlashSwing,1);

		GELEM_ABILITY_BASE();

		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);GELEM_UID(1)
		GELEM_VAR_INIT(RecordID,_idDefSkill,RecordID_Invalid);GELEM_UID(2)

	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;

public://Take it as protected

	virtual void _InitTechs()	 override{	}


	RecordID _idSkill;
	RecordID _idDefSkill;

};


class CUpgradeFlashSwing_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeFlashSwing_Init,LevelAbilityType_FlashSwing);

	BEGIN_GOBJ_PURE(CUpgradeFlashSwing_Init,1);
		GELEM_VAR_INIT(RecordID,idDefaultSkill,RecordID_Invalid);
			GELEM_EDITVAR("缺省(左键)技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	RecordID idSkill;
	RecordID idDefaultSkill;

};


class CUpgradeFlashSwing_LevelUp:public CLevelAbilityUpgrade_LevelUp
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeFlashSwing_LevelUp,LevelAbilityType_FlashSwing);
	BEGIN_GOBJ_PURE(CUpgradeFlashSwing_LevelUp,1);
	END_GOBJ();

};

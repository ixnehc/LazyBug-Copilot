#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

class CUpgradeUnarmed_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeUnarmed_Init,LevelAbilityType_Unarmed);

	BEGIN_GOBJ_PURE(CUpgradeUnarmed_Init,1);
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	RecordID idSkill;

};




struct LevelRecordSkill;
class CLevelAbility_Unarmed:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_Unarmed,CUpgradeUnarmed_Init,LevelAbilityType_Unarmed);

	BEGIN_GOBJ_PURE_UID(CLevelAbility_Unarmed,1);

		GELEM_ABILITY_BASE();

	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;
	virtual void _OnUpdate(LevelTick dt) override;

	virtual int GetSkillStack()	{		return MAX_LEVEL_SKILL_STACK;	}


public://Take it as protected

	virtual void _InitTechs()	 override{	}

};



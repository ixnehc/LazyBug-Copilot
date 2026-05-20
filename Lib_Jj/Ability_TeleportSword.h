#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "valueset/valueset.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

#include "Tech_Fury.h"

struct LevelRecordSkill;
class CLevelAbility_TeleportSword:public CLevelAbility
{
	DEFINE_ABILITY_CLASS(CLevelAbility_TeleportSword,LevelAbilityType_TeleportSword);

	BEGIN_GOBJ_PURE_UID(CLevelAbility_TeleportSword,1);

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

	virtual void _OnEvent(LevelEvent &e) override;

	virtual void _InitTechs();

	RecordID _idSkill;
	RecordID _idDefSkill;
};


class CUpgradeTeleportSword_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeTeleportSword_Init,LevelAbilityType_TeleportSword);

	CUpgradeTeleportSword_Init()
	{
		GConstructor();
		vsDmgMultiply.ResetFloat(1.0f);
	}
	~CUpgradeTeleportSword_Init()
	{
		GDestructor();
	}

	BEGIN_GOBJ(CUpgradeTeleportSword_Init,1);
		GELEM_VAR_INIT(RecordID,idDefaultSkill,RecordID_Invalid);
			GELEM_EDITVAR("缺省(左键)技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_OBJVAR( ValueSet, vsDmgMultiply);
			GELEM_EDITOBJ_EX("距离伤害曲线","距离伤害曲线",GSem( GSem_Unknown, "0,0,-1,20" ));
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	RecordID idSkill;
	RecordID idDefaultSkill;

	ValueSet vsDmgMultiply;

};


class CUpgradeTeleportSword_LevelUp:public CLevelAbilityUpgrade_LevelUp
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeTeleportSword_LevelUp,LevelAbilityType_TeleportSword);

	BEGIN_GOBJ_PURE(CUpgradeTeleportSword_LevelUp,1);
	END_GOBJ();

};

#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"


class CUpgradeBomb_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeBomb_Init,LevelAbilityType_Bomb);

	BEGIN_GOBJ_PURE(CUpgradeBomb_Init,1);

		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:

	RecordID idSkill;


	friend class CLevelAbility_Bomb;

};



struct CBBombThrust;
struct LevelRecordSkill;
class CLevelAbility_Bomb:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_Bomb,CUpgradeBomb_Init,LevelAbilityType_Bomb);

	CLevelAbility_Bomb()
	{
		GConstructor();
	}
	~CLevelAbility_Bomb()
	{
		GDestructor();
	}

	BEGIN_GOBJ_UID(CLevelAbility_Bomb,1);

		GELEM_ABILITY_BASE();

	END_GOBJ();


public:

	virtual void _OnEvent(LevelEvent &e) override;


	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;

	virtual void _UpdateStackCount() override;
	virtual void _OnBuildArtifactState(LevelItemState &state)	 override;

public://Take it as protected

};



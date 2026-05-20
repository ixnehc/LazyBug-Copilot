#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

#include <unordered_set>


class CUpgradeWhetstone_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeWhetstone_Init,LevelAbilityType_Whetstone);

	BEGIN_GOBJ_PURE(CUpgradeWhetstone_Init,1);

		GELEM_VAR_INIT(float,addBleed,10.0f);
			GELEM_EDITVAR("增加流血攻击值",GVT_F,GSem(GSem_Float,"0.1,1000.0,0.1"),"增加流血攻击值");
		GELEM_VAR_INIT(float,ratePierce,0.5f);
			GELEM_EDITVAR("增加穿刺伤害比率",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"增加伤害比率");
		GELEM_VARVECTOR_INIT(RecordID,skills,RecordID_Invalid);
			GELEM_EDITVAR("影响的技能",GVT_S,GSem(GSem_RecordID,"skills"),"哪些技能受到影响");

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

	BOOL ExistSkillID(RecordID idSkill);

protected:

	float ratePierce;
	float addBleed;

	std::vector<RecordID> skills;

	std::unordered_set<RecordID> cacheSkills;


	friend class CLevelAbility_Whetstone;

};



struct CBWhetstoneThrust;
struct LevelRecordSkill;
class CLevelAbility_Whetstone:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_Whetstone,CUpgradeWhetstone_Init,LevelAbilityType_Whetstone);

	CLevelAbility_Whetstone()
	{
		GConstructor();
	}
	~CLevelAbility_Whetstone();


	BEGIN_GOBJ_UID(CLevelAbility_Whetstone,1);

		GELEM_ABILITY_BASE();
	END_GOBJ();

public:

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnBuildArtifactState(LevelItemState &state)	 override;

	virtual void _OnEvent(LevelEvent &e) override;

public://Take it as protected

	virtual void _InitTechs()	 override{	}

public://以下只在Client上工作


};



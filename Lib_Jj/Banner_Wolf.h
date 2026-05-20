#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeBannerWolf_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeBannerWolf_Init,LevelAbilityType_Banner_Wolf);

	BEGIN_GOBJ_PURE(CUpgradeBannerWolf_Init,1);


	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:


	friend class CLevelBanner_Wolf;

};



struct CBShieldMaskThrust;
struct LevelRecordSkill;
class CLevelBanner_Wolf:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelBanner_Wolf,CUpgradeBannerWolf_Init,LevelAbilityType_Banner_Wolf);

	CLevelBanner_Wolf()
	{
		GConstructor();
	}
	~CLevelBanner_Wolf();


	BEGIN_GOBJ_UID2(CLevelBanner_Wolf,444,1);

		GELEM_ABILITY_BASE();

	END_GOBJ();


public:

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnStartDay() override;
	virtual void _OnUpdate(LevelTick dt) override;

	virtual void _OnBuildArtifactState(LevelItemState &state)	 override;

	virtual void _OnEvent(LevelEvent &e) override;

public://Take it as protected

	virtual void _InitTechs()	 override{	}


public://以下只在Client上工作


};



#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeBannerFire_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeBannerFire_Init,LevelAbilityType_Banner_Fire);

	BEGIN_GOBJ_PURE(CUpgradeBannerFire_Init,1);


	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:


	friend class CLevelBanner_Fire;

};



struct CBShieldMaskThrust;
struct LevelRecordSkill;
class CLevelBanner_Fire:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelBanner_Fire,CUpgradeBannerFire_Init,LevelAbilityType_Banner_Fire);

	CLevelBanner_Fire()
	{
		GConstructor();
	}
	~CLevelBanner_Fire();


	BEGIN_GOBJ_UID2(CLevelBanner_Fire,443,1);

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



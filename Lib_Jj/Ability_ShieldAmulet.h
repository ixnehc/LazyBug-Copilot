#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

#define SHIELDAMULET_SHIELDHEIGHT (1.8f)
#define SHIELDAMULET_SHIELDRANGE (0.5f)
#define SHIELDAMULET_SHIELDFOV (60.0f)


class CUpgradeShieldAmulet_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeShieldAmulet_Init,LevelAbilityType_ShieldAmulet);

	BEGIN_GOBJ_PURE(CUpgradeShieldAmulet_Init,1);

		GELEM_VAR_INIT(float,speedRot,60.0f);
			GELEM_EDITVAR("旋转速度",GVT_F,GSem(GSem_Float,"0.1,10000.0,0.05"),"旋转速度(以角度为单位)");

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:

	float speedRot;
	friend class CLevelAbility_ShieldAmulet;

};



struct CBShieldAmuletThrust;
struct LevelRecordSkill;
class CLevelAbility_ShieldAmulet:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_ShieldAmulet,CUpgradeShieldAmulet_Init,LevelAbilityType_ShieldAmulet);

	CLevelAbility_ShieldAmulet()
	{
		GConstructor();
		_tStart=ANIMTICK_INFINITE;
		_speedRot=0.0f;
	}
	~CLevelAbility_ShieldAmulet();


	BEGIN_GOBJ_UID(CLevelAbility_ShieldAmulet,1);

		GELEM_ABILITY_BASE();

	END_GOBJ();

	AnimTick GetStartTime()	{		return _tStart;	}
	float GetRotSpeed()	{		return _speedRot;	}

	BOOL HitTest(i_math::line3df &line,float radius,i_math::vector3df &posHit);


public:

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnStartDay() override;
	virtual void _OnUpdate(LevelTick dt) override;

	virtual void _OnBuildArtifactState(LevelItemState &state)	 override;

	virtual void _OnEvent(LevelEvent &e) override;


	AnimTick _tStart;
	float _speedRot;

public://Take it as protected

	virtual void _InitTechs()	 override{	}


public://以下只在Client上工作

	void DepositDmgAbort(LevelDmgAbort &abort);
	BOOL FetchDmgAbort(LevelDmgAbort &abort);

	std::deque<LevelDmgAbort> _abortsDmg;

};



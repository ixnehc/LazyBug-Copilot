#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeShieldMask_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeShieldMask_Init,LevelAbilityType_ShieldMask);

	BEGIN_GOBJ_PURE(CUpgradeShieldMask_Init,1);

		GELEM_VAR_INIT(int,nMaxCharge,5);
			GELEM_EDITVAR("最大充能数",GVT_S,GSem_Interger,"最大充能数");


	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:

	int nMaxCharge;
	

	friend class CLevelAbility_ShieldMask;

};



struct CBShieldMaskThrust;
struct LevelRecordSkill;
class CLevelAbility_ShieldMask:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_ShieldMask,CUpgradeShieldMask_Init,LevelAbilityType_ShieldMask);

	CLevelAbility_ShieldMask()
	{
		GConstructor();
		_nMaxCharge=0;
	}
	~CLevelAbility_ShieldMask();


	BEGIN_GOBJ_UID(CLevelAbility_ShieldMask,1);

		GELEM_ABILITY_BASE();
		GELEM_VAR_INIT(int,_nCharge,0);GELEM_UID(1)

	END_GOBJ();


	virtual int GetChargeCount()	{		return _nCharge;	}
	virtual int GetMaxChargeCount()	{		return _nMaxCharge;	}

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

	int _nMaxCharge;
	int _nCharge;

public://以下只在Client上工作

	void DepositDmgAbort(LevelDmgAbort &abort);
	BOOL FetchDmgAbort(LevelDmgAbort &abort);

	std::deque<LevelDmgAbort> _abortsDmg;

};



#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeHPAmulet_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeHPAmulet_Init,LevelAbilityType_HPAmulet);

	BEGIN_GOBJ_PURE(CUpgradeHPAmulet_Init,1);

		GELEM_VAR_INIT(DWORD,_recoverPerDay,20);
			GELEM_EDITVAR("每日增加最大MaxHP",GVT_U,GSem_Interger,"每日增加最大MaxHP");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:
	int _recoverPerDay;

	friend class CLevelAbility_HPAmulet;

};


struct LevelRecordSkill;
class CLevelAbility_HPAmulet:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_HPAmulet,CUpgradeHPAmulet_Init,LevelAbilityType_HPAmulet);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_HPAmulet,1);

		GELEM_ABILITY_BASE();

	END_GOBJ();

	virtual void _OnBuildRT()	{	}
	virtual void _OnClearRT()	{	}
	virtual void _OnUpdate(LevelTick dt){}
	virtual void _SaveSync(CDataPacket &dp)	{	}
	virtual void _LoadSync(CDataPacket &dp,CRecords *records)	{	}


	virtual void _OnDaily() override;

public://Take it as protected

};


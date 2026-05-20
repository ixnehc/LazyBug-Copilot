#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeMoneyBag_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeMoneyBag_Init,LevelAbilityType_MoneyBag);

	BEGIN_GOBJ_PURE(CUpgradeMoneyBag_Init,1);

		GELEM_VAR_INIT(DWORD,_recoverPerDay,20);
			GELEM_EDITVAR("每日生成Gold数量",GVT_U,GSem_Interger,"每日生成Gold数量");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:
	int _recoverPerDay;

	friend class CLevelAbility_MoneyBag;

};


struct LevelRecordSkill;
class CLevelAbility_MoneyBag:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_MoneyBag,CUpgradeMoneyBag_Init,LevelAbilityType_MoneyBag);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_MoneyBag,1);

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


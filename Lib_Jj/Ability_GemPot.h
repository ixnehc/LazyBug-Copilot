#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeGemPot_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeGemPot_Init,LevelAbilityType_GemPot);

	BEGIN_GOBJ_PURE(CUpgradeGemPot_Init,1);

		GELEM_VAR_INIT(DWORD,_recoverPerDay,20);
			GELEM_EDITVAR("每日生成Gem数量",GVT_U,GSem_Interger,"每日生成Gem数量");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:
	int _recoverPerDay;

	friend class CLevelAbility_GemPot;

};


struct LevelRecordSkill;
class CLevelAbility_GemPot:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_GemPot,CUpgradeGemPot_Init,LevelAbilityType_GemPot);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_GemPot,1);

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


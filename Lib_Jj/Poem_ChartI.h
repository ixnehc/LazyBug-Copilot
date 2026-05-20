#pragma once
#include "LevelDefines.h"

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "LevelPoem.h"

struct LevelRecordSkill;
class CLevelPoem_ChartI:public CLevelPoem
{
	DEFINE_ABILITY_CLASS(CLevelPoem_ChartI,LevelAbilityType_Poem_ChartI);

	CLevelPoem_ChartI()
	{
		GConstructor();
	}

	~CLevelPoem_ChartI()
	{
		GDestructor();
	}

	BEGIN_GOBJ_UID2(CLevelPoem_ChartI,428,1);


	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *recordsSkill) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;
	virtual void _OnUpdate(LevelTick dt) override;


public://Take it as protected
	virtual void _InitTechs()	 override{	}


};


class CUpgradePoemChartI_Init:public CLevelPoemInitial
{
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradePoemChartI_Init,LevelAbilityType_Poem_ChartI);

	BEGIN_GOBJ_PURE(CUpgradePoemChartI_Init,1);
		GELEM_POEM_BASE();

	END_GOBJ();

	virtual BOOL Init(CLevelAbility *ability);
	virtual RecordID GetSkillID()		{			return RecordID_Invalid;		}

};


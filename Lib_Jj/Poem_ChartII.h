#pragma once
#include "LevelDefines.h"

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "LevelPoem.h"

struct LevelRecordSkill;
class CLevelPoem_ChartII:public CLevelPoem
{
	DEFINE_ABILITY_CLASS(CLevelPoem_ChartII,LevelAbilityType_Poem_ChartII);

	CLevelPoem_ChartII()
	{
		GConstructor();
	}

	~CLevelPoem_ChartII()
	{
		GDestructor();
	}

	BEGIN_GOBJ_UID2(CLevelPoem_ChartII,428,1);


	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *recordsSkill) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;
	virtual void _OnUpdate(LevelTick dt) override;


public://Take it as protected
	virtual void _InitTechs()	 override{	}


};


class CUpgradePoemChartII_Init:public CLevelPoemInitial
{
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradePoemChartII_Init,LevelAbilityType_Poem_ChartII);

	BEGIN_GOBJ_PURE(CUpgradePoemChartII_Init,1);
		GELEM_POEM_BASE();

	END_GOBJ();

	virtual BOOL Init(CLevelAbility *ability);
	virtual RecordID GetSkillID()		{			return RecordID_Invalid;		}

};


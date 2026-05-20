/********************************************************************
	created:	2012/10/23 
	author:		cxi
	
	purpose:	AI Actions
*********************************************************************/
#include "stdh.h"

#include "Act_FlyUp.h"
#include "LevelRecords.h"
#include "LevelObjResidable.h"

#include "Random/Random.h"


#include "Level.h"


BIND_ACT_PARAM(Act_FlyUp,ActParam_FlyUp);



void Act_FlyUp::Start(AnimTick t)
{
	ActParam_FlyUp *param=(ActParam_FlyUp*)_param;

	_tStart=t;

	CLevelSkillDriver *driver=_owner->GetSkillDriver();
	if (driver)
	{
		LevelSkillTarget target;
		driver->Start(param->idSkill,target,FALSE,ClientSkillID_Invalid);
	}
	else
		_result=A_Fail;
}

void Act_FlyUp::Update(AnimTick t)
{
	if (_result!=A_Pending)
		return;

	LevelMoveMethod method=_owner->GetMoveMethod();

	if (method!=LevelMoveMethod_Flying)
	{
		if (t-_tStart>ANIMTICK_FROM_SECOND(2.0f))
			_result=A_Fail;
		return;
	}

	if (_tStartFly==ANIMTICK_INFINITE)
		_tStartFly=t;

	ActParam_FlyUp *param=(ActParam_FlyUp*)_param;
	if (t-_tStartFly>param->dur)
		_result=A_Ok;
}

void Act_FlyUp::Finish()
{
}

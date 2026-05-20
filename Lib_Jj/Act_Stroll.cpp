/********************************************************************
	created:	2012/10/23 
	author:		cxi
	
	purpose:	AI Actions
*********************************************************************/
#include "stdh.h"

#include "Act_Stroll.h"
#include "LevelRecords.h"

#include "Random/Random.h"


#include "Level.h"

BIND_ACT_PARAM(Act_Stroll,ActParam_Stroll);


void Act_Stroll::Start(AnimTick t)
{
}

void Act_Stroll::Update(AnimTick t)
{
	ActParam_Stroll *param=(ActParam_Stroll *)_param;
	if (_tNextMove!=ANIMTICK_INFINITE)
	{
		if (_tNextMove<=t)
		{//选择一个地方移动

			CLevelSkillDriver *driver=_owner->GetSkillDriver();

			if (driver)
			{
				CLevel *lvl=_owner->GetLevel();
				LevelPos pos=_owner->GetFramePos();

				float rangeX=CSysRandom::RandRange(-param->range,param->range);
				float rangeY=CSysRandom::RandRange(-param->range,param->range);

				pos.x+=rangeX;
				pos.y+=rangeY;

				LevelSkillTarget target;
				target.SetPos(pos);
				driver->StartMove(target);
			}

			AnimTick dur=CSysRandom::RandVaryUInt(param->gap,param->gapVary);
			_tNextMove=t+dur;
		}
	}
	else
	{
		AnimTick dur=CSysRandom::RandVaryUInt(param->gap,param->gapVary);
		_tNextMove=t+dur;
	}


}


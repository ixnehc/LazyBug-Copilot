/********************************************************************
	created:	2012/10/23 
	author:		cxi
	
	purpose:	AI Actions
*********************************************************************/
#include "stdh.h"

#include "Act_Attack.h"
#include "LevelRecords.h"

#include "Random/Random.h"


#include "Level.h"


BIND_ACT_PARAM(Act_Attack,ActParam_Attack);


void Act_Attack::Start(AnimTick t,float radius)
{
	ActParam_Attack *param=(ActParam_Attack*)_param;

	//记录methods
	if (TRUE)
	{
		LevelRecordSkill *rec=_owner->GetLevel()->GetRecords()->GetSkill(param->idSkill);
		if (rec)
			_methods=rec->methodsTarget;
	}

	CLevelSkillDriver *driver=_owner->GetSkillDriver();
	if (!driver)
		return;

	if (param->radius>radius)
		radius=param->radius;
	extern CLevelObj *AIUtil_DetectClosestPlayerUnit(CLevelObj *lo,float range,CLevelObj *toIgnore,LevelMoveMethodMask methods);
	CLevelObj *lo=AIUtil_DetectClosestPlayerUnit(_owner,radius,NULL,_methods);

	_tLastCheckEscape=t;
	if (lo)
	{
		if (lo->GetFramePos().getDistanceSQFrom(_owner->GetFramePos())<param->distKeep*param->distKeep)
		{
			//目标离自己太近,我们先要离远一点
			extern BOOL AIUtil_Flee(CLevelObj *lo,CLevelObj *loEscapeFrom,float distKeep);
			AIUtil_Flee(_owner,lo,param->distKeep*1.5f);
			SAFE_REPLACE(_target,lo);
			_bEscaping=TRUE;
		}
		else
		{
			//立即开始攻击
			LevelRecordSkill *recSkill=_owner->GetLevel()->GetRecords()->GetSkill(param->idSkill);
			if (recSkill)
			{
				LevelSkillTarget target;
				target.SetObjID(lo->GetID());
				driver->Start(recSkill,target,FALSE,ClientSkillID_Invalid);
			}
			SAFE_REPLACE(_target,lo);
		}
	}
}

void Act_Attack::Update(AnimTick t)
{
	ActParam_Attack *param=(ActParam_Attack*)_param;
	CLevelSkillDriver *driver=_owner->GetSkillDriver();
	if (!driver)
		return;

	if ((t>_tLastCheckEscape+param->durCheckEscape)&&(param->distKeep>0.0f))
	{
		if (driver->IsWorking())
		{
			if (_target)
			{
				if (_target->GetFramePos().getDistanceSQFrom(_owner->GetFramePos())<param->distKeep*param->distKeep)
				{
					extern BOOL AIUtil_Flee(CLevelObj *lo,CLevelObj *loEscapeFrom,float distKeep);
					AIUtil_Flee(_owner,_target,param->distKeep*1.5f);
					_bEscaping=TRUE;
					return;
				}
			}
		}
	}

	if (_bEscaping)
	{
		if (_target)
		{
			if (_target->GetFramePos().getDistanceSQFrom(_owner->GetFramePos())>=param->distKeep*param->distKeep)
			{//在范围之外了,
				LevelRecordSkill *recSkill=_owner->GetLevel()->GetRecords()->GetSkill(param->idSkill);
				if (recSkill)
				{
					LevelSkillTarget target;
					target.SetObjID(_target->GetID());
					driver->Start(recSkill,target,FALSE,ClientSkillID_Invalid);
				}
				_bEscaping=FALSE;
				return;
			}
		}
	}

	if (driver)
	{
		if (!driver->IsWorking())
		{
			if (_target)
				_tLoseTarget=t;
			extern CLevelObj *AIUtil_DetectClosestPlayerUnit(CLevelObj *lo,float range,CLevelObj *toIgnore,LevelMoveMethodMask methods);
			CLevelObj *lo=AIUtil_DetectClosestPlayerUnit(_owner,param->radius,_target,_methods);
			SAFE_RELEASE(_target);
			if (lo)
			{
				LevelRecordSkill *recSkill=_owner->GetLevel()->GetRecords()->GetSkill(param->idSkill);
				if (recSkill)
				{
					LevelSkillTarget target;
					target.SetObjID(lo->GetID());
					driver->Start(recSkill,target,FALSE,ClientSkillID_Invalid);
				}
				SAFE_REPLACE(_target,lo);
				_tLoseTarget=ANIMTICK_INFINITE;
			}
		}
	}
}

void Act_Attack::Finish()
{
	SAFE_RELEASE(_target);
}

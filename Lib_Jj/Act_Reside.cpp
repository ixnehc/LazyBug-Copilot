/********************************************************************
	created:	2012/10/23 
	author:		cxi
	
	purpose:	AI Actions
*********************************************************************/
#include "stdh.h"

#include "Act_Reside.h"
#include "LevelRecords.h"
#include "LevelObjResidable.h"

#include "Random/Random.h"


#include "Level.h"


BIND_ACT_PARAM(Act_Reside,ActParam_Reside);

void Act_Reside::_ClearTarget()
{
	if (_target)
	{
		CLevelObjResidable *seats=_target->GetResidable();
		if (seats)
			seats->Cancel(_token);
	}
	SAFE_RELEASE(_target);
	_token=LevelObjSeatToken_Invalid;
}


void Act_Reside::Start(AnimTick t,CLevelObj *loTarget,LevelObjSeatToken token)
{
	ActParam_Reside *param=(ActParam_Reside*)_param;

	_tStart=t;

	if (loTarget)
	{
		CLevelSkillDriver *driver=_owner->GetSkillDriver();
		LevelRecordSkill *recSkill=_owner->GetLevel()->GetRecords()->GetSkill(param->idSkill);
		if (recSkill)
		{
			LevelSkillTarget target;
			target.SetObjID(loTarget->GetID());
			driver->Start(recSkill,target,FALSE,ClientSkillID_Invalid);
		}
		SAFE_REPLACE(_target,loTarget);
		_token=token;
	}
	else
		_result=A_Fail;
}

void Act_Reside::Update(AnimTick t)
{
	if (_result!=A_Pending)
		return;
	extern AnimTick AIUtil_GetBuffFlagAge(CLevelObj *lo,DWORD flagBuff);

	ActParam_Reside *param=(ActParam_Reside*)_param;
	CLevelSkillDriver *driver=_owner->GetSkillDriver();
	if (driver)
	{
		if (!driver->IsWorking())
		{
			extern BOOL AIUtil_TestAnyBuff(CLevelObj *lo,DWORD flagBuff);
			if (AIUtil_TestAnyBuff(_owner,BuffFlag_Reside))
			{//进入了
				_ClearTarget();

				_result=A_Ok;

				return;
			}

			if (t>_tStart+param->tSearching)
			{//时间太长仍没进入,失败
				_result=A_Fail;
				_ClearTarget();
				return;
			}

			//继续尝试
			extern CLevelObj *AIUtil_DetectClosestResidable(CLevelObj *lo,float range,CLevelObj *toIgnore,ClassUID uid);
			CLevelObj *lo=AIUtil_DetectClosestResidable(_owner,param->radius,_target,param->uidAgent);
			_ClearTarget();
			LevelObjSeatToken token=LevelObjSeatToken_Invalid;

			if (lo)
			{
				CLevelObjResidable *seats=lo->GetResidable();
				if (seats)
					token=seats->Preserve();

				if (token==LevelObjSeatToken_Invalid)
				{
					_result=A_Fail;
					_ClearTarget();
					return;
				}

				CLevelSkillDriver *driver=_owner->GetSkillDriver();
				LevelRecordSkill *recSkill=_owner->GetLevel()->GetRecords()->GetSkill(param->idSkill);
				if (recSkill)
				{
					LevelSkillTarget target;
					target.SetObjID(lo->GetID());
					driver->Start(recSkill,target,FALSE,ClientSkillID_Invalid);
				}
				SAFE_REPLACE(_target,lo);
				_token=token;
			}
			else
				_result=A_Fail;
		}
	}
}

void Act_Reside::Finish()
{
	_ClearTarget();
}

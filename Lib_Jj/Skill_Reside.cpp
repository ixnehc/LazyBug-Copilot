/********************************************************************
	created:	2012/10/22 
	author:		cxi
	
	purpose:	进入某个Agent的技能
*********************************************************************/
#include "stdh.h"


#include "Skill_Reside.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "Buff_ResideHole.h"

#include "LevelDecider.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_Sweep
BIND_SKILLPARAM(Skill_Reside,SkillParam_Reside);


extern BOOL CheckAttackable(CLevelObj *lo);

BOOL Skill_Reside::PreInitStartCheck(CLevelObj *owner,LevelRecordSkill *rec,LevelSkillTarget &target)
{
	LevelObjID id=target.ObjID();
	if (owner)
	{
		if (owner->GetLevel())
		{
			if (CLevelObj *loTarget=owner->GetLevel()->GetIDs()->LoFromID(id))
			{
				if (CLevelDecider::CheckOccupyResidable(loTarget))
					return TRUE;
			}
		}
	}

	return FALSE;
}


void Skill_Reside::_OnStart()
{
	_SetState(SkillState_Casting);
	_AddStartOp();

	GetLevel()->AddAffect(_owner);


	//启动一个Buff
	SkillParam_Reside *param=_rec->GetParam<SkillParam_Reside>();
	if (param)
	{
		if (param->buffEnter!=RecordID_Invalid)
		{
			BuffArg_Reside paramBuff;
			paramBuff.idTarget=_target.ObjID();

			GetLevel()->GetDecider()->MakeBuff(LevelOSB(this),GetOwner(),param->buffEnter,ANIMTICK_INFINITE,&paramBuff,LevelOpLink());
		}
	}

	_SetState(SkillState_Finished);

}


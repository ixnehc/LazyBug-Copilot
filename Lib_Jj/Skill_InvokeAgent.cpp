
#include "stdh.h"


#include "Skill_InvokeAgent.h"

#include "LevelRecordSkill.h"

#include "Level.h"

#include "LevelDecider.h"

#include "LoAgent.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_InvokeAgent
BIND_SKILLPARAM(Skill_InvokeAgent,SkillParam_InvokeAgent);


void Skill_InvokeAgent::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	if (_target.tp==LevelSkillTarget::Target_DefObj)
	{
		CLevelObj *lo=GetLevel()->GetIDs()->LoFromID(_target.ObjID());
		if (lo)
		{
			if (lo->GetType()==LevelObjType_Agent)
			{
				((CLoAgent*)lo)->Invoke(GetOwner());
			}
		}
	}

	_SetState(SkillState_Casted);
}

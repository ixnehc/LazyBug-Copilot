
#include "stdh.h"

#include "Level.h"

#include "LevelSkills.h"

#include "LevelSkill.h"

#include "Log/LogDump.h"

void CLevelSkills::Destroy()
{
	for (int i=0;i<_skills.size();i++)
	{
		CLevelSkill *skill=_skills[i];
		skill->Finish();
		SAFE_RELEASE(skill);
	}
	_skills.clear();
}

void CLevelSkills::Update()
{
	LevelTick t=_level->GetT_();
	DWORD c=0;
	for (int i=0;i<_skills.size();i++)
	{
		CLevelSkill *skill=_skills[i];
		if (!skill->CheckOwnerAlive())
		{
			skill->Finish();
			SAFE_RELEASE(skill);
			continue;
		}
		skill->Update();
		if (skill->GetState()==SkillState_Finished)
		{
			skill->Finish();
			SAFE_RELEASE(skill);
			continue;
		}
		else
		{
// 			LevelTick age=ANIMTICK_SAFE_MINUS(t,skill->GetBirthTick());
// 			if (age>ANIMTICK_FROM_SECOND(120.0f))
// 			{
// 				LOG_DUMP_1P("CLevelSkills",Log_Error,"有一个技能(%s)持续了太长的时间!",skill->GetClass()->GetName());
// 			}
		}
		_skills[c]=_skills[i];
		c++;
	}

	_skills.resize(c);
}

CLevelSkill *CLevelSkills::FindSkillByClientID(ClientSkillID idClient)
{
	for (int i=0;i<_skills.size();i++)
	{
		CLevelSkill *skill=_skills[i];
		if (skill->GetClientID()==idClient)
			return skill;
	}
	return NULL;

}

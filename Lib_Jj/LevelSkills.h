#pragma once

#include "LevelSkill.h"



class CLevelSkill;
class CLevelSkills
{
public:
	CLevelSkills()
	{
		_level=NULL;
	}
	BOOL Create(CLevel *level)
	{
		_level=level;
		return TRUE;
	}

	void Destroy();

	void Update();

	void AddSkill(CLevelSkill *skill)
	{
		skill->AddRef();
		_skills.push_back(skill);
	}

	CLevelSkill *FindSkillByClientID(ClientSkillID idClient);


protected:

	CLevel *_level;
	std::vector<CLevelSkill*>_skills;

};
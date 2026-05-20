#pragma once

#include "LevelSkill.h"


struct SkillParam_InvokeAgent:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_InvokeAgent);

	BEGIN_GOBJ_PURE(SkillParam_InvokeAgent,1);
		GELEM_VAR_INIT(unsigned __int64,idEffect,0);
			GELEM_EDITVAR("效果Proto",GVT_Bx8,GSem_ProtoPath,"主角的播放动作");

	END_GOBJ();

	unsigned __int64 idEffect;
};



class Skill_InvokeAgent:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_InvokeAgent,6);

	Skill_InvokeAgent()
	{
	}

	enum OpCode
	{
	};

	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return 1<<LevelSkillTarget::Target_DefObj;
	}
	virtual BOOL IsInvokingAgent()	{		return TRUE;	}



protected:
	virtual void _OnStart();
	virtual void _OnBreak()	{		_SetState(SkillState_Finished);	}




};


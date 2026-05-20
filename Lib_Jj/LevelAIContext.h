#pragma once

#include <unordered_set>

#include "class/class.h"

#include "LevelDefines.h"

enum LevelAIMentalState
{
	LevelAIMentalState_None,
	LevelAIMentalState_Relax,
	LevelAIMentalState_Alert,
	LevelAIMentalState_Combat,

	LevelAIMentalState_ForceDword=0xffffffff
};

#define LevelAIMentalState_SemConstraint "Relax:1,Alert:2,Combat:3"
inline const char *GetMentalStateName(LevelAIMentalState state)
{
	switch(state)
	{
		case LevelAIMentalState_Relax:
			return "Relax";
		case LevelAIMentalState_Alert:
			return "Alert";
		case LevelAIMentalState_Combat:
			return "Combat";
	}
	return "n/a";
}

struct LevelAIContext
{
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(LevelAIContext);

	static StringID GetStdCmd_Controlled();
	static StringID GetStdCmd_Combat();


	LevelAIContext()
	{
		idCmd=StringID_Invalid;
		stateMental=LevelAIMentalState_Relax;
	}

	StringID idCmd;
	LevelPos pos;

	DWORD GetThreatingCount()	{		return threatings.size();	}
	void AddThreating(LevelObjID id);
	void RemoveThreating(LevelObjID id);
	DWORD GetAlertedCount()	{		return alerteds.size();	}
	void AddAlerted(LevelObjID id);
	void RemoveAlerted(LevelObjID id);
	DWORD GetCombatedCount()	{		return combateds.size();	}
	void AddCombated(LevelObjID id);
	void RemoveCombated(LevelObjID id);

	LevelAIMentalState stateMental;
	std::unordered_set<LevelObjID> threatings;
	std::unordered_set<LevelObjID> alerteds;
	std::unordered_set<LevelObjID> combateds;

};


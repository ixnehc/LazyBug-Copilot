
#include "stdh.h"

#include "LevelAIContext.h"
 

StringID LevelAIContext::GetStdCmd_Controlled()
{
	static StringID idCmd=StringID_Invalid;
	if (idCmd!=StringID_Invalid)
		return idCmd;
	CStrLib *strlib=StrLib_Get();
	if (strlib)
	{
		idCmd=strlib->FindStr(0,"[Controlled]","AI命令");
		return idCmd;
	}

	return StringID_Invalid;
}

StringID LevelAIContext::GetStdCmd_Combat()
{
	static StringID idCmd=StringID_Invalid;
	if (idCmd!=StringID_Invalid)
		return idCmd;

	CStrLib *strlib=StrLib_Get();
	if (strlib)
	{
		idCmd=strlib->FindStr(0,"[Combat]","AI命令");
		return idCmd;
	}

	return StringID_Invalid;

}

void LevelAIContext::AddThreating(LevelObjID id)
{
	threatings.insert(id);
}

void LevelAIContext::RemoveThreating(LevelObjID id)
{
	std::unordered_set<LevelObjID>::iterator it=threatings.find(id);
	if (it!=threatings.end())
		threatings.erase(it);
}

void LevelAIContext::AddAlerted(LevelObjID id)
{
	alerteds.insert(id);
}

void LevelAIContext::RemoveAlerted(LevelObjID id)
{
	std::unordered_set<LevelObjID>::iterator it=alerteds.find(id);
	if (it!=alerteds.end())
		alerteds.erase(it);
}

void LevelAIContext::AddCombated(LevelObjID id)
{
	combateds.insert(id);
}

void LevelAIContext::RemoveCombated(LevelObjID id)
{
	std::unordered_set<LevelObjID>::iterator it=combateds.find(id);
	if (it!=combateds.end())
		combateds.erase(it);
}

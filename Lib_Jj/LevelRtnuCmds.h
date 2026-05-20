#pragma once

#include "class/class.h"

#include "LevelDefines.h"

#include "LevelObj.h"

struct LevelRtnuCmdRT:public LevelRtnuCmd
{
	LevelRtnuCmdRT()
	{
		tStart=0;
	}
	AnimTick tStart;
};


class CLoUnit;
class CLevelPlayer;
struct LPSRetinueData;
class CBehaviorPersist;
class CLevelBehavior;
class CLevelRtnuCmds
{
public:
	CLevelRtnuCmds()
	{
		Zero();
	}

	void Zero()
	{
		_level=NULL;
	}

	void Init(CLevel *level)
	{
		_level=level;
	}

	void Clear()
	{
		_cmds.clear();
		Zero();
	}

	BOOL Fetch(LevelObjID id,LevelRtnuCmd &cmd);
	BOOL Fetch(LevelObjID id,LevelRtnuCmdType tp,LevelRtnuCmd &cmd);
	void Add(LevelRtnuCmd &cmd);

protected:
	CLevel *_level;

	std::unordered_map<LevelObjID,LevelRtnuCmdRT> _cmds;
};
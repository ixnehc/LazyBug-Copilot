#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "LevelDefines.h"

#include "LevelObj.h"


class CLevelNPCs;
struct NPCDistribute;
struct GObjBase;
struct LevelRecordNPC;
struct LPSRetinueData;

class CLevelNPC
{
public:
	CLevelNPC()
	{
		_level=NULL;
		_rec=NULL;
		_distrib=NULL;
	}

	struct Entry
	{
		Entry()
		{
			bValid=0;
			loUnit=NULL;
			bhv=NULL;
			memset(&state,0,sizeof(state));
		}
		BYTE bValid:1;
		LevelNPCState state;
		CLoUnit *loUnit;
		CLevelBehavior *bhv;
	};


	BOOL Init(CLevel *level,LevelRecordNPC *rec,NPCDistribute *distrib);

	void Clear();
	void Update(AnimTick t);

	void AddPlayer(LevelPlayerID idPlayer);
	void RemovePlayer(LevelPlayerID idPlayer);

	virtual BOOL IsOwningLo(CLevelObj * lo);
	virtual BOOL SwitchRetinue(LevelPlayerID idPlayer,CLevelObj * lo);

protected:

	virtual void _OnUpdate(AnimTick t);

	//基本信息
	CLevel *_level;
	LevelRecordNPC *_rec;
	NPCDistribute *_distrib;//分布信息

	Entry _entries[LEVEL_MAX_PLAYER];


};

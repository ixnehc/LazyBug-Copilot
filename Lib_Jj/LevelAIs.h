#pragma once

#include "class/class.h"
#include <unordered_map>

#include "LevelDefines.h"

class CLevelTroops;
class CLevelAI
{
public:
	DEFINE_CLASS(CLevelAI);
	CLevelAI()
	{
		Zero();
	}
	void Zero()
	{
		_level=NULL;
		_nmBg=StringID_Invalid;
		_troops=NULL;
		memset(_bhvs,0,sizeof(_bhvs));
	}
	void Init(CLevel *level,StringID nmBg)	{		_level=level;_nmBg=nmBg;	}
	void Clear();

	void VerifyPlayerAIs();

	void Update();

	CLevelTroops *ObtainTroops();
	CLevelTroops *GetTroops()	{		return _troops;	}

protected:

	void _LoadPersist(LevelPlayerStates *lps,CLevelBehavior *bhv);
	void _SavePersist(LevelPlayerStates *lps,CLevelBehavior *bhv);

	CLevel *_level;

	StringID _nmBg;

	CLevelBehavior* _bhvs[LEVEL_MAX_PLAYER];

	CLevelTroops *_troops;

	friend class CLevelAIs;
};


class CLevelAIs
{
public:
	DEFINE_CLASS(CLevelAIs);
	
	CLevelAIs()
	{
		Zero();
	}
	~CLevelAIs()
	{
		Clear();
	}
	void Zero()
	{
		_level=NULL;
	}
	void Init(CLevel *level);
	void Clear();

	void Update();

	void OnPlayerEnter();
	void OnPlayerLeave();

protected:

	void _VerifyPlayerAIs();

	std::vector<CLevelAI*>_ais;

	CLevel *_level;
};


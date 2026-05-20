
#pragma once

#include "LevelDefines.h"

enum LevelHookType
{
	LH_PlayerEnterLevel,
	LH_PlayerLeaveLevel,
	LH_PlayerEnterWorld,
	LH_PlayerLeaveWorld,

	LevelHookMax,

};

struct LevelHook_PlayerEnterLevel:public LevelHook
{
	virtual int GetType()
	{
		return LH_PlayerEnterLevel;
	}
	LevelPlayerID idPlayer;
};

struct LevelHook_PlayerLeaveLevel:public LevelHook
{
	virtual int GetType()
	{
		return LH_PlayerLeaveLevel;
	}
	LevelPlayerID idPlayer;
};

struct LevelHook_PlayerEnterWorld:public LevelHook
{
	virtual int GetType()
	{
		return LH_PlayerEnterWorld;
	}
	LevelPlayerID idPlayer;
};

struct LevelHook_PlayerLeaveWorld:public LevelHook
{
	virtual int GetType()
	{
		return LH_PlayerLeaveWorld;
	}
	LevelPlayerID idPlayer;
};

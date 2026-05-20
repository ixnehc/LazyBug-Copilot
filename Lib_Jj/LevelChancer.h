#pragma once

#include "LevelDefines.h"

typedef DWORD LevelChanceHandle;
#define LevelChanceHandle_Invalid (0)

struct LevelChanceResult
{
	float chance;//0..1之间的值
};

struct LevelChanceEntry
{
	DWORD wt;
	DWORD idxResult;
};

struct LevelChanceChannel
{
	DEFINE_CLASS(LevelChanceChannel);
	LevelChanceChannel()
	{
		wtTotal=0;
	}
	std::vector<LevelChanceEntry> entries;

	DWORD wtTotal;//所有entry总共的weight
};


class CLevel;
class CLoUnit;
class CLevelChanceData
{
public:
	CLevelChanceData()
	{
		_nResults=0;
		_chSpawner=NULL;
	}

	void Init();
	void Clear();

	LevelChanceHandle Register_Spawn(DWORD wt);


protected:
	LevelChanceChannel *_chSpawner;

	DWORD _nResults;

friend class CLevelChancer;
};

class CLevelChancer
{
public:
	CLevelChancer()
	{
		_data=NULL;
	}
	void Init(CLevelChanceData *data);
	void Clear();

	void MakeDice_Spawn(float rate);//rate为0..1之间的值,将指定的几率分布到给定的channel中去
	void MakeDice_Spawn(int wt);

	BOOL GetChance(LevelChanceHandle hChance,float &chance);

protected:

	void _MakeDice(LevelChanceChannel *ch,int nWt);
	CLevelChanceData *_data;
	std::deque<LevelChanceResult> _results;
};
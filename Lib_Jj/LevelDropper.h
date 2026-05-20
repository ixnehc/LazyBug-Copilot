#pragma once

#include "LevelDefines.h"
#include "class/class.h"

#include "LevelItemBuffType.h"

#define MAX_TC_COUNT 200

struct TreasureClass
{
	std::vector<LevelRecordItem *>items;
};

struct LevelRecordTC;
struct LevelRecordItemSeed;
class CLevelDropper
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CLevelDropper);
	CLevelDropper()
	{
		Zero();
	}
	~CLevelDropper()
	{
		Clear();
	}

	void Zero()
	{
		_records=NULL;
		_nResult=0;
		_nSoul=0;
		_nMP=0;
		_nCrystal=0;
		memset(_tcs,0,sizeof(_tcs));
	}
	void Init(CLevelRecords *records);
	void Clear();

	void MakeDrop(CLevelObj *loSrc,CLevelObj *loTarget);

	LevelItemState *GetResults(DWORD &c)
	{
		c=_nResult;
		return _results;
	}
	int GetResultSoul()
	{
		return _nSoul;
	}
	int GetResultMP()
	{
		return _nMP;
	}
	int GetResultCrystal()
	{
		return _nCrystal;
	}

protected:

	struct ItemBuffTypeEntry
	{
		EItemBuffType tp;
		float wt;
	};


	DWORD _CalcTryCount(LevelGrade grd);

	void _GenItemBuffs(LevelItemState &state,LevelGrade grd);

	void _GenItemBuff(LevelItemState &state,LevelRecordItemSeed *recSeed);

	CLevelRecords *_records;

	LevelItemState _results[16];
	DWORD _nResult;
	int _nSoul;
	int _nMP;
	int _nCrystal;

	TreasureClass _tcs[MAX_TC_COUNT];

	std::vector<LevelRecordItemSeed*>_seeds[ItemBuff_Max];//按照buff类型分类的seeds


	ItemBuffTypeEntry _entries[512];//临时Buffer


};


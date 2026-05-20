
#pragma once

#include "LevelDefines.h"

#include "LevelHookDefines.h"


class CLevelObj;

class CLevelHooks
{
public:
	struct HookNode
	{
		CLevelObj *lo;
		DWORD prior;
	};

	struct Entry
	{
		std::vector<HookNode> nodes;
	};

	CLevelHooks()
	{
		memset(_flagsHk,0,sizeof(_flagsHk));

		_lastgc=0;
	}
	BOOL Init();
	void Clear();

	BOOL SendHook(LevelHook &e);
	BOOL RegisterHook(LevelHookType tp,CLevelObj*lo,DWORD prior);

	void GarbageCollect(BOOL bFull);

protected:

	void _ClearEntry(Entry *entry);

	void _FlushDead();

	Entry _entriesHk[LevelHookMax];
	BYTE _flagsHk[LevelHookMax];//这个数组用来标记某个LevelHookType是否存在(是否曾经被注册(RegisterHook(..)过)
	std::vector<LevelHookType> _registereds;//存储所有被注册(RegisterHook(..))过的LevelHookType

	DWORD _lastgc;//上一次gc到哪?

};

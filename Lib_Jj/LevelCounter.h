#pragma once

#include "class/class.h"

#include "LevelDefines.h"

#include "MagicBoardDefines.h"

//各种计数器,内部使用
class CLevelCounter
{
public:
	DEFINE_CLASS(CLevelCounter)

	CLevelCounter()
	{
		memset(miningMB,0,sizeof(miningMB));
	}
	short miningMB[MBRes_ActualMax];

};



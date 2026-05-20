#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "LevelDefines.h"

#include "LevelAttrs.h"

#include "MBUtil.h"


class CLoMagicBoard;
struct MagicBoardAIContext
{
	MagicBoardAIContext()
	{
		level=NULL;
		lo=NULL;
		idPlayer=LevelPlayerID_Invalid;
	}
	CLevel *level;
	CLoMagicBoard *lo;
	LevelPlayerID idPlayer;
	std::vector<WORD> seals;//reachµÄseals
	std::vector<WORD> unseals;
	std::vector<WORD> commits;

	LevelAttr_MagicBoard attr;

};


class CMagicBoardAI
{
public:
	CMagicBoardAI()
	{
		_bhv=NULL;
	}
	void Init(CLoMagicBoard *loMB);
	void Clear();
	void Update();

	MagicBoardAIContext &GetCtx()	{		return _ctx;	}

protected:
	MagicBoardAIContext _ctx;

	CLevelBehavior *_bhv;
};
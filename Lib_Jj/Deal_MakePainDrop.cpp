#include "stdh.h"

#include "Deal_MakePainDrop.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

BIND_DEAL(Deal_MakePainDrop);

void Deal_MakePainDrop::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level) 
	{
		LevelStrike strike;
		strike.SetDir(0.0f,1.0f);
		level->GetDecider()->MakePainDrop(osbSrc,loTarget,strike,arg.link);
	}

}

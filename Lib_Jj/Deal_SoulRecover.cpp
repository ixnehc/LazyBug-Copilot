#include "stdh.h"

#include "Deal_SoulRecover.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

BIND_DEAL(Deal_SoulRecover);


void Deal_SoulRecover::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level)
		level->GetDecider()->MakeSoulRecover(loTarget,arg.amount);

}

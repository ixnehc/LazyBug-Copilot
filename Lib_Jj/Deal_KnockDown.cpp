#include "stdh.h"

#include "Deal_KnockDown.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

BIND_DEAL(Deal_KnockDown);


void Deal_KnockDown::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level) 
	{
		if (dur==0)
			level->GetDecider()->MakeKD(osbSrc,loTarget,idBuff,arg.dir.getXZ().safe_normalize(),arg.link,ANIMTICK_INFINITE);
		else
			level->GetDecider()->MakeKD(osbSrc,loTarget,idBuff,arg.dir.getXZ().safe_normalize(),arg.link,dur);
	}
}

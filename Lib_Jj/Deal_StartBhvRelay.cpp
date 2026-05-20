#include "stdh.h"

#include "Deal_StartBhvRelay.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

#include "LevelBuff.h"

BIND_DEAL(Deal_StartBhvRelay);


void Deal_StartBhvRelay::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level) 
	{
		if (loTarget)
		{
			extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
			if (CLevelBuff *buff=LevelUtil_FindBuffByRecordID(loTarget,idBuff))
			{
				CLevelBehavior *bhv=buff->GetBhv();
				if (bhv)
					bhv->StartRelay(nmRelay);
			}
		}
	}

}

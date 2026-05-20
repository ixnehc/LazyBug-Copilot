#include "stdh.h"

#include "Deal_CleanOwnerBuff.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "LevelAttrs.h"

#include "Level.h"

BIND_DEAL(Deal_CleanOwnerBuff);


void Deal_CleanOwnerBuff::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	if (osbSrc.b)
	{
		CLevel *level=osbSrc.GetLevel();
		CLevelObj *loSrc=osbSrc.GetOwner();
		if (loSrc&&level)
		{
			if (loSrc==loTarget)
			{
				level->GetDecider()->RemoveBuff(osbSrc,loTarget,osbSrc.b,arg.link);
			}
		}
	}
}

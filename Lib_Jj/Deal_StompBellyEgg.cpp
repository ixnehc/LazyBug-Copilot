#include "stdh.h"

#include "Deal_StompBellyEgg.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "behaviorgraph/BehaviorMem.h"
#include "LevelBehavior.h"

#include "Level.h"

#include "LoBelly.h"


BIND_DEAL(Deal_StompBellyEgg);

void Deal_StompBellyEgg::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level)
	{
		CLoBelly *loBelly=(CLoBelly *)level->GetUniqueObj(LevelUniqueObj_Belly);
		if (loBelly)
		{
			loBelly->StompEgg(loTarget,arg.link);
		}
	}

}

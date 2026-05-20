#include "stdh.h"

#include "Deal_ModSlatesRes.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "LevelAttrs.h"

#include "Level.h"

#include "LoSlatesA.h"

BIND_DEAL(Deal_ModSlatesRes);


void Deal_ModSlatesRes::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level)
	{
		CLevelObj *lo=osbSrc.GetOwner();
		if (lo)
		{
			if (lo->GetClass()->IsSameWith(Class_Ptr2(CLoSlatesA)))
			{
				CLoSlatesA *loSlates=(CLoSlatesA *)lo;
				loSlates->ModRes(_nMod);
			}
		}
	}
}

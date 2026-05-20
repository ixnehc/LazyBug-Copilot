#include "stdh.h"

#include "Deal_ModRes.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "LevelAttrs.h"

#include "Level.h"

BIND_DEAL(Deal_ModRes);


void Deal_ModRes::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level)
	{
		if (_nMod==0)
			level->GetDecider()->MakeResModify(osbSrc,loTarget,_tp,arg.amount,arg.link);
		else
			level->GetDecider()->MakeResModify(osbSrc,loTarget,_tp,_nMod,arg.link);
	}
}

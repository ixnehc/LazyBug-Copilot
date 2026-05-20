#include "stdh.h"

#include "Deal_KnockBack.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

BIND_DEAL(Deal_KnockBack);


void Deal_KnockBack::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level) 
	{
		LevelStrike strike;
		strike.idSrc=osbSrc.GetOwnerID();
		strike.SetDir(arg.dir.getXZ());
		strike.SetStr(_str);
		level->GetDecider()->MakeKB(osbSrc,loTarget,strike,arg.link);
	}
}

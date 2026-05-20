#include "stdh.h"

#include "Deal_Dmg.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

BIND_DEAL(Deal_Dmg);


void Deal_Dmg::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	LevelStrike strike;
	strike.idSrc=osbSrc.GetOwnerID();
	strike.SetDir(arg.dir.getXZ());
	strike.SetStr(_strKill);
//	strike.str=(BYTE)(_strKill*10.0f);

	CLevel *level=osbSrc.GetLevel();
	if (level)
		level->GetDecider()->MakeDamage(osbSrc,loTarget,strike,_attacks.Get(),arg.link,_tpBlock,arg.multiply);

}

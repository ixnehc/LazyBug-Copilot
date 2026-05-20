#include "stdh.h"

#include "Deal_CreateSpore.h"

#include "LevelOSB.h"
#include "LevelEvents.h"

#include "LevelRecordEO.h"
#include "LevelRecords.h"

#include "Log/LogDump.h"


#include "Level.h"

#include "EoEnv.h" 

BIND_DEAL(Deal_CreateSpore);


void Deal_CreateSpore::Make(LevelOSB &osbSrc,LevelPos3D &pos,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level)
	{
		EoEnv *eoEnv=(EoEnv *)level->GetEoEnv();
		if (eoEnv)
		{
			eoEnv->DetonateSpore(osbSrc,pos.getXZ(),_radiusDetonate,arg.link);
			eoEnv->SpawnSpore(pos.getXZ(),arg.link);
		}
	}
}

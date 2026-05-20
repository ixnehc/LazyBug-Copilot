#include "stdh.h"

#include "Deal_Dizzy.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

#include "LevelRecords.h"
#include "LevelRecordGlobal.h"

BIND_DEAL(Deal_Dizzy);


void Deal_Dizzy::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level) 
	{
		LevelStrike strike;
		strike.idSrc=osbSrc.GetOwnerID();
		strike.SetDir(arg.dir.getXZ());

		if (level->GetRecords()->GetGlobal()->idDefBuff_Dizzy!=RecordID_Invalid)
		if (dur==0)
			level->GetDecider()->MakeBuff(osbSrc,loTarget,level->GetRecords()->GetGlobal()->idDefBuff_Dizzy,ANIMTICK_INFINITE,NULL,arg.link);
		else
			level->GetDecider()->MakeBuff(osbSrc,loTarget,level->GetRecords()->GetGlobal()->idDefBuff_Dizzy,dur,NULL,arg.link);
	}
}

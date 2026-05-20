#include "stdh.h"

#include "Deal_RemoveBuff.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

BIND_DEAL(Deal_RemoveBuff);


void Deal_RemoveBuff::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level) 
	{
		if (bRemoveFromOwner)
			loTarget=osbSrc.GetOwner();
		if (loTarget)
		{
			extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
			for (int i=0;i<idsRequiredBuff.size();i++)
			{
				if (!LevelUtil_FindBuffByRecordID(loTarget,idsRequiredBuff[i]))
					return;
			}

			if (CLevelBuff *buff=LevelUtil_FindBuffByRecordID(loTarget,idBuff))
				level->GetDecider()->RemoveBuff(osbSrc,loTarget,buff,arg.link);
		}
	}

}

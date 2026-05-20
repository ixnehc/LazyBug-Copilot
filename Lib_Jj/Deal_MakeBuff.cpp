#include "stdh.h"

#include "Deal_MakeBuff.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

BIND_DEAL(Deal_MakeBuff);

extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);

void Deal_MakeBuff::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level) 
	{
		if (bAddToOwner)
			loTarget=osbSrc.GetOwner();

		for (int i=0;i<idsBlockerBuff.size();i++)
		{
			if (LevelUtil_FindBuffByRecordID(loTarget,idsBlockerBuff[i]))
				return;
		}

		for (int i=0;i<idsRequiredBuff.size();i++)
		{
			if (!LevelUtil_FindBuffByRecordID(loTarget,idsRequiredBuff[i]))
				return;
		}

		CLevelBuff *buff=LevelUtil_FindBuffByRecordID(loTarget,idBuff);
		if (!buff)
		{
			if (loTarget)
			{
				if (dur==0)
					level->GetDecider()->MakeBuff(osbSrc,loTarget,idBuff,ANIMTICK_INFINITE,NULL,arg.link);
				else
					level->GetDecider()->MakeBuff(osbSrc,loTarget,idBuff,dur,NULL,arg.link);
			}
		}
		else
		{
			if (opOnSameBuff==1)
				buff->MergeDur(dur);
			if ((opOnSameBuff==2)||(opOnSameBuff==3))
			{
				if (opOnSameBuff==3)
					level->GetDecider()->RemoveBuff(osbSrc,loTarget,buff,arg.link);

				if (dur==0)
					level->GetDecider()->MakeBuff(osbSrc,loTarget,idBuff,ANIMTICK_INFINITE,NULL,arg.link);
				else
					level->GetDecider()->MakeBuff(osbSrc,loTarget,idBuff,dur,NULL,arg.link);
			}

		}
	}

}

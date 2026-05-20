#include "stdh.h"

#include "Deal_Jink.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

#include "Buff_Jink.h"

BIND_DEAL(Deal_Jink);


void Deal_Jink::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level) 
	{
		if (idBuff!=RecordID_Invalid)
		{
			extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
			if (!LevelUtil_FindBuffByRecordID(loTarget,idBuff))
			{
				CLevelObj *loSrc=osbSrc.GetRootOwner();
				if (loSrc)
				{
					extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
					CLevelSkill *skill=LevelUtil_GetCastingSkill(loSrc);
					if (skill)
					{
						LevelSkillTarget &target=skill->GetTarget();
						extern BOOL LevelUtil_CalcTargetPos(CLevel *level,LevelSkillTarget &target,LevelPos&pos);
						LevelPos pos;
						if (LevelUtil_CalcTargetPos(level,target,pos))
						{
							BuffArg_Jink arg;
							arg.pos=pos;
							arg.face=loSrc->GetFrameFace();
							level->GetDecider()->MakeBuff(osbSrc,loSrc,idBuff,0,&arg,LevelOpLink());
						}
					}
				}
			}
		}
	}

}

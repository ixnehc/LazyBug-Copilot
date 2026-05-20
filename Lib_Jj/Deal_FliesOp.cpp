#include "stdh.h"

#include "Deal_FliesOp.h"

#include "Buff_Flies.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

BIND_DEAL(Deal_FliesOp);


void Deal_FliesOp::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level) 
	{
		extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
		CLevelBuff *buff=LevelUtil_FindBuffByRecordID(loTarget,idBuff);
		if (buff)
		{
			if (buff->GetClass()->IsSameWith(Class_Ptr2(Buff_Flies)))
			{
				Buff_Flies *buffFlies=(Buff_Flies *)buff;

				switch(op)
				{
					case OverrideEnchanted_On:
					{
						buffFlies->OverrideEnchanted(TRUE);
						break;
					}
					case OverrideEnchanted_Off:
					{
						buffFlies->OverrideEnchanted(FALSE);
						break;
					}
					case ClearOverrideEnchanted:
					{
						buffFlies->ClearOverrideEnchanted();
						break;
					}
					case SetForm_Default:
					{
						buffFlies->SetForm(Buff_Flies::Form_Default);
						break;
					}
					case SetForm_Scattered:
					{
						buffFlies->SetForm(Buff_Flies::Form_Scattered);
						break;
					}
				}
			}
		}
	}
}

#include "stdh.h"

#include "Deal_CureSP.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "LevelAttrs.h"

#include "Level.h"

BIND_DEAL(Deal_CureSP);


void Deal_CureSP::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	LevelStrike strike;
	strike.idSrc=osbSrc.GetOwnerID();

	CLevel *level=osbSrc.GetLevel();
	if (level)
	{
		switch(_mode)
		{
			case 0:
				level->GetDecider()->CommitSPMod((float)_nCure,osbSrc,loTarget,arg.link,TRUE);
				break;
			case 1:
			{
				LevelAttr_Base *attr=loTarget->GetAttr_Base();
				if (attr)
				{
					float nCure=((float)attr->hp.GetMax_Int())*_rateCure;
					level->GetDecider()->CommitSPMod(nCure,osbSrc,loTarget,arg.link,TRUE);
				}
				break;
			}
			case 2:
			{
				CLevelObj *loOwner=osbSrc.GetOwner();
				if (loOwner)
				{
					LevelAttr_Base *attr=loOwner->GetAttr_Base();
					if (attr)
						level->GetDecider()->CommitSPMod(attr->hp.GetCur_Float(),osbSrc,loTarget,arg.link,TRUE);
				}
				break;
			}
		}
	}
}

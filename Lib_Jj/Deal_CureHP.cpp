#include "stdh.h"

#include "Deal_CureHP.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "LevelAttrs.h"

#include "Level.h"

BIND_DEAL(Deal_CureHP);


void Deal_CureHP::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	LevelStrike strike;
	strike.idSrc=osbSrc.GetOwnerID();

	CLevel *level=osbSrc.GetLevel();
	if (level)
	{
		switch(_mode)
		{
			case 0:
				if (!_bMaxHP)
					level->GetDecider()->MakeCure(osbSrc,loTarget,_nCure,strike,arg.link);
				else
					level->GetDecider()->MakeCure_MaxHP(osbSrc,loTarget,_nCure,arg.link);
				break;
			case 1:
			{
				LevelAttr_Base *attr=loTarget->GetAttr_Base();
				if (attr)
				{
					int nCure=(int)(((float)attr->hp.GetMax_Int())*_rateCure);
					if (!_bMaxHP)
						level->GetDecider()->MakeCure(osbSrc,loTarget,nCure,strike,arg.link);
					else
						level->GetDecider()->MakeCure_MaxHP(osbSrc,loTarget,nCure,arg.link);
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
					{
						if (!_bMaxHP)
							level->GetDecider()->MakeCure(osbSrc,loTarget,attr->hp.GetCur_Int(),strike,arg.link);
						else
							level->GetDecider()->MakeCure_MaxHP(osbSrc,loTarget,attr->hp.GetCur_Int(),arg.link);
					}
				}
				break;
			}
			case 3:
			{
				CLevelObj *loOwner=osbSrc.GetOwner();
				if (loOwner)
				{
					LevelAttr_Base *attr=loOwner->GetAttr_Base();
					if (attr)
					{
						LevelAttr_Base *attrTarget=loTarget->GetAttr_Base();
						if (attrTarget)
						{
							int nCure=(int)(((float)attrTarget->hp.GetMax_Int())*attr->hp.GetRatio());
							if (!_bMaxHP)
								level->GetDecider()->MakeCure(osbSrc,loTarget,nCure,strike,arg.link);
							else
								level->GetDecider()->MakeCure_MaxHP(osbSrc,loTarget,nCure,arg.link);
						}
					}
				}
				break;
			}
		}
	}
}

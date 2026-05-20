#include "stdh.h"

#include "Deal_Suck.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "LevelAttrs.h"

#include "Level.h"

BIND_DEAL(Deal_Suck);


void Deal_Suck::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	LevelStrike strike;
	strike.idSrc=osbSrc.GetOwnerID();

	CLevel *level=osbSrc.GetLevel();
	if (level)
	{
		switch(_mode)
		{
			case 0:
			{
				level->GetDecider()->MakeSuck_HP(osbSrc,loTarget,_nSuck,arg.link);
				break;
			}
			case 1:
			{
				assert(FALSE);
				break;
			}
		}
	}
}

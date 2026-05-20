#include "stdh.h"

#include "Deal_IncMP.h"

#include "LevelOSB.h"

#include "Level.h"

#include "Ability_MagicRing.h"

BIND_DEAL(Deal_IncMP);


void Deal_IncMP::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	extern CLevelAbility *LevelUtil_GetActiveAbility(CLevelObj *lo,LevelAbilityType tp);
	CLevelAbility_MagicRing *ability=(CLevelAbility_MagicRing *)LevelUtil_GetActiveAbility(loTarget,LevelAbilityType_MagicRing);
	if (ability)
		ability->IncMP(arg.amount);
}

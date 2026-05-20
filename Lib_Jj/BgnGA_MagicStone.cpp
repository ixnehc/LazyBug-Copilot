/********************************************************************
	created:	2022/07/22 
	author:		cxi
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelUtil.h"

#include "Ability_MagicRing.h"

#include "BgnGA_MagicStone.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_MagicStoneReward

BIND_BGN_CLASS(CBgnGA_MagicStoneReward,CBgpGA_MagicStoneReward);


void CBgnGA_MagicStoneReward::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelPlayer *player=_GetTalkPlayer();

	if (player)
	{
		CLevelAbility_MagicRing *ability=(CLevelAbility_MagicRing *)LevelUtil_GetActiveAbility(player,LevelAbilityType_MagicRing);
		if (ability)
		{
			ability->ResetMP();
			ability->IncGrade();
		}
	}

	_OutputOk(outputs,1,"结束");

}

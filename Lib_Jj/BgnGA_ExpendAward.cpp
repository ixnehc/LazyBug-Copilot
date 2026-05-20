/********************************************************************
	created:	2022/8/2 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"
#include "LevelRecordUpgrade.h"

#include "LevelOSB.h"

#include "BgnGA_RollAwards.h"
#include "BgnGA_ExpendAward.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelAbility.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_ExpendAward
BIND_BGN_CLASS(CBgnGA_ExpendAward,CBgpGA_ExpendAward);

BOOL CBgnGA_ExpendAward::_Expend(CLevelPlayer *player,LevelAward *award)
{
	extern BOOL LevelUtil_ExpendAward(CLevelPlayer *player,LevelAward *award);
	if (!LevelUtil_ExpendAward(player,award))
		return FALSE;

	return TRUE;
}

void CBgnGA_ExpendAward::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_ExpendAward*pad=_GetPad<CBgpGA_ExpendAward>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	CLevelPlayer *player=_GetTalkPlayer();

	LevelBehaviorContext *ctx=_GetCtx();

	if (TRUE)
	{
		RollAwardsResult *result=_GetMem()->GetObj<RollAwardsResult>(pad->award);

		if (result)
		{
			if (pad->idxAward<(int)result->awards.size())
			{
				LevelBehaviorContext *ctx=_GetCtx();
				if (player)
				{
					LevelPlayerStates *lps=player->GetLPS();
					if (lps)
					{
						if (pad->idxAward<0)
						{
							for (int i=0;i<result->awards.size();i++)
								_Expend(player,&result->awards[i]);
						}
						else
						{
							_Expend(player,&result->awards[pad->idxAward]);
						}
					}
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}

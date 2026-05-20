/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelBehavior.h"
#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelRtnus.h"
#include "LevelTroops.h"
#include "BgnTroop_SwitchRetinue.h"

#include "LevelSkillDriver.h"
#include "LoUnit.h"



#include "LevelUtil.h"


////////////////////////////////////////////////////////////////////////
//CBgnTroop_SwitchRetinue
BIND_BGN_CLASS(CBgnTroop_SwitchRetinue,CBgpTroop_SwitchRetinue);

void CBgnTroop_SwitchRetinue::Destroy()
{
}


void CBgnTroop_SwitchRetinue::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_SwitchRetinue*pad=_GetPad<CBgpTroop_SwitchRetinue>();
	CLevel *level=_GetLevel();
	LevelBehaviorContext *ctx=_GetCtx();

	CLevelTroop *troop=_GetTroop(pad->_troop);
	if (troop)
	{
		extern CLevelObj *LevelUtil_DetectClosestPlayer(CLevelObj *lo,float range);
		CLevelObj *loPlayer=LevelUtil_DetectClosestPlayer(ctx->lo,20.0f);
		if (loPlayer)
		{
			extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
			CLevelPlayer *player=LevelUtil_PlayerFromLo(loPlayer);
			if (player)
			{
				DWORD c=troop->GetFrameCount();
				for (int i=0;i<c;i++)
				{
					LevelTroopFrame *frm=troop->GetFrame(i);
					if (frm)
					{
						if (frm->idUnit!=LevelObjID_Invalid)
						{
							extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
							CLevelObj *loUnit = LevelUtil_GetAliveLo(level,frm->idUnit);
							extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
							if (!LevelUtil_CheckDead(loUnit))
							{
								if (loUnit->GetType()==LevelObjType_Unit)
								{
									if (player->GetRtnus())
										player->GetRtnus()->Add_New((CLoUnit*)loUnit,TRUE);
								}
							}
						}
					}
				}
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
	}
	_OutputFail(outputs,2,"失败");
}


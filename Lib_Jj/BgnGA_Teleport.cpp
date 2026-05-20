/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:创建道具
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Protocal.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "BgnGA_Teleport.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "Log/LogDump.h"
#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgnGA_Teleport
BIND_BGN_CLASS(CBgnGA_Teleport,CBgpGA_Teleport);

void CBgnGA_Teleport::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_Teleport*pad=_GetPad<CBgpGA_Teleport>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	if (level)
	{
		BP_TeleportSite*param=&pad->param;

		if (param)
		{
			CLevelObj *loPlayer=_GetTalkLo();
			if (loPlayer)
			{
				LevelTeleportQuest *quest=Class_New2(LevelTeleportQuest);
				if (pad->tpTarget==LevelTeleportTarget::None)
				{
					quest->idMap=param->idMap;
					quest->idSite=param->nmSite;
				}
				else
				{
					LevelTeleportTarget target;
					level->GetWorld()->GetTeleportTarget(pad->tpTarget,target);
					quest->idMap=target.idMap;
					if (target.nmSite!=StringID_Invalid)
						quest->idSite=target.nmSite;
					else
					{
						quest->bSitePos=TRUE;
						quest->posSite=target.posSite;
					}
				}

				quest->loPlayer=loPlayer;
				quest->loPlayer->AddRef();

				quest->tDoTeleport=level->GetT_()+ANIMTICK_FROM_SECOND(0.4f);

				level->AddTeleportQuest(quest);
			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;

}

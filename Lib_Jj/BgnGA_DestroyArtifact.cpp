/********************************************************************
	created:	2022/4/19 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "LevelOSB.h"

#include "BgnGA_DestroyArtifact.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_DestroyArtifact
BIND_BGN_CLASS(CBgnGA_DestroyArtifact,CBgpGA_DestroyArtifact);


void CBgnGA_DestroyArtifact::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_DestroyArtifact*pad=_GetPad<CBgpGA_DestroyArtifact>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	void LevelUtil_RemoveArtifact(CLevelPlayer *player,LevelArtifactType tpArtifact);
	CLevelPlayer *player=_GetTalkPlayer();
	if (player)
	{
		if (pad->tpArtifact!=LevelArtifact_None)
			LevelUtil_RemoveArtifact(player,pad->tpArtifact);
		else
		{
			LevelRecordItem *rec=level->GetRecords()->GetItem(pad->idArtifact);
			if (rec)
			{
				if(rec->tpArtifact!=LevelArtifact_None)
					LevelUtil_RemoveArtifact(player,rec->tpArtifact);
			}

		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}

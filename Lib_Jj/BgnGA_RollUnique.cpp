/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:创建道具
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h" 

#include "LevelRecords.h" 
#include "LevelRecordItem.h"

#include "LevelOSB.h"

#include "BgnGA_RollUnique.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoItem.h"


#include "Log/LogDump.h"
#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgnGA_RollUnique
BIND_BGN_CLASS(CBgnGA_RollUnique,CBgpGA_RollUnique);


void CBgnGA_RollUnique::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_RollUnique*pad=_GetPad<CBgpGA_RollUnique>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	std::vector<RollUniqueEntry*> entries;
	entries.reserve(16);

	BOOL bOk=FALSE;
	if (pad->varResult!=StringID_Invalid)
	{
		_GetMem()->SetID(pad->varResult,BehaviorMemType_ItemRecord,RecordID_Invalid);

		extern CLevelPlayer *LevelUtil_GetFirstPlayer(CLevel *level);
		CLevelPlayer *player=LevelUtil_GetFirstPlayer(level);
		if (player)
		{
			LevelPlayerStates *lps=player->GetLPS();
			if (lps)
			{
				for (int i=0;i<pad->param.passes.size();i++)
				{
					RollUniquePass &pass=pad->param.passes[i];

					entries.clear();
					for (int j=0;j<pass.entries.size();j++)
					{
						RollUniqueEntry *e=&pass.entries[j];
						if (e->idItem==RecordID_Invalid)
							continue;
						if (LPS_FindArtifact(lps,e->idItem))
							continue;
						if (LPS_CheckItemMemory(lps,e->idItem))
							continue;

						entries.push_back(e);
					}

					if (entries.size()<=0)
						continue;

					RollUniqueEntry *entry=CSysRandom::RollWeighted<RollUniqueEntry>(entries);
					if (entry)
					{
						_GetMem()->SetID(pad->varResult,BehaviorMemType_ItemRecord,entry->idItem);
						bOk=TRUE;
						break;
					}

				}
			}
		}
	}

	if (bOk)
		_OutputOk(outputs,1,"成功");
	else
		_OutputFail(outputs,2,"失败");
}

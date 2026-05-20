
#include "stdh.h"

#include "Level.h"

#include "LoSlideway.h"

#include "Random/Random.h"
//#include "Log/LogDump.h"

#include "LevelRecords.h"
#include "LevelRecordGlobal.h"


BOOL CLoSlideway::OnActivate()
{
  
	return TRUE;
}


void CLoSlideway::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if (!_bLoaded)
		_LoadPersistS(idPlayer);

	bp->Bit_Write(_bReached);

	bContent=TRUE;
}

void CLoSlideway::_LoadPersistS(LevelPlayerID idPlayer)
{
	LevelGUID guid=_GetGUID();
	if (guid==LevelGUID_Invalid)
		return;

	CLevelPlayer *player=_level->GetPlayer(idPlayer);
	if (player)
	{
		LevelPlayerStates *lps=player->GetLPS();
		extern LevelPersistEntry_AgentS *LPS_FindPersistEntry_AgentS(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
		LevelPersistEntry_AgentS *entry=LPS_FindPersistEntry_AgentS(lps,_level->GetMapID(),guid);
		if (entry)
		{
			_bReached=entry->mem_.flag0;
			_bLoaded=TRUE;
		}
	}

}

void CLoSlideway::_SavePersistS(LevelPlayerID idPlayer)
{
	LevelGUID guid=_GetGUID();
	if (guid==LevelGUID_Invalid)
		return;

	CLevelPlayer *player=_level->GetPlayer(idPlayer);
	if (player)
	{
		LevelPlayerStates *lps=player->GetLPS();
		extern LevelPersistEntry_AgentS *LPS_QueryPersistEntry_AgentS(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
		LevelPersistEntry_AgentS *entry=LPS_QueryPersistEntry_AgentS(lps,_level->GetMapID(),guid);
		if (entry)
		{
			entry->mem_.flag0=_bReached;
		}
	}
}

void CLoSlideway::NotifyReached()
{
	if (!_bReached)
	{
		_bReached=1;

		DWORD c;
		LevelPlayerID *ids=_level->GetPlayerIDs(c);
		for (int i=0;i<c;i++)
			_SavePersistS(ids[i]);

		LevelRecordGlobal *recGlobal=_level->GetRecords()->GetGlobal();
		if (recGlobal->slidewaysetting.nmReachedSignal!=StringID_Invalid)
		{
			_level->GetEventMap()->AddSignal(recGlobal->slidewaysetting.nmReachedSignal,GetFramePos(),recGlobal->slidewaysetting.radiusSignal,GetID());
		}

	}
}

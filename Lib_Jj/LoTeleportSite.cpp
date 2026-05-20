
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "LoTeleportSite.h"

#include "LevelRecords.h"

#include "LevelPersistData.h"

     
 
BOOL CLoTeleportSite::_LoadOccupy(LevelPlayerID idPlayer)
{
	BOOL bOccupy=FALSE;
	LevelPlayerStates *lps=_level->GetLPS(idPlayer);
	if (lps)
	{
		CLevelObjParam*lop=GetLop();
		if (lop)
		{
			LevelGUID guid=lop->GetGUID_();

			if (guid!=LevelGUID_Invalid)
			{
				RecordID idMap=_level->GetMapID();
				LevelPersistEntry_Agent*entry=LPS_FindPersistEntry_Agent(lps,idMap,guid);
				if (entry)
				{
					if (entry->data[0]!=0)
						bOccupy=TRUE;
				}
			}
		}
	}
	return bOccupy;
}

void CLoTeleportSite::_SaveOccupy(LevelPlayerID idPlayer)
{
	extern BOOL LevelUtil_CheckDay(LevelPlayerID idPlayer,CLoAgent *loAgent);
	LevelUtil_CheckDay(idPlayer,this);
}


void CLoTeleportSite::PostCreate()
{
	CLoAgent::PostCreate();

	_RegisterLevelHook(LH_PlayerEnterLevel,LevelHookPriority_Default);
	_RegisterLevelHook(LH_PlayerLeaveLevel,LevelHookPriority_Default);

	_occupies=0;
	for (int i=0;i<LEVEL_MAX_PLAYER;i++)
	{
		BOOL bOccupy=_LoadOccupy((LevelPlayerID)i);
		if (bOccupy)
			_occupies|=(1<<i);
	}

}

void CLoTeleportSite::OnDestroy()
{
}


BOOL CLoTeleportSite::OnActivate()
{

	return TRUE;
}

void CLoTeleportSite::HandleHook(LevelHook &hk0)
{
	switch(hk0.GetType())
	{
		case LH_PlayerEnterLevel:
		{
			LevelHook_PlayerEnterLevel *hk=(LevelHook_PlayerEnterLevel *)&hk0;
			if (_LoadOccupy(hk->idPlayer))
				_occupies|=(1<<hk->idPlayer);
			break;
		}
		case LH_PlayerLeaveLevel:
		{
			LevelHook_PlayerLeaveLevel*hk=(LevelHook_PlayerLeaveLevel*)&hk0;
			_occupies&=(~(1<<hk->idPlayer));
			break;
		}
	}
}


void CLoTeleportSite::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bContent=TRUE;

	BOOL bOccupy=_occupies&(1<<idPlayer);

	bp->Bit_Write(bOccupy);
}


void CLoTeleportSite::Update()
{
	for (int i=0;i<LEVEL_MAX_PLAYER;i++)
	{
		if (_occupies&(1<<i))
			continue;//已经占有了
		CLevelPlayer *player=_level->GetPlayer((LevelPlayerID)i);
		if (player)
		{
			CLoUnit *lo=player->GetLoUnit();
			if (lo->GetFramePos().getDistanceSQFrom(GetFramePos())<5.0f*5.0f)
			{
				_occupies|=(1<<i);
				_SaveOccupy((LevelPlayerID)i);
			}
		}
	}
}

void CLoTeleportSite::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if((_occupiesLast&(1<<idPlayer))!=(_occupies&(1<<idPlayer)))
	{
		bContent=1;
		bp->Bit_Write(1);//有变化
		bp->Bit_Write(_occupies&(1<<idPlayer));
	}
	else
		bp->Bit_Write(0);//没有变化
}

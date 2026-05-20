
#include "stdh.h"

#include "Level.h"

#include "LoTeleporter.h"

#include "Random/Random.h"
#include "Log/LogDump.h"

#include "LevelRecords.h"


BOOL CLoTeleporter::OnActivate()
{
	CSysRandom rand;

  
	return TRUE;
}


void CLoTeleporter::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
}



void CLoTeleporter::Invoke(CLevelObj *loFrom)
{
	LopTeleporter *lop=GetLop<LopTeleporter>();
	if (lop)
	{
		if (lop->idMap!=RecordID_Invalid)
		{
			if (lop->nmSite!=StringID_Invalid)
			{
				if (loFrom->IsPlayer())
				{
					LevelTeleportQuest *quest=Class_New2(LevelTeleportQuest);
					quest->idMap=lop->idMap;
					quest->idSite=lop->nmSite;

					quest->loPlayer=loFrom;
					quest->loPlayer->AddRef();

					quest->tDoTeleport=_level->GetT_()+ANIMTICK_FROM_SECOND(0.4f);

					_level->AddTeleportQuest(quest);
				}
			}
			else
			{
				LOG_DUMP("LoTeleporter",Log_Error,"未指定传送点!");
			}
		}
		else
		{
			LOG_DUMP("LoTeleporter",Log_Error,"未指定地图!");
		}
	}


}

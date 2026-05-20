
#include "stdh.h"
#include "Level.h"
#include "LevelPersistData.h"
#include "Protocal.h"


////////////////////////////////////////////////////////////////////////
//LPSPersistSetS

void LevelPersistData_AgentS::Save(CDataPacket &dp)
{
	dp.Data_NextWord()=entries.size();

	std::unordered_map<LevelGUID,LevelPersistEntry_AgentS>::iterator it;
	for (it=entries.begin();it!=entries.end();it++)
	{
		dp.Data_WriteSimple((*it).first);
		LevelPersistEntry_AgentS *entry=&((*it).second);
		entry->mem_.Save(dp);
	}
}

void LevelPersistData_AgentS::Load(CDataPacket &dp)
{
	WORD sz=dp.Data_NextWord();
	for (int i=0;i<sz;i++)
	{
		LevelGUID guid;
		dp.Data_ReadSimple(guid);
		LevelPersistEntry_AgentS entry;
		entry.mem_.Load(dp);
		entries[guid]=entry;
	}
}



//////////////////////////////////////////////////////////////////////////
//LevelPersistData_AgentBrief
void SendNetMsg_PersistEntry_AgentBrief(CLevel *level,LevelPlayerID idPlayer,LevelGUID guid,LevelAgentBrief &entry)
{
	SCAgentBrief msg;
	msg.guid=guid;

	DP_BeginSave(dp,msg.data);
	entry.Save(dp);
	DP_EndSave();

	level->SendNetMsg(idPlayer,&msg);
}

void LevelPersistData_AgentBrief::Save(CDataPacket &dp)
{
	dp.Data_NextWord()=entries.size();

	std::unordered_map<LevelGUID,LevelAgentBrief>::iterator it;
	for (it=entries.begin();it!=entries.end();it++)
	{
		dp.Data_WriteSimple((*it).first);
		LevelAgentBrief *entry=&((*it).second);
		entry->Save(dp);
	}
}

void LevelPersistData_AgentBrief::Load(CDataPacket &dp)
{
	WORD sz=dp.Data_NextWord();
	for (int i=0;i<sz;i++)
	{
		LevelGUID guid;
		dp.Data_ReadSimple(guid);
		entries[guid].Load(dp);
	}

}

void LevelPersistData_AgentBrief::ClearCur()
{
	std::unordered_map<LevelGUID,LevelAgentBrief>::iterator it;
	for (it=entries.begin();it!=entries.end();it++)
	{
		LevelAgentBrief *entry=&((*it).second);
		entry->ClearCur();
	}
}


////////////////////////////////////////////////////////////////////////
//LPSPersistSet

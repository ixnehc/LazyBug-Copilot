#pragma once


#include "LevelDefines.h"

#include "LevelAgentBrief.h"

class CLevel;

//S代表Simple
struct LevelPersistData_AgentS
{
	DEFINE_CLASS(LevelPersistData_AgentS);

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);
	void CopyFrom(LevelPersistData_AgentS*src)
	{
		entries=src->entries;
	}
	void Clear()
	{
		entries.clear();
	}

	LevelPersistEntry_AgentS*Find(LevelGUID guid)
	{
		std::unordered_map<LevelGUID,LevelPersistEntry_AgentS>::iterator it=entries.find(guid);
		if (it==entries.end())
			return NULL;
		return &(*it).second;
	}
	void Erase(LevelGUID guid)
	{
		std::unordered_map<LevelGUID,LevelPersistEntry_AgentS>::iterator it=entries.find(guid);
		if (it!=entries.end())
			entries.erase(it);
	}
	LevelPersistEntry_AgentS*Obtain(LevelGUID guid)
	{
		return &entries[guid];
	}

	void Set(LevelGUID guid,LevelPersistEntry_AgentS&v)
	{
		entries[guid]=v;
	}
	std::unordered_map<LevelGUID,LevelPersistEntry_AgentS> entries;

};


struct LevelPersistData_AgentBrief
{
	DEFINE_CLASS(LevelPersistData_AgentBrief);

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);
	void CopyFrom(LevelPersistData_AgentBrief*src)
	{
		entries=src->entries;
	}
	void Clear()
	{
		entries.clear();
	}
	void ClearCur();

	LevelAgentBrief*Find(LevelGUID guid)
	{
		std::unordered_map<LevelGUID,LevelAgentBrief>::iterator it=entries.find(guid);
		if (it==entries.end())
			return NULL;
		return &(*it).second;
	}
	void Erase(LevelGUID guid)
	{
		std::unordered_map<LevelGUID,LevelAgentBrief>::iterator it=entries.find(guid);
		if (it!=entries.end())
			entries.erase(it);
	}
	LevelAgentBrief*Obtain(LevelGUID guid)
	{
		return &entries[guid];
	}

	void Set(LevelGUID guid,LevelAgentBrief&v)
	{
		entries[guid].CopyFrom(v);
	}
	std::unordered_map<LevelGUID,LevelAgentBrief> entries;

};


template <int T_size,typename T_keytype=LevelGUID>
struct LevelPersistDataFixedSize
{
	void Save(CDataPacket &dp)
	{
		dp.Data_NextWord()=entries.size();

		std::unordered_map<T_keytype,LevelPersistEntryFixedSize<T_size>>::iterator it;
		for (it=entries.begin();it!=entries.end();it++)
		{
			dp.Data_WriteSimple((*it).first);
			LevelPersistEntryFixedSize<T_size> *entry=&((*it).second);
			dp.Data_WriteData(entry,entry->szData+1);
		}
	}
	void Load(CDataPacket &dp)
	{
		WORD sz=dp.Data_NextWord();
		for (int i=0;i<sz;i++)
		{
			T_keytype guid;
			dp.Data_ReadSimple(guid);
			LevelPersistEntryFixedSize<T_size> entry,*pEntry;
			pEntry=(LevelPersistEntryFixedSize<T_size>*)dp.GetCurBufferPointer();
			dp.Data_ReadData(&entry,pEntry->szData+1);
			entries[guid]=entry;
		}
	}
	void CopyFrom(LevelPersistDataFixedSize<T_size>*src)
	{
		entries=src->entries;
	}
	void Clear()
	{
		entries.clear();
	}

	LevelPersistEntryFixedSize<T_size> *Find(LevelGUID guid)
	{
		std::unordered_map<T_keytype,LevelPersistEntryFixedSize<T_size> >::iterator it=entries.find(guid);
		if (it==entries.end())
			return NULL;
		return &(*it).second;
	}
	void Erase(LevelGUID guid)
	{
		std::unordered_map<T_keytype,LevelPersistEntryFixedSize<T_size> >::iterator it=entries.find(guid);
		if (it!=entries.end())
			entries.erase(it);
	}
	LevelPersistEntryFixedSize<T_size> *Obtain(T_keytype guid)
	{
		return &entries[guid];
	}

	void Set(T_keytype guid,LevelPersistEntryFixedSize<T_size> &v)
	{
		entries[guid]=v;
	}
	std::unordered_map<T_keytype,LevelPersistEntryFixedSize<T_size> > entries;
};


//512byte 的储藏数据块
struct LevelPersistData_Agent:public LevelPersistDataFixedSize<512,LevelGUID>
{
	DEFINE_CLASS(LevelPersistData_Agent);
};


//512byte 的储藏数据块
struct LevelPersistData_LevelAI:public LevelPersistDataFixedSize<512,StringID>
{
	DEFINE_CLASS(LevelPersistData_LevelAI);
};

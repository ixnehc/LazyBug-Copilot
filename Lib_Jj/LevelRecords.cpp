
#include "stdh.h"

#include <fstream>

#include "records/records.h"

#include "LevelRecords.h"

#include "log/LogDump.h"

#include "resdata/ResDataDefines.h" 

#include "stringparser/stringparser.h"
   

#include "LevelRecordGlobal.h"
#include "LevelRecordUnit.h"
#include "LevelRecordSkill.h"
#include "LevelRecordItem.h"
#include "LevelRecordBuff.h"
#include "LevelRecordRegion.h"
#include "LevelRecordFlock.h"
#include "LevelRecordPosture.h"
#include "LevelRecordItemClass.h"
#include "LevelRecordAgent.h"
#include "LevelRecordGesture.h"
#include "LevelRecordNPC.h"
#include "LevelRecordMap.h"
#include "LevelRecordEffectSet.h"
#include "LevelRecordEo.h"
#include "LevelRecordItemSeed.h"
#include "LevelRecordMagicBoard.h"
#include "LevelRecordMagicTile.h"
#include "LevelRecordUpgrade.h"
#include "LevelRecordRes.h"
#include "LevelRecordSlateType.h"
#include "LevelRecordGoods.h"
//XXXXX: More LevelRecord


LevelRecordEntry *GetLevelRecordEntries(DWORD &c)
{
	static std::vector<LevelRecordEntry>entries;
	if (entries.size()<=0)
	{
		entries.push_back(LevelRecordEntry("Global",Class_Ptr2(LevelRecordGlobal)));
		entries.push_back(LevelRecordEntry("Units",Class_Ptr2(LevelRecordUnit)));
		entries.push_back(LevelRecordEntry("Skills",Class_Ptr2(LevelRecordSkill)));
		entries.push_back(LevelRecordEntry("Buffs",Class_Ptr2(LevelRecordBuff)));
		entries.push_back(LevelRecordEntry("Items",Class_Ptr2(LevelRecordItem)));
		entries.push_back(LevelRecordEntry("Regions",Class_Ptr2(LevelRecordRegion)));
		entries.push_back(LevelRecordEntry("Flocks",Class_Ptr2(LevelRecordFlock)));
		entries.push_back(LevelRecordEntry("ItemClasses",Class_Ptr2(LevelRecordItemClass)));
		entries.push_back(LevelRecordEntry("Postures",Class_Ptr2(LevelRecordPosture)));
		entries.push_back(LevelRecordEntry("Agents",Class_Ptr2(LevelRecordAgent)));
		entries.push_back(LevelRecordEntry("Gestures",Class_Ptr2(LevelRecordGesture)));
		entries.push_back(LevelRecordEntry("NPCs",Class_Ptr2(LevelRecordNPC)));
		entries.push_back(LevelRecordEntry("Maps",Class_Ptr2(LevelRecordMap)));
		entries.push_back(LevelRecordEntry("EffectSets",Class_Ptr2(LevelRecordEffectSet)));
		entries.push_back(LevelRecordEntry("EOs",Class_Ptr2(LevelRecordEo)));
		entries.push_back(LevelRecordEntry("ItemSeeds",Class_Ptr2(LevelRecordItemSeed)));
		entries.push_back(LevelRecordEntry("MagicBoards",Class_Ptr2(LevelRecordMagicBoard)));
		entries.push_back(LevelRecordEntry("MagicTiles",Class_Ptr2(LevelRecordMagicTile)));
		entries.push_back(LevelRecordEntry("Upgrades",Class_Ptr2(LevelRecordUpgrade)));
		entries.push_back(LevelRecordEntry("Resources",Class_Ptr2(LevelRecordRes)));
		entries.push_back(LevelRecordEntry("SlateTypes",Class_Ptr2(LevelRecordSlateType)));
		entries.push_back(LevelRecordEntry("Goods",Class_Ptr2(LevelRecordGoods)));
		//XXXXX: More LevelRecord
	}

	c=entries.size();
	return entries.data();
}

//////////////////////////////////////////////////////////////////////////
//CLevelRecords

CRecords*LoadRecords(const char *pathRoot,const char *name)
{
	CRecords *records=NULL;

	CClass *clss=NULL;
	if (TRUE)
	{
		DWORD c;
		LevelRecordEntry *entries=GetLevelRecordEntries(c);
		for (int i=0;i<c;i++)
		{
			LevelRecordEntry *entry=&entries[i];
			if (StringEqualNoCase(name,entry->name))
			{
				clss=entry->clss;
				break;
			}
		}
		if (!clss)
			return NULL;
	}

	std::string path;
	path=pathRoot;
	path+="\\";
	path+=name;
	path+=".rcs";

	std::ifstream ifs;
	ifs.open(path.c_str(),std::ios_base::in|std::ios_base::binary);
	if (!ifs.is_open())
		return NULL;

	LevelResFileHeader header;
	ifs.read((char *)&header,sizeof(header));

	if (header.type!=Res_Records)
	{
		ifs.close();
		return NULL;
	}

	ifs.seekg(header.off);

	std::vector<BYTE>buf;
	DWORD sz;
	ifs.read((char *)&sz,sizeof(sz));
	buf.resize(sz);
	ifs.read((char *)buf.data(),sz);

	ifs.close();

	CDataPacket dp;
	dp.SetDataBufferPointer(buf.data());

	records=Class_New2(CRecords);
	records->Init(clss);
	records->Load(&dp);

	return records;
}


#define LOAD_RECORDS(name)																														\
{																																											\
	CRecords*var=_LoadRecords(#name+1);																									\
	if (var)																																								\
	{																																										\
		_buf[_nLevelRecords++]=var;																													\
		name=var;																																					\
	}																																										\
}

CRecords *CLevelRecords::_LoadRecords(const char *name)
{
	return LoadRecords(_pathRoot.c_str(),name);
}


void CLevelRecords::_DoLoad()
{
	LOAD_RECORDS(_global);
	LOAD_RECORDS(_units);
	LOAD_RECORDS(_items);
	LOAD_RECORDS(_skills);
	LOAD_RECORDS(_buffs);
	LOAD_RECORDS(_regions);
	LOAD_RECORDS(_flocks);
	LOAD_RECORDS(_itemclasses);
	LOAD_RECORDS(_postures);
	LOAD_RECORDS(_agents);
	LOAD_RECORDS(_gestures);
	LOAD_RECORDS(_npcs);
	LOAD_RECORDS(_maps);
// 	LOAD_RECORDS(_effectsets);
	LOAD_RECORDS(_eos);
	LOAD_RECORDS(_itemseeds);
	LOAD_RECORDS(_magicboards);
	LOAD_RECORDS(_magictiles);
	LOAD_RECORDS(_upgrades);
	LOAD_RECORDS(_resources);
	LOAD_RECORDS(_slatetypes);
	LOAD_RECORDS(_goods);
	//XXXXX: More LevelRecord

	if (FALSE)
	{
		DWORD c;
		RecordID *ids=_eos->GetRecords(c);

		std::string s="(闪电弓)箭2";
		for (int i=0;i<c;i++)
		{
			if (s==_eos->GetName(ids[i]))
			{
				CRecord*rec=_eos->GetRecord(ids[i]);
				int v=0;
				v++;
			}
		}
	}

	//将_global里的第一个record记录下来
	if (_global)
	{
		DWORD c;
		RecordID *ids=_global->GetRecords(c);

		if (c>0)
			_recGlobal=(LevelRecordGlobal*)_global->GetSafeRecord(ids[0]);
	}

	//记录Postures
	if(_postures)
	{
		memset(_recPostures,0,sizeof(_recPostures));

		DWORD c;
		RecordID *ids=_postures->GetRecords(c);
		
		for (int i=0;i<c;i++)
		{
			LevelRecordPosture *rec=GetPosture(ids[i]);
			if (rec->tp<LevelPosture_Max)
				_recPostures[rec->tp]=rec;
		}
	}

	//记录InitialUpgrade
	if (_upgrades)
	{
		memset(_recAbilityInitialUpgrades,0,sizeof(_recAbilityInitialUpgrades));

		DWORD c;
		RecordID *ids=_upgrades->GetRecords(c);

		for (int i=0;i<c;i++)
		{
			LevelRecordUpgrade *rec=GetUpgrade(ids[i]);
			if (rec->upgrade)
			{
				CLevelUpgrade::Type tp=rec->upgrade->GetUpgradeType();
				if (tp==CLevelUpgrade::Ability)
				{
					CLevelAbilityUpgrade *upgrade=(CLevelAbilityUpgrade *)rec->upgrade;
					LevelAbilityType tp=upgrade->GetAbilityType();
					if (tp<LevelAbilityType_Max)
					{
						if (upgrade->GetChannel()==CLevelAbilityUpgrade::Channel_Initial)
							_recAbilityInitialUpgrades[tp]=rec;
						if (upgrade->GetChannel()==CLevelAbilityUpgrade::Channel_LevelUp)
							_recAbilityLevelUpUpgrades[tp]=rec;
					}
				}
			}
		}
	}

	if (_slatetypes)
	{
		memset(_recSlateTypes,0,sizeof(_recSlateTypes));
		DWORD c;
		RecordID *ids=_slatetypes->GetRecords(c);

		for (int i=0;i<c;i++)
		{
			LevelRecordSlateType *rec=GetSlateType(ids[i]);
			if (rec->family==LevelSlateFamily_A)
				_recSlateTypes[rec->TypeA]=rec;
			if (rec->family==LevelSlateFamily_B)
				_recSlateTypes[rec->TypeB]=rec;
		}
	}

}


void CLevelRecords::Init(const char *pathRoot)
{
	_pathRoot=pathRoot;

	_DoLoad();
}

void CLevelRecords::Clear()
{
	for (int i=0;i<_nLevelRecords;i++)
	{
		if (_buf[i])
		{
			_buf[i]->Clear();
			Safe_Class_Delete(_buf[i]);
		}
	}

	Zero();
}

CRecords *CLevelRecords::GetRecords(DWORD idx)
{
	if (idx>=_nLevelRecords)
		return NULL;
	return _buf[idx];
}


CRecord *CLevelRecords::_GetRecord(CRecords *records,RecordID id)
{
	if (id==0)
		return NULL;
	CRecord *record=NULL;
	if ((id>>16)==0)
		record=records->GetRecord(id);
	else
		record=records->GetSafeRecord(id);
	if (record)
		return record;

	LOG_DUMP_2P("CLevelRecords",Log_Error,"无法在Records(\"%s\")里找到指定的Record(%u)!",
				records->GetRecordClass()->GetName(),(DWORD)id);
	return records->GetEmptyRecord();
}

LevelRecordGlobal*CLevelRecords::GetGlobal()
{
	return _recGlobal;
}


LevelRecordUnit*CLevelRecords::GetUnit(RecordID id)
{
	if (!_units)
		return NULL;
	return (LevelRecordUnit*)_GetRecord(_units,id);
}

LevelRecordItem*CLevelRecords::GetItem(RecordID id)
{
	if (!_items)
		return NULL;
	return (LevelRecordItem*)_GetRecord(_items,id);
}

LevelRecordSkill*CLevelRecords::GetSkill(RecordID id)
{
	if (!_skills)
		return NULL;
	return (LevelRecordSkill*)_GetRecord(_skills,id);
}

LevelRecordBuff*CLevelRecords::GetBuff(RecordID id)
{
	if (!_buffs)
		return NULL;
	return (LevelRecordBuff*)_GetRecord(_buffs,id);
}

LevelRecordRegion*CLevelRecords::GetRegion(RecordID id)
{
	if (!_regions)
		return NULL;
	return (LevelRecordRegion*)_GetRecord(_regions,id);
}


LevelRecordFlock*CLevelRecords::GetFlock(RecordID id)
{
	if (!_flocks)
		return NULL;
	return (LevelRecordFlock*)_GetRecord(_flocks,id);
}

LevelRecordItemClass*CLevelRecords::GetItemClass(RecordID id)
{
	if (!_itemclasses)
		return NULL;
	return (LevelRecordItemClass*)_GetRecord(_itemclasses,id);
}

LevelRecordItemClass *CLevelRecords::GetItemClassOfItem(RecordID id)
{
	LevelRecordItem *recItem=GetItem(id);
	if (!recItem)
		return NULL;
	return GetItemClass(recItem->clss);
}

LevelRecordPosture*CLevelRecords::GetPostureOfItem(RecordID id)
{
	LevelRecordItemClass *recClass=GetItemClassOfItem(id);
	if (!recClass)
		return NULL;

	if (recClass->posture==RecordID_Invalid)
		return NULL;

	return GetPosture(recClass->posture);
}



LevelRecordPosture*CLevelRecords::GetPosture(RecordID id)
{
	if (!_postures)
		return NULL;
	return (LevelRecordPosture*)_GetRecord(_postures,id);
}

LevelRecordPosture*CLevelRecords::GetPostureByType(LevelPostureType tpPosture)
{
	if (((DWORD)tpPosture)<sizeof(_recPostures))
		return _recPostures[tpPosture];
	return NULL;
}


LevelRecordAgent*CLevelRecords::GetAgent(RecordID id)
{
	if (!_agents)
		return NULL;
	return (LevelRecordAgent*)_GetRecord(_agents,id);
}

LevelRecordGesture*CLevelRecords::GetGesture(RecordID id)
{
	if (!_gestures)
		return NULL;
	return (LevelRecordGesture*)_GetRecord(_gestures,id);
}

LevelRecordNPC*CLevelRecords::GetNPC(RecordID id)
{
	if (!_npcs)
		return NULL;
	return (LevelRecordNPC*)_GetRecord(_npcs,id);
}

LevelRecordMap*CLevelRecords::GetMap(RecordID id)
{
	if (!_maps)
		return NULL;
	return (LevelRecordMap*)_GetRecord(_maps,id);
}

LevelRecordEffectSet*CLevelRecords::GetEffectSet(RecordID id)
{
	if (!_effectsets)
		return NULL;
	return (LevelRecordEffectSet*)_GetRecord(_effectsets,id);
}

LevelRecordEo*CLevelRecords::GetEo(RecordID id)
{
	if (!_eos)
		return NULL;
	return (LevelRecordEo*)_GetRecord(_eos,id);
}

LevelRecordItemSeed*CLevelRecords::GetItemSeed(RecordID id)
{
	if (!_itemseeds)
		return NULL;
	return (LevelRecordItemSeed*)_GetRecord(_itemseeds,id);
}

LevelRecordMagicBoard*CLevelRecords::GetMagicBoard(RecordID id)
{
	if (!_magicboards)
		return NULL;
	return (LevelRecordMagicBoard*)_GetRecord(_magicboards,id);
}

LevelRecordMagicTile*CLevelRecords::GetMagicTile(RecordID id)
{
	if (!_magictiles)
		return NULL;
	return (LevelRecordMagicTile*)_GetRecord(_magictiles,id);
}

LevelRecordUpgrade*CLevelRecords::GetUpgrade(RecordID id)
{
	if (!_upgrades)
		return NULL;
	return (LevelRecordUpgrade*)_GetRecord(_upgrades,id);
}

LevelRecordRes*CLevelRecords::GetResource(RecordID id)
{
	if (!_resources)
		return NULL;
	return (LevelRecordRes*)_GetRecord(_resources,id);
}

LevelRecordSlateType*CLevelRecords::GetSlateType(RecordID id)
{
	if (!_slatetypes)
		return NULL;
	return (LevelRecordSlateType*)_GetRecord(_slatetypes,id);
}

LevelRecordGoods*CLevelRecords::GetGoods(RecordID id)
{
	if (!_goods)
		return NULL;
	return (LevelRecordGoods*)_GetRecord(_goods,id);
}


//XXXXX: More LevelRecord



LevelRecordUpgrade *CLevelRecords::GetInitialUpgrade(LevelAbilityType tp)
{
	if (tp<LevelAbilityType_Max)
		return _recAbilityInitialUpgrades[(int)tp];

	return NULL;
}

LevelRecordUpgrade *CLevelRecords::GetLevelUpUpgrade(LevelAbilityType tp)
{
	if (tp<LevelAbilityType_Max)
		return _recAbilityLevelUpUpgrades[(int)tp];

	return NULL;
}

LevelRecordSlateType *CLevelRecords::GetSlateType(LevelSlateType tp)
{
	if (tp<LevelSlateType_Max)
		return _recSlateTypes[(int)tp];
	return NULL;
}

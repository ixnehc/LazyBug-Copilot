#pragma once

#include "class/class.h"

#include "records/records.h"

#include "LevelSlateDefines.h"

class CClass;
struct LevelRecordEntry
{
	LevelRecordEntry(const char *s,CClass *c)
	{
		name=s;
		clss=c;
	}
	const char *name;
	CClass *clss;
};

extern LevelRecordEntry *GetLevelRecordEntries(DWORD &c);

struct LevelRecordGlobal;
struct LevelRecordUnit;
struct LevelRecordItem;
struct LevelRecordSkill;
struct LevelRecordBuff;
struct LevelRecordRegion;
struct LevelRecordFlock;
struct LevelRecordItemClass;
struct LevelRecordPosture;
struct LevelRecordAI;
struct LevelRecordAgent;
struct LevelRecordGesture;
struct LevelRecordNPC;
struct LevelRecordMap;
struct LevelRecordEffectSet;
struct LevelRecordEo;
struct LevelRecordItemSeed;
struct LevelRecordMagicBoard;
struct LevelRecordMagicTile;
struct LevelRecordUpgrade;
struct LevelRecordRes;
struct LevelRecordSlateType;
struct LevelRecordGoods;
//XXXXX: More LevelRecord


class CLevelRecords
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CLevelRecords);
	CLevelRecords()
	{
		Zero();
	}
	~CLevelRecords()
	{
		Clear();
	}
	void Zero()
	{
		memset(_buf,0,sizeof(_buf));
		_nLevelRecords=0;

		_recGlobal=NULL;
		memset(_recPostures,0,sizeof(_recPostures));
		memset(_recAbilityInitialUpgrades,0,sizeof(_recAbilityInitialUpgrades));
		memset(_recAbilityLevelUpUpgrades,0,sizeof(_recAbilityLevelUpUpgrades));
		memset(_recSlateTypes,0,sizeof(_recSlateTypes));

		_global=NULL;
		_units=NULL;
		_items=NULL;
		_skills=NULL;
		_buffs=NULL;
		_regions=NULL;
		_flocks=NULL;
		_itemclasses=NULL;
		_postures=NULL;
		_agents=NULL;
		_gestures=NULL;
		_npcs=NULL;
		_maps=NULL;
		_effectsets=NULL;
		_eos=NULL;
		_itemseeds=NULL;
		_magicboards=NULL;
		_magictiles=NULL;
		_upgrades=NULL;
		_resources=NULL;
		_slatetypes=NULL;
		_goods=NULL;
		//XXXXX: More LevelRecord
	}

	virtual void Clear();
	void Init(const char *pathRoot);

	BOOL IsEmpty()	{		return _nLevelRecords<=0;	}

	DWORD GetRecordsCount()	{		return _nLevelRecords;	}
	CRecords *GetRecords(DWORD idx);

	LevelRecordGlobal*GetGlobal();
	LevelRecordUnit*GetUnit(RecordID id);
	LevelRecordItem*GetItem(RecordID id);
	LevelRecordSkill*GetSkill(RecordID id);
	LevelRecordBuff*GetBuff(RecordID id);
	LevelRecordRegion*GetRegion(RecordID id);
	LevelRecordFlock*GetFlock(RecordID id);
	LevelRecordItemClass*GetItemClass(RecordID id);
	LevelRecordPosture*GetPosture(RecordID id);
	LevelRecordPosture*GetPostureByType(LevelPostureType tpPosture);
	LevelRecordAgent*GetAgent(RecordID id);
	LevelRecordGesture*GetGesture(RecordID id);
	LevelRecordNPC*GetNPC(RecordID id);
	LevelRecordMap*GetMap(RecordID id);
	LevelRecordEffectSet*GetEffectSet(RecordID id);
	LevelRecordEo*GetEo(RecordID id);
	LevelRecordItemSeed*GetItemSeed(RecordID id);
	LevelRecordMagicBoard*GetMagicBoard(RecordID id);
	LevelRecordMagicTile*GetMagicTile(RecordID id);
	LevelRecordUpgrade*GetUpgrade(RecordID id);
	LevelRecordRes*GetResource(RecordID id);
	LevelRecordSlateType*GetSlateType(RecordID id);
	LevelRecordGoods*GetGoods(RecordID id);
	//XXXXX: More LevelRecord

	LevelRecordItemClass *GetItemClassOfItem(RecordID id);//id是一个LevelRecordItem的id
	LevelRecordPosture*GetPostureOfItem(RecordID id);//id是一个LevelRecordItem的id
	LevelRecordUpgrade *GetInitialUpgrade(LevelAbilityType tp);
	LevelRecordUpgrade *GetLevelUpUpgrade(LevelAbilityType tp);
	LevelRecordSlateType *GetSlateType(LevelSlateType tp);

	CRecords *GetRecords_Map()	{		return _maps;	}
	CRecords *GetRecords_Item()	{		return _items;	}
	CRecords *GetRecords_Seed()	{		return _itemseeds;	}
	CRecords *GetRecords_Upgrade()	{		return _upgrades;	}
	CRecords *GetRecords_Skill()	{		return _skills;	}
	CRecords *GetRecords_Resource()	{		return _resources;	}
	CRecords *GetRecords_Goods()	{		return _goods;	}

protected:

	void _DoLoad();

	virtual CRecords *_LoadRecords(const char *name);

	CRecord *_GetRecord(CRecords*records,RecordID id);

	std::string _pathRoot;


	CRecords *_global;
	CRecords*_units;
	CRecords*_items;
	CRecords*_skills;
	CRecords*_buffs;
	CRecords*_regions;
	CRecords*_flocks;
	CRecords*_itemclasses;
	CRecords*_postures;
	CRecords*_agents;
	CRecords *_gestures;
	CRecords *_npcs;
	CRecords *_maps;
	CRecords *_effectsets;
	CRecords *_eos;
	CRecords *_itemseeds;
	CRecords *_magicboards;
	CRecords *_magictiles;
	CRecords *_upgrades;
	CRecords *_resources;
	CRecords *_slatetypes;
	CRecords *_goods;
	//XXXXX: More LevelRecord

	LevelRecordGlobal*_recGlobal;
	LevelRecordPosture *_recPostures[LevelPosture_Max];
	LevelRecordUpgrade *_recAbilityInitialUpgrades[LevelAbilityType_Max];
	LevelRecordUpgrade *_recAbilityLevelUpUpgrades[LevelAbilityType_Max];
	LevelRecordSlateType *_recSlateTypes[LevelSlateType_Max];


	CRecords*_buf[64];
	DWORD _nLevelRecords;
};


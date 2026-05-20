#pragma once

#include "LevelObjSrc.h"
#include "LevelChancer.h"

#include "LevelSlatesBasis.h"

#include "circum/CircumSites.h"

#include <unordered_map>

struct LevelLoc
{
	LevelPos pos;
	LevelFace face;
};

struct LevelRoute
{
	DEFINE_CLASS(LevelRoute);
	std::vector<LevelPos> nodes;
};

struct LevelTeleportSite
{
	DEFINE_CLASS(LevelTeleportSite);
	LevelPos pos;//主角的落脚位置

	CCircumSites csites;
};




//分布信息
struct LosNPCLoc;
struct NPCDistribute
{
	RecordID idNPC;
	struct Entry
	{
		LosNPCLoc *loc;
		RecordID idMap;
	};
	std::vector<Entry> entries;
};

struct NPCBasis
{
	void Clear()
	{
		distribs.clear();
	}
	std::vector<NPCDistribute> distribs;//NPC的分布
};


class CLevelBasis
{
public:
	DEFINE_CLASS(CLevelBasis);

	CLevelBasis()
	{
		Zero();
	}
	void Zero()
	{
		_srces=NULL;
	}
	struct Src
	{
		CLevelObjSrc *src;
		CLevelObjParam *param;
	};

	void Build(CLevelSources *srces,const char *pathSrces,CLevelRecords *records);
	void Clear();

	CCircumSites &GetDefCircumSites()	{		return _csitesDef;	}

	LevelPos FindLocPos(StringID idCheckPoint);
	LevelLoc *FindLoc(StringID idCheckPoint);
	LevelRoute *FindRoute(StringID nmRoute);
	LevelTeleportSite*FindTeleportSite(StringID nm);

	CLevelSlatesBasis &GetSlatesBasis()	{		return _basisSlates;	}


protected:

	CLevelSources *_srces;
	std::vector<Src> _buf;

	std::unordered_map<StringID,LevelLoc>_locs;//check points
	std::unordered_map<StringID,LevelRoute*>_routes;//routes
	std::unordered_map<StringID,LevelTeleportSite*>_tpsites;//teleport sites

	CLevelChanceData _chancedata;

	CCircumSites _csitesDef;//缺省的csites

	CLevelSlatesBasis _basisSlates;


	friend class CLevel;
};


class CWorldBasis
{
public:
	DEFINE_CLASS(CWorldBasis);
	void Build(CLevelSources **srces,RecordID *idMaps,DWORD nSources,CLevelRecords *records);
	void Clear();
	NPCBasis *GetNPCBasis()	{		return &_npcs;	}

protected:
	NPCBasis _npcs;

};
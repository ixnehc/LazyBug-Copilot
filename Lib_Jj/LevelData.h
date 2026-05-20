#pragma once

#include "nav/navdata.h"
#include "gamedata/GameTileMap.h"
#include "gamedata/GameTrisMap.h"

#include "LevelObjSrc.h"
#include "LevelBasis.h"

#include "LevelSlateDefinesB.h"

#include "gamedata/GameRgnGrid.h"

#include <unordered_map>


class CLevelData
{
public:
	DEFINE_CLASS(CLevelData);
	CLevelData()
	{
		_srces=NULL;
		_basis=NULL;
		_grids=NULL;
	}

	void Clear();

	BOOL Load(const char *pathNavData,const char *pathSrces,const char *pathGrids,const char *pathGti,const char *pathGtr,CLevelRecords *records);

	void ReloadSources(CLevelRecords *records);
	void ReloadGrids();

	navData *GetNavData()	{		return &_ndata;	}
	GameTileMap*GetGtm()	{		return &_gtm;	}
	CGameTrisMap*GetGtr()	{		return &_gtr;	}
	CLevelSources*GetSources()	{		return _srces;	}
	CLevelBasis*GetBasis()	{		return _basis;	}
	CGameRgnGrids*GetRgnGrids()	{		return _grids;	}

	const char *GetNavDataPath()	{		return _pathNavData.c_str();	}
	const char *GetSourcesPath()	{		return _pathSrces.c_str();	}
	const char *GetRgnGridsPath()	{		return _pathGrids.c_str();	}

protected:
	std::string _pathNavData;
	std::string _pathSrces;
	std::string _pathGrids;
	std::string _pathGtm;
	std::string _pathGtr;

	BOOL _LoadGtm(const char *pathGtm);
	BOOL _LoadGtr(const char *pathGtr);

	CLevelSources *_LoadSources(const char *pathSources);
	CGameRgnGrids*_LoadGrids(const char *pathGrids);

	navData _ndata;
	GameTileMap _gtm;
	CGameTrisMap _gtr;
	CLevelSources *_srces;
	CLevelBasis *_basis;//注意_basis依赖于_srces内容的存在,所以两者要同时删除
	std::vector<CLevelSources *>_srcesOld;
	std::vector<CLevelBasis*>_basisOld;
	CGameRgnGrids *_grids;
	std::vector<CGameRgnGrids *>_gridsOld;
};


class CWorldData
{
public:
	CWorldData()
	{
		_basis=NULL;
	}
	void Clear();
	void LoadDatabaseSlatesB(const char *pathRoot);
	BOOL AddLevel(RecordID idMap,const char *pathRoot,CLevelRecords *records);
	void BuildBasis(CLevelRecords *records);
	void ReloadSources(CLevelRecords *records);
	void ReloadGrids();
	CLevelData *FindLevel(RecordID idMap);
	SlatesBData *PickUpSlatesB(BOOL bEasy);
	CWorldBasis *GetBasis()	{		return _basis;	}
protected:
	struct DatabaseSlatesB
	{
		void Clear()
		{
			easies.clear();
			hards.clear();
		}

		std::deque<SlatesBData> easies;
		std::deque<SlatesBData> hards;
	};
	void _LoadSlatesB(const char *pathFolder,std::deque<SlatesBData>&buf);
	void _LoadSlatesB(const char *path,SlatesBData&data);
	DatabaseSlatesB _dbSlatesB;

	std::unordered_map<RecordID,CLevelData*> _datas;

	std::vector<CWorldBasis *>_basisOld;
	CWorldBasis *_basis;
};
#pragma once

#include "LevelDefines.h"


class CLevelNPC;
class CLevel;
class CJjWorld;

struct LPSNpcData;
struct LevelPlayerStates;

//某个Player的在世界中的所有NPC
class CNPCPendings
{
public:
	CNPCPendings()
	{
		Zero();
	}


	struct Pending
	{
		Pending()
		{
			idNPC=RecordID_Invalid;
			dataNPC=NULL;
		}

		RecordID idNPC;
		LPSNpcData *dataNPC;
		LevelPos pos;
	};

	struct Pendings
	{
		std::vector<Pending> buf;//所有要在这个地图中创建的的NPC
	};

	void Zero()
	{
		_idPlayer=LevelPlayerID_Invalid;
		_world=NULL;
		_lps=NULL;
	}

	CJjWorld *GetWorld()	{		return _world;	}
	LevelPlayerID GetPlayerID()	{		return _idPlayer;	}
	LevelPlayerStates *GetLPS()	{		return _lps;	}

	void Init(CJjWorld *world,LevelPlayerID idPlayer);
	void Clear();

	Pendings *GetMapPendings(RecordID idMap)
	{
		return _GetMapPendings(idMap);
	}
	void EraseMapPending(RecordID idMap)
	{
		_EraseMapPendings(idMap);
	}

	Pendings*GetRtnuPendings()	{		return &_rtnus;	}
	void ClearRtnuPendings();



protected:

	LevelPlayerID _idPlayer;
	CJjWorld *_world;
	LevelPlayerStates *_lps;

	//等待创建的NPC
	Pendings *_ObtainMapPendings(RecordID idMap);
	Pendings *_GetMapPendings(RecordID idMap);
	void _EraseMapPendings(RecordID idMap);
	std::unordered_map<RecordID,Pendings> _mps;//在各张地图上等待创建的NPC
	Pendings _rtnus;


};


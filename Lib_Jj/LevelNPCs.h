#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "LevelDefines.h"

#include "LevelObj.h"
#include "NPCPendings.h"


class CLevelNPCs;
struct LevelRecordNPC;
struct LPSNpcData;
class CLoUnit;
class CLevelNPC
{
public:
	DEFINE_CLASS(CLevelNPC);
	CLevelNPC()
	{
		Zero();
	}

	void Zero()
	{
		_npcs=NULL;
		_rec=NULL;
		_loUnit=NULL;
		_state=NPCState_Default;
		_bInUpdate=0;
		_bStateDirty=0;
	}

	void Init(CLevelNPCs *npcs,LevelRecordNPC *rec)
	{
		_npcs=npcs;
		_rec=rec;
	}
	LevelRecordNPC *GetRec()	{		return _rec;	}
	RecordID GetRecID();

	BOOL Create(LevelPos &pos,LPSNpcData *dataNPC);
	BOOL CreateTeleport(CLevel *level,LevelPos &pos,CLevelNPC *npcOrg);

	void Destroy();

	void Update();

	void AddCoSkillCharge(LevelRecordSkill *recSkill,LevelSkillGrade grd,LevelSkillTarget &target);

	void SwitchState(NPCState stateNew);

	CLevelNPCs *GetNPCs()	{		return _npcs;	}
	LPSNpcData *GetNpcData();

protected:

	CLevelNPCs *_npcs;
	LevelRecordNPC *_rec;

	NPCState _state;
	BYTE _bStateDirty:1;
	BYTE _bInUpdate:1;
	CLoUnit *_loUnit;


};

class CLevelNPCs
{
public:
	DEFINE_CLASS(CLevelNPCs)
	CLevelNPCs()
	{
		Zero();
	}
	void Zero()
	{
		_level=NULL;
		_idPlayer=LevelPlayerID_Invalid;
		_lps=NULL;
	}
	BOOL Create(CLevel *level,LevelPlayerID idPlayer,CNPCPendings::Pendings *pendings);
	BOOL CreateTeleport(CLevel *level,LevelPlayerID idPlayer,CLevelNPCs *npcsOrg);
	void Destroy(RecordID idNPC);
	void Update();

	CLevelNPC *FetchNPC(RecordID idNPC);
	void AddNPC(CLevelNPC *npc);

	void AddCoSkillCharge(LevelRecordSkill *recSkill,LevelSkillGrade grd,LevelSkillTarget &target);

	CLevel *GetLevel()	{		return _level;	}
	LevelPlayerID GetPlayerID()	{		return _idPlayer;	}
	LevelPlayerStates *GetLPS()	{		return _lps;	}

protected:
	CLevel *_level;
	LevelPlayerID _idPlayer;
	LevelPlayerStates *_lps;
	std::vector<CLevelNPC *>_npcs;

};
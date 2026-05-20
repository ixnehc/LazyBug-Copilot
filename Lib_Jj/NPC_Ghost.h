#pragma once


#include "LevelDefines.h"

#include "records/recordsdefine.h"

#include "LevelObj.h"

#include "LevelNPC.h"


struct NPCParam_Ghost:public LevelNPCParam
{
	DEFINE_NPCPARAM_CLASS(NPCParam_Ghost);

	virtual BOOL CreateNPCs(CLevelNPCs *npcs,LevelRecordNPC *rec,NPCDistribute &distribute)
	{
		extern BOOL CreateNPCs(NPCParam_Ghost*param,CLevelNPCs *npcs,LevelRecordNPC *rec,NPCDistribute &distribute);
		return CreateNPCs(this,npcs,rec,distribute);
	}

	BEGIN_GOBJ_PURE(NPCParam_Ghost,1);

		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位",GVT_U,GSem(GSem_RecordID,"units"),"哪个单位");

		GELEM_VAR_INIT(StringID,nmBG,StringID_Invalid);
			GELEM_EDITVAR("行为图",GVT_U,GSem(GSem_StringID,"行为图名称"),"使用哪个行为图");

		GELEM_VAR_INIT(int,grdBase,1);
			GELEM_EDITVAR("等级",GVT_S,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"单位的等级");


	END_GOBJ();

	RecordID idUnit;
	StringID nmBG;
	int grdBase;

};


class CLoUnit;
class CLevelBehavior;
class CBehaviorPersist;


//鬼魂型:只对某个Player可见,对每个Player有各自的BG,位置不固定,可以做各种事情
class NPC_Ghost:public CLevelNPC
{
public:
	DEFINE_CLASS(NPC_Ghost);

	struct Entry
	{
		CLoUnit *loUnit;
		CLevelBehavior *bhv;
		CBehaviorPersist *persist;
	};

	NPC_Ghost()
	{
		Zero();
	}

	void Zero()
	{
		memset(_entries,0,sizeof(_entries));
	}


protected:

	virtual void _OnCreate();
	virtual void _OnDestroy();

	virtual void _OnAddPlayer(LevelPlayerID idPlayer);
	virtual void _OnRemovePlayer(LevelPlayerID idPlayer);

	virtual void _OnUpdate(AnimTick t);

	void _SaveEntry(Entry *entry,LevelPlayerStates *lps);
	void _LoadEntry(Entry *entry,LevelPlayerStates *lps);

	void _ClearEntry(Entry *entry);

	Entry _entries[LEVEL_MAX_PLAYER];

};

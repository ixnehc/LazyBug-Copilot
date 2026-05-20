#pragma once


#include "LevelDefines.h"

#include "records/recordsdefine.h"

#include "LevelObj.h"

#include "LevelNPC.h"


struct NPCParam_Laentia:public LevelNPCParam
{
	DEFINE_NPCPARAM_CLASS(NPCParam_Laentia);

	virtual BOOL CreateNPCs(CLevelNPCs *npcs,LevelRecordNPC *rec,NPCDistribute &distribute)
	{
		extern BOOL CreateNPCs(NPCParam_Laentia*param,CLevelNPCs *npcs,LevelRecordNPC *rec,NPCDistribute &distribute);
		return CreateNPCs(this,npcs,rec,distribute);
	}

	BEGIN_GOBJ_PURE(NPCParam_Laentia,1);

		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位",GVT_U,GSem(GSem_RecordID,"units"),"哪个单位");

		GELEM_VAR_INIT(StringID,nmBG,StringID_Invalid);
			GELEM_EDITVAR("行为图",GVT_U,GSem(GSem_StringID,"行为图名称"),"使用哪个行为图");

	END_GOBJ();

	RecordID idUnit;
	StringID nmBG;

};


class CLoUnit;
class CLevelBehavior;


//兰提亚:对所有Player可见,位置随机出现在某处后,固定,可以跟随某个玩家
class NPC_Laentia:public CLevelNPC
{
public:
	DEFINE_CLASS(NPC_Laentia);

	struct Entry
	{
		CLoUnit *loUnit;
		CLevelBehavior *bhv;
		CLevelBehaviorPersist *persist;
	};

	NPC_Laentia()
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

};

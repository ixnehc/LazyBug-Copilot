#pragma once


#include "LevelDefines.h"

#include "records/recordsdefine.h"

#include "LevelObj.h"

#include "LevelNPC.h"


struct NPCParam_Standard:public LevelNPCParam
{
	DEFINE_NPCPARAM_CLASS(NPCParam_Standard);

	virtual BOOL CreateNPCs(CLevelNPCs *npcs,LevelRecordNPC *rec,NPCDistribute &distribute)
	{
		extern BOOL CreateNPCs(NPCParam_Standard*param,CLevelNPCs *npcs,LevelRecordNPC *rec,NPCDistribute &distribute);
		return CreateNPCs(this,npcs,rec,distribute);
	}


	BEGIN_GOBJ_PURE(NPCParam_Standard,1);

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

//Standard NPC:对所有Player可见,位置固定,对每个Player有各自的BehaviorGraph
class NPC_Standard:public CLevelNPC
{
public:
	DEFINE_CLASS(NPC_Standard);

	NPC_Standard()
	{
		Zero();
	}

	void Zero()
	{
		_loUnit=NULL;
		memset(_bhv,0,sizeof(_bhv));
	}


protected:

	virtual void _OnCreate();
	virtual void _OnDestroy();

	virtual void _OnAddPlayer(LevelPlayerID idPlayer);
	virtual void _OnRemovePlayer(LevelPlayerID idPlayer);

	virtual void _OnUpdate(AnimTick t);

	CLoUnit *_loUnit;
	CLevelBehavior *_bhv[LEVEL_MAX_PLAYER];

};

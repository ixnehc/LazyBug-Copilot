#pragma once


#include "LevelDefines.h"

#include "records/recordsdefine.h"

#include "LevelObj.h"

#include "LevelNPC.h"


struct NPCParam_Pygmy:public LevelNPCParam
{
	DEFINE_NPCPARAM_CLASS(NPCParam_Pygmy);

	virtual BOOL CreateNPCs(CLevelNPCs *npcs,LevelRecordNPC *rec,NPCDistribute &distribute)
	{
		extern BOOL CreateNPCs(NPCParam_Pygmy*param,CLevelNPCs *npcs,LevelRecordNPC *rec,NPCDistribute &distribute);
		return CreateNPCs(this,npcs,rec,distribute);
	}


	BEGIN_GOBJ_PURE(NPCParam_Pygmy,1);

		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位",GVT_U,GSem(GSem_RecordID,"units"),"哪个单位");

		GELEM_VAR_INIT(StringID,nmBG,StringID_Invalid);
			GELEM_EDITVAR("NPC时的AI",GVT_U,GSem(GSem_StringID,"行为图名称"),"NPC时AI的行为图");


	END_GOBJ();

	RecordID idUnit;
	StringID nmBG;
};


class CLoUnit;
class CLevelBehavior;


//侏儒型:对所有Player可见,只有一个BG,为所有Player竞争,没有persist
class NPC_Pygmy:public CLevelNPC
{
public:
	DEFINE_CLASS(NPC_Pygmy);

	struct Entry
	{
	};

	NPC_Pygmy()
	{
		Zero();
	}

	void Zero()
	{
		_loUnit=NULL;
		_bhv=NULL;
	}

	virtual BOOL IsOwningLo(CLevelObj * lo)	
	{		
		if (_loUnit==lo)	
			return TRUE;
		return FALSE;
	}

	virtual BOOL SwitchRetinue(LevelPlayerID idPlayer,CLevelObj *lo);

protected:

	virtual void _OnCreate();
	virtual void _OnDestroy();

	virtual void _OnAddPlayer(LevelPlayerID idPlayer);
	virtual void _OnRemovePlayer(LevelPlayerID idPlayer);

	virtual void _OnUpdate(AnimTick t);

	void _CreateLo();

	void _SwitchRetinue(LevelPlayerID idPlayer);

	void _PostProcessBehavior();

	void _ClearBehavior();

	CLoUnit *_loUnit;
	CLevelBehavior *_bhv;

};

#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"

#define CLASSUID_SPStone 5


struct LopSPStone:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopSPStone,CLASSUID_SPStone);

	BEGIN_GOBJ_PURE(LopSPStone,1);

		GELEM_VAR_INIT(int,amount,100);
			GELEM_EDITVAR("灵气点数",GVT_S,GSem_Interger,"灵气石蓄积的灵气点数");

	END_GOBJ();

	int amount;
};

struct LosSPStone:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosSPStone,CLASSUID_SPStone);

	BEGIN_GOBJ_PURE(LosSPStone,1);

		GELEM_AGENTRECORD();

		GELEM_VAR_INIT(RecordID,Skill,RecordID_Invalid);
			GELEM_EDITVAR("施放技能",GVT_U,GSem(GSem_RecordID,"skills"),"触发后施放的技能");
		GELEM_STRING_INIT(SoulEffect,"");
			GELEM_EDITVAR("灵气效果",GVT_String,GSem_ProtoPath,"灵气效果");
	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

	RecordID Skill;
	std::string SoulEffect;

};


class CLoSPStone:public CLoAgent
{
public:
	CLoSPStone()
	{
		_bGathered=FALSE;
	}
	DEFINE_LEVELOBJ_CLASS(CLoSPStone,CLASSUID_SPStone);

	virtual const char *GetShowName()	{		return "灵气石";	}

	virtual BOOL OnActivate();

	virtual BOOL IsServerOnly()	{		return FALSE;	}

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	int GetSPAmount();
	BOOL IsGathered()	{		return _bGathered;	}
	void SetGathered()	{		_bGathered=TRUE;	}

	virtual void Invoke(CLevelObj *loFrom);

protected:
	BOOL _bGathered;//是否已被采集了



};

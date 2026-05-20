#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"

#define CLASSUID_LifeFlies 6


struct LopLifeFlies:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopLifeFlies,CLASSUID_LifeFlies);

	BEGIN_GOBJ_PURE(LopLifeFlies,1);

		GELEM_VAR_INIT(int,amnt,100);
			GELEM_EDITVAR("生命萤火个数",GVT_S,GSem_Interger,"生命萤火的个数");

	END_GOBJ();


	int amnt;
};

struct LosLifeFlies:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosLifeFlies,CLASSUID_LifeFlies);

	BEGIN_GOBJ_PURE(LosLifeFlies,1);

		GELEM_AGENTRECORD();
	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

};


class CLoLifeFlies:public CLoAgent
{
public:
	CLoLifeFlies()
	{
		_amntSleep=_amntFly=0;
	}
	DEFINE_LEVELOBJ_CLASS(CLoLifeFlies,CLASSUID_LifeFlies);

	virtual const char *GetShowName()	{		return "生命虫";	}

	virtual BOOL OnActivate();

	virtual BOOL IsServerOnly()	{		return FALSE;	}

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);


protected:
	DWORD _amntSleep;
	DWORD _amntFly;



};

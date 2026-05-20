#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"

#define CLASSUID_Teleporter 15


struct LopTeleporter:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopTeleporter,CLASSUID_Teleporter);

	BEGIN_GOBJ_PURE(LopTeleporter,1);

		GELEM_VAR_INIT(RecordID,idMap,RecordID_Invalid);
			GELEM_EDITVAR("地图名称",GVT_U,GSem(GSem_RecordID,"maps"),"要传送到哪张地图上");

		GELEM_VAR_INIT( StringID,nmSite,StringID_Invalid);	
			GELEM_EDITVAR( "位置名称", GVT_U, GSem(GSem_StringID,"地图传送点名称"), "传送到哪个传送点上" );
	END_GOBJ();

	RecordID idMap;
	StringID nmSite;

};

struct LosTeleporter:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosTeleporter,CLASSUID_Teleporter);

	BEGIN_GOBJ_PURE(LosTeleporter,1);

		GELEM_AGENTRECORD();

	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

};


class CLoTeleporter:public CLoAgent
{
public:
	CLoTeleporter()
	{
	}
	DEFINE_LEVELOBJ_CLASS(CLoTeleporter,CLASSUID_Teleporter);

	virtual const char *GetShowName()	{		return "传送器";	}

	virtual BOOL OnActivate();

	virtual BOOL IsServerOnly()	{		return FALSE;	}

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);


	virtual void Invoke(CLevelObj *loFrom);

protected:



};

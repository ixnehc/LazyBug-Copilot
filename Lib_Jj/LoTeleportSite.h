#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "LevelChancer.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"
#include "LevelObjResidable.h"

#include "LevelAttrs.h"

#include "LevelBuff.h"

#define CLASSUID_TeleportSite 14


struct LopTeleportSite:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopTeleportSite,CLASSUID_TeleportSite);

	BEGIN_GOBJ_PURE(LopTeleportSite,1);

		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "传送点名称", GVT_U, GSem(GSem_StringID,"地图传送点名称"), "地图穿送点的名称" );

		GELEM_VARVECTOR(i_math::matrix43f,ring0)
			GELEM_EDITVAR("站立的位点(第1圈)",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"站立的位点");
		GELEM_VARVECTOR(i_math::matrix43f,ring1)
			GELEM_EDITVAR("站立的位点(第2圈)",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"站立的位点");
		GELEM_VARVECTOR(i_math::matrix43f,ring2)
			GELEM_EDITVAR("站立的位点(第3圈)",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"站立的位点");
		GELEM_VARVECTOR(i_math::matrix43f,ring3)
			GELEM_EDITVAR("站立的位点(第4圈)",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"站立的位点");


	END_GOBJ();

	StringID nm;
	std::vector<i_math::matrix43f> ring0;//站立点
	std::vector<i_math::matrix43f> ring1;//站立点
	std::vector<i_math::matrix43f> ring2;//站立点
	std::vector<i_math::matrix43f> ring3;//站立点

	virtual StringID GetUniqueName()	{		return nm;	}

};

struct LosTeleportSite:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosTeleportSite,CLASSUID_TeleportSite);

	BEGIN_GOBJ_PURE(LosTeleportSite,1);

		GELEM_AGENTRECORD();


	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

};


class CLoTeleportSite:public CLoAgent
{
public:
	CLoTeleportSite()
	{
		_occupies=0;
		_occupiesLast=0;
	}
	DEFINE_LEVELOBJ_CLASS(CLoTeleportSite,CLASSUID_TeleportSite);

	virtual const char *GetShowName()	{		return "传送点";	}

	virtual void PostCreate();
	virtual void OnDestroy();

	virtual BOOL OnActivate();

	virtual void Update();

	virtual BOOL IsServerOnly()	{		return TRUE;	}

	void HandleHook(LevelHook &hk);

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnPostWriteSync()	{		_occupiesLast=_occupies;	}

protected:
	BOOL _LoadOccupy(LevelPlayerID idPlayer);
	void _SaveOccupy(LevelPlayerID idPlayer);

	LevelPlayerMask _occupies;//哪些player已经占有了这个Teleport Site
	LevelPlayerMask _occupiesLast;//同步的Cache


};

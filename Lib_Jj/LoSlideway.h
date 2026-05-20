#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"

#define CLASSUID_Slideway 63


struct LopSlideway:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopSlideway,CLASSUID_Slideway);

	BEGIN_GOBJ_PURE(LopSlideway,1);


	END_GOBJ();

};

struct LosSlideway:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosSlideway,CLASSUID_Slideway);

	BEGIN_GOBJ_PURE(LosSlideway,1);

		GELEM_AGENTRECORD();

		GELEM_VARVECTOR(i_math::vector3df,path)
			GELEM_EDITVAR("路点",GVT_Fx3,GSem(GSem_Unknown,"MatSetLS"),"路点");
		GELEM_VAR_INIT(float,speedRewind,3.0f);
			GELEM_EDITVAR("倒回速度",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"倒回速度");
		GELEM_VAR_INIT(float,radius,0.1f);
			GELEM_EDITVAR("半径",GVT_F,GSem(GSem_Float,"0.01,10.0,0.01"),"半径");

		GELEM_VAR_INIT(int,strRequired,1);
			GELEM_EDITVAR("力量需求",GVT_S,GSem_Interger,"力量需求");
		GELEM_VAR_INIT(float,spDrain,5.0f);
			GELEM_EDITVAR("sp消耗速度",GVT_F,GSem(GSem_Float,"0.0,100.0,0.01"),"sp消耗速度");

	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}
	std::vector<i_math::vector3df> path;
	float speedRewind;
	float radius;

	int strRequired;
	float spDrain;

};


class CLoSlideway:public CLoAgent
{
public:
	CLoSlideway()
	{
		_bReached=FALSE;
		_bLoaded=FALSE;
	}
	DEFINE_LEVELOBJ_CLASS(CLoSlideway,CLASSUID_Slideway);

	virtual const char *GetShowName()	{		return "滑槽";	}

	virtual BOOL OnActivate();

	virtual BOOL IsServerOnly()	{		return FALSE;	}

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);


	void NotifyReached();

protected:

	void _LoadPersistS(LevelPlayerID idPlayer);
	void _SavePersistS(LevelPlayerID idPlayer);

	BOOL _bLoaded;

	BOOL _bReached;



};

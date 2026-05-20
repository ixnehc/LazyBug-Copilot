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

#define CLASSUID_WatchTower 9


struct LopWatchTower:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopWatchTower,CLASSUID_WatchTower);

	BEGIN_GOBJ_PURE(LopWatchTower,1);

		GELEM_VAR_INIT(float,rateUnit,0.0f);
			GELEM_EDITVAR("单位出现几率",GVT_F,GSem(GSem_Float,"0,1,0.05"),"岗楼上出现单位的几率");

		GELEM_VAR_INIT(int,grdBase,1);
			GELEM_EDITVAR("单位等级",GVT_S,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"单位的等级");
		GELEM_VAR_INIT(int,grdVary,0);
			GELEM_EDITVAR("单位等级浮动",GVT_S,GSem(GSem_Interger,LevelGradeVary_SemConstraint),"单位的等级的浮动值");

	END_GOBJ();

	float rateUnit;
	int grdBase;
	int grdVary;

};

struct LosWatchTower:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosWatchTower,CLASSUID_WatchTower);

	BEGIN_GOBJ_PURE(LosWatchTower,1);

		GELEM_AGENTRECORD();

		GELEM_VARVECTOR(i_math::spheref,shape)
			GELEM_EDITVAR("外形模拟",GVT_Fx4,GSem(GSem_Unknown,"SphereSetLS"),"外形的模拟");
		GELEM_VARVECTOR(i_math::matrix43f,sitesEntry)
			GELEM_EDITVAR("爬上岗楼的位点",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"爬上岗楼的位点");
		GELEM_VARVECTOR(i_math::matrix43f,sitesExit)
			GELEM_EDITVAR("落下岗楼的位点",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"落下岗楼的位点");
		GELEM_VARVECTOR(i_math::matrix43f,sitesStand)
			GELEM_EDITVAR("在岗楼上站立的位点",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"在岗楼上站立的位点");

		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("出现单位",GVT_U,GSem(GSem_RecordID,"units"),"岗楼上会出现的单位");
		GELEM_VAR_INIT(RecordID,idResideBuff,RecordID_Invalid);
			GELEM_EDITVAR("驻留Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"岗楼上出现的单位的驻留Buff");

	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}


	RecordID idUnit;
	RecordID idResideBuff;
	std::vector<i_math::spheref> shape;//外形模拟
	std::vector<i_math::matrix43f> sitesEntry;//爬上点
	std::vector<i_math::matrix43f> sitesExit;//落下点
	std::vector<i_math::matrix43f> sitesStand;//站立点


};


class CLoWatchTower:public CLoAgent
{
public:
	CLoWatchTower()
	{
		_attrResists=NULL;
		_attrEvade=NULL;
	}
	DEFINE_LEVELOBJ_CLASS(CLoWatchTower,CLASSUID_WatchTower);

	virtual const char *GetShowName()	{		return "岗楼";	}

	virtual void PostCreate();
	virtual void OnDestroy();

	virtual BOOL OnActivate();

	virtual void Update();

	virtual LevelObjShapeType GetShapeType()	{		return LevelObjShape_MultiCircle;	}//外形模拟方式
	virtual LevelObjCircle *GetShapeCircles(DWORD &count);

	virtual CLevelObjResidable*GetResidable()	{		return &_residable;	}
	virtual CLevelBuffs *GetBuffs()	{		return &_buffs;	}

	virtual BOOL IsServerOnly()	{		return FALSE;	}

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	virtual LevelAttr_Base*GetAttr_Base()	{		return &_attrBase;	}
	virtual LevelAttr_Resists*GetAttr_Battle()	{		return _attrResists;	}
	virtual LevelAttr_Evade*GetAttr_Evade()	{		return _attrEvade;	}


	virtual BOOL FindEntry(LevelPos &pos,LevelPos &posEntry);

	virtual void OnEvent(LevelEvent &e);

protected:

	void _CreateInitialUnit();

	void _UpdateAI();

	//Shape
	std::vector<CUnit*>_shape;
	CLevelObjResidable_Single _residable;
	std::vector<LevelObjCircle> _circles;

	//Attrs
	LevelAttr_Base _attrBase;
	LevelAttr_Evade *_attrEvade;
	LevelAttr_Resists *_attrResists;

	//Buffs
	CLevelBuffs _buffs;



};

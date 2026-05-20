#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "LevelChancer.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"
#include "LevelObjResidable.h"

#define CLASSUID_Hole 8

struct HoleSpawnAmount
{
	RecordID idUnit;
	int amountMin;
	int amountMax;

	int grdBase;
	int grdVary;


	BEGIN_GOBJ_PURE(HoleSpawnAmount,1);

		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位的类型",GVT_U,GSem(GSem_RecordID,"units"),"洞内单位的类型,如果为空,使用第一种单位");

		GELEM_VAR_INIT(int,amountMin,1);
			GELEM_EDITVAR("最小个数",GVT_S,GSem_Interger,"洞内最少有几个单位");
		GELEM_VAR_INIT(int,amountMax,3);
			GELEM_EDITVAR("最大个数",GVT_S,GSem_Interger,"洞内最多有几个单位");

		GELEM_VAR_INIT(int,grdBase,1);
			GELEM_EDITVAR("等级",GVT_S,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"单位的等级");
		GELEM_VAR_INIT(int,grdVary,0);
			GELEM_EDITVAR("等级浮动",GVT_S,GSem(GSem_Interger,LevelGradeVary_SemConstraint),"单位的等级的浮动值");

	END_GOBJ();

};

struct LopHole:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopHole,CLASSUID_Hole);

	LopHole();
	~LopHole();

	BEGIN_GOBJ(LopHole,1);

		GELEM_ALLOWDISABLE();

		GELEM_VAR_INIT(LevelChanceHandle,hChance,LevelChanceHandle_Invalid);

		GELEM_OBJVECTOR(HoleSpawnAmount,amnts)
			GELEM_EDITOBJ("出生单位","各种出生单位的信息");

	END_GOBJ();

	virtual BOOL CheckCreateChance(CLevel *level,CLevelObjSrc *los);

	std::vector<HoleSpawnAmount> amnts;

	LevelChanceHandle hChance;
};

struct HoleSpawnCategory
{
	RecordID idUnit;
	RecordID idBirthBuff;
	float rate;
	std::vector<i_math::matrix43f> sites;//出生点

	BEGIN_GOBJ_PURE(HoleSpawnCategory,1);


		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位的类型",GVT_U,GSem(GSem_RecordID,"units"),"洞内单位的类型");
		GELEM_VAR_INIT(RecordID,idBirthBuff,RecordID_Invalid);
			GELEM_EDITVAR("单位出生Buf",GVT_U,GSem(GSem_RecordID,"buffs"),"单位钻出来的出生Buff");

		GELEM_VAR_INIT(float,rate,1.0f);
			GELEM_EDITVAR("出生几率",GVT_F,GSem(GSem_Float,"0,1,0.001"),"多大的几率出生");
		GELEM_VARVECTOR(i_math::matrix43f,sites)
			GELEM_EDITVAR("出生位点",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"单位出生的位点");

	END_GOBJ();
};

struct LosHole:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosHole,CLASSUID_Hole);

	BEGIN_GOBJ_PURE(LosHole,1);

		GELEM_ALLOWDISABLE();
		GELEM_AGENTRECORD();

		GELEM_VARVECTOR(i_math::spheref,shape)
			GELEM_EDITVAR("外形模拟",GVT_Fx4,GSem(GSem_Unknown,"SphereSetLS"),"洞外形的模拟");
		GELEM_VARVECTOR(i_math::matrix43f,sitesReside)
			GELEM_EDITVAR("驻留的位点",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"单位钻入洞中后停留的位点");


		GELEM_OBJVECTOR(HoleSpawnCategory,cats)
			GELEM_EDITOBJ("出生单位信息","所有可以在这个洞里出生的单位信息");

	END_GOBJ();


	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

	std::vector<i_math::spheref> shape;//外形模拟
	std::vector<i_math::matrix43f> sitesReside;//Reside的位点

	std::vector<HoleSpawnCategory> cats;//各种类型

};


class CLoHole:public CLoAgent
{
public:
	CLoHole()
	{
		_grdRef=0;
	}
	DEFINE_LEVELOBJ_CLASS(CLoHole,CLASSUID_Hole);

	virtual const char *GetShowName()	{		return "洞";	}

	virtual void PostCreate();
	virtual void OnDestroy();

	virtual BOOL OnActivate();

	virtual void Update();

	virtual LevelObjShapeType GetShapeType()	{		return LevelObjShape_MultiCircle;	}//外形模拟方式
	virtual LevelObjCircle *GetShapeCircles(DWORD &count);

	virtual CLevelObjResidable*GetResidable()	{		return &_residable;	}

	virtual BOOL IsServerOnly()	{		return FALSE;	}

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);


protected:

	struct BugInfo
	{
		int count;
		LevelGrade grdBase;
		LevelGrade grdVary;
	};

	void _UpdateAI();

	LevelGrade _grdRef;

	std::vector<CUnit*>_shape;

	std::vector<LevelObjCircle> _circles;

	CLevelObjResidable_Infinite _residable;

	std::vector<BugInfo>_infosBug;


};

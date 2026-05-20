#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "LevelDefines.h"

#include "LevelChancer.h"

#include "records/records.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"


struct LosSpawner:public CLevelObjSrc
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosSpawner,1);

	BEGIN_GOBJ_PURE(LosSpawner,1);

		GELEM_ALLOWDISABLE();

		GELEM_AGENTRECORD();
	
		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);GELEM_VERSION(2)
			GELEM_EDITVAR("单位id",GVT_U,GSem(GSem_RecordID,"units"),"单位的id");

	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return FALSE;	}

	RecordID idUnit;
};

struct LopSpawner:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopSpawner,1);


	BEGIN_GOBJ_PURE(LopSpawner,1);
		
		GELEM_ALLOWDISABLE();

		GELEM_VAR_INIT(BOOL,bRef,TRUE);
			GELEM_EDITVAR("参考地图分布信息",GVT_S,GSem_Boolean,"如果为TRUE,表示会参考地图上的Agent分布信息来随机产生");
		GELEM_VAR_INIT(float,radius,3.0f);
			GELEM_EDITVAR("半径",GVT_F,GSem(GSem_Float,"0.0f,20.0f,0.11f"),"创建单位的范围半径");
		GELEM_VAR_INIT(int,count,3);
			GELEM_EDITVAR("个数",GVT_S,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20"),"创建单位的个数");
		GELEM_VAR_INIT(int,grdBase,1);
			GELEM_EDITVAR("等级",GVT_S,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"单位的等级");
		GELEM_VAR_INIT(int,grdVary,0);
			GELEM_EDITVAR("等级浮动",GVT_S,GSem(GSem_Interger,LevelGradeVary_SemConstraint),"单位的等级的浮动值");
		GELEM_VAR_INIT(DWORD,idMaster,LevelPlayerID_Wild);
			GELEM_EDITVAR("主人ID",GVT_U,GSem(GSem_Interger,"野外敌对:15,野外中立:14,0:0,1:1,2:2,3:3"),"属于那个Player");

	END_GOBJ();

	virtual BOOL CheckCreateChance(CLevel *level,CLevelObjSrc *los);


	BOOL bRef;

	float radius;
	int count;
	DWORD idMaster;
	int grdBase;
	int grdVary;

};

class CLoSpawner:public CLoAgent
{
public:
	DEFINE_LEVELOBJ_CLASS(CLoSpawner,1);

	virtual const char *GetShowName()	{		return "单位产生器";	}

	virtual BOOL OnActivate();

	virtual BOOL IsServerOnly()	{		return TRUE;	}


protected:


};

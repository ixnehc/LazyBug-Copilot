#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "bitset/bitset.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "LevelChancer.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"
#include "LevelObjResidable.h"

#define CLASSUID_DormantSpawner 16

#define DORMANTSPAWNER_MAXSITE (32)

struct LopDormantSpawner:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopDormantSpawner,CLASSUID_DormantSpawner);

	BEGIN_GOBJ_PURE(LopDormantSpawner,1);

		GELEM_ALLOWDISABLE();

		GELEM_VARVECTOR(i_math::matrix43f,sites)
			GELEM_EDITVAR("休眠位点",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"休眠体出现的位点");
		GELEM_VARVECTOR(i_math::matrix43f,sitesRevive)
			GELEM_EDITVAR("苏醒位点",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"休眠体苏醒后出现的位点");

		GELEM_VAR_INIT(int,grdBase,1);
			GELEM_EDITVAR("等级",GVT_S,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"单位的等级");
		GELEM_VAR_INIT(int,grdVary,0);
			GELEM_EDITVAR("等级浮动",GVT_S,GSem(GSem_Interger,LevelGradeVary_SemConstraint),"单位的等级的浮动值");
		GELEM_VAR_INIT(int,nMin,1);
			GELEM_EDITVAR("最小个数",GVT_S,GSem_Interger,"最小个数");
		GELEM_VAR_INIT(int,nMax,1);
			GELEM_EDITVAR("最大个数",GVT_S,GSem_Interger,"最小个数");

	END_GOBJ();

	virtual BOOL CheckCreateChance(CLevel *level,CLevelObjSrc *los);

	int grdBase;
	int grdVary;

	int nMin;
	int nMax;

	std::vector<i_math::matrix43f> sites;//出生点
	std::vector<i_math::matrix43f> sitesRevive;//出生点
};

struct LosDormantSpawner:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosDormantSpawner,CLASSUID_DormantSpawner);

	BEGIN_GOBJ_PURE(LosDormantSpawner,1);

		GELEM_ALLOWDISABLE();

		GELEM_AGENTRECORD();
		GELEM_AGENTGUID();

		GELEM_VAR_INIT(BOOL,bPersist,FALSE);
			GELEM_EDITVAR("保存状态",GVT_U,GSem(GSem_Interger,"不保存状态,保存状态"),"是否要保存状态");

		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位的类型",GVT_U,GSem(GSem_RecordID,"units"),"休眠单位的类型");
		GELEM_VAR_INIT(RecordID,idDormantBuff,RecordID_Invalid);
			GELEM_EDITVAR("单位的休眠Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位的休眠Buff");
	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

	BOOL bPersist;

	RecordID idUnit;
	RecordID idDormantBuff;
};


class CLoDormantSpawner:public CLoAgent
{
public:
	CLoDormantSpawner()
	{
		_bPersistValid=FALSE;
		_nBuffEntries=0;
	}
	DEFINE_LEVELOBJ_CLASS(CLoDormantSpawner,CLASSUID_DormantSpawner);

	virtual const char *GetShowName()	{		return "休眠单位出生器";	}

	virtual BOOL OnActivate();
	virtual void OnDeactivate();
	virtual void Update() override;

	virtual BOOL IsServerOnly()	{		return TRUE;	}


protected:

	void _UpdateBuffEntries();
	struct BuffEntry
	{
		int iSite;
		CLevelBuff *buff;
	};
	BuffEntry _entriesBuff[DORMANTSPAWNER_MAXSITE];
	int _nBuffEntries;

	//Persist数据
	void _LoadPersist();
	void _SavePersist();
	BOOL _bPersistValid;
	Bitset<1> _dormants;


};

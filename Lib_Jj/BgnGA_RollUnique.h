#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorParam.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "records/recordsdefine.h"



struct RollUniqueEntry
{
	RecordID idItem;
	float wt;

    BEGIN_GOBJ_PURE(RollUniqueEntry,1);

		GELEM_VAR_INIT(RecordID,idItem,RecordID_Invalid);
			GELEM_EDITVAR("道具",GVT_U,GSem(GSem_RecordID,"items"),"创建哪个道具");

		GELEM_VAR_INIT(float,wt,1.0f);
			GELEM_EDITVAR("权重",GVT_U,GSem(GSem_Float,"0.01,1000000,0.01"),"权重");

    END_GOBJ();    
};

struct RollUniquePass
{
	std::vector<RollUniqueEntry> entries;

    BEGIN_GOBJ_PURE(RollUniquePass,1);

		GELEM_OBJVECTOR(RollUniqueEntry,entries);
			GELEM_EDITOBJ("道具列表","道具列表");

    END_GOBJ();    
};


struct RollUniqueParam
{
	std::vector<RollUniquePass> passes;

	BEGIN_GOBJ_PURE(RollUniqueParam,1);

		GELEM_OBJVECTOR(RollUniquePass,passes);
			GELEM_EDITOBJ("Pass列表","Pass列表")

	END_GOBJ();

};

class CBgpGA_RollUnique:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_RollUnique);

	virtual const char *GetTypeName()	{		return "随机Unique";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";

		if (varResult!=StringID_Invalid)
		{
			FormatString(s,"根据参数随机产生Unique道具存入变量[%s]",assist->GetStr(varResult));
		}

	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_RollUnique,476,1);

		GELEM_OBJ(RollUniqueParam,param);
			GELEM_EDITOBJ("参数","参数");
			GELEM_BVR();
		GELEM_BEHAVIORMEM_ITEMRECORD(varResult,"结果变量","道具的ID保存在那个变量中")

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(RollUniqueParam,param);
	StringID varResult;

};


class CBgnGA_RollUnique:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_RollUnique);

	CBgnGA_RollUnique()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


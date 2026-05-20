#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorParam.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "records/recordsdefine.h"



struct RollItemCountEntry
{
	int count;
	float wt;

    BEGIN_GOBJ_PURE(RollItemCountEntry,1);

		GELEM_VAR_INIT( int,count,1);	
			GELEM_EDITVAR( "数量", GVT_S, GSem_Interger, "数量" );

		GELEM_VAR_INIT(float,wt,1.0f);
			GELEM_EDITVAR("权重",GVT_U,GSem(GSem_Float,"0.01,1000000,0.01"),"权重");


    END_GOBJ();    

};


struct RollItemEntry
{
	RecordID idItem;
	float wt;
	int nMin;
	int nMax;

    BEGIN_GOBJ_PURE(RollItemEntry,1);

		GELEM_VAR_INIT(RecordID,idItem,RecordID_Invalid);
			GELEM_EDITVAR("道具",GVT_U,GSem(GSem_RecordID,"items"),"创建哪个道具");

		GELEM_VAR_INIT(float,wt,1.0f);
			GELEM_EDITVAR("权重",GVT_U,GSem(GSem_Float,"0.01,1000000,0.01"),"权重");

		GELEM_VAR_INIT( int,nMin,1);	
			GELEM_EDITVAR( "最小值", GVT_S, GSem_Interger, "最小值" );

		GELEM_VAR_INIT( int,nMax,5);	
			GELEM_EDITVAR( "最大值", GVT_S, GSem_Interger, "最大值" );

    END_GOBJ();    
};


struct RollItemParam
{
	std::vector<RollItemCountEntry> counts;
	std::vector<RollItemEntry> entries;

	BEGIN_GOBJ_PURE(RollItemParam,1);

		GELEM_OBJVECTOR(RollItemCountEntry,counts);
			GELEM_EDITOBJ("数量列表","数量列表")

		GELEM_OBJVECTOR(RollItemEntry,entries);
			GELEM_EDITOBJ("道具列表","道具列表")

	END_GOBJ();

};


struct RollItemResult
{
	StringID varItem;
	StringID varCount;

	BEGIN_GOBJ_PURE(RollItemResult,1);
		GELEM_BEHAVIORMEM_ITEMRECORD(varItem,"道具ID","道具的ID保存在那个变量中")
		GELEM_BEHAVIORMEM_INTERGER(varCount,"道具数量","道具的数量保存在那个变量中")
	END_GOBJ();    
};

class CBgpGA_RollItems:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_RollItems);

	CBgpGA_RollItems()
	{
		GConstructor();
	}
	~CBgpGA_RollItems()
	{
		GDestructor();
	}

	virtual const char *GetTypeName()	{		return "随机道具";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";

// 		if (bprCandidates.nmRef!=StringID_Invalid)
// 		{
// 
// 			std::string s2;
// 			for (int i=0;i<results.size();i++)
// 			{
// 				if (results[i].varItem==StringID_Invalid)
// 					continue;
// 				if (i==0)
// 					FormatString(s2,"%s",assist->GetStr(results[i].varItem));
// 				else
// 					AppendFmtString(s2,"\r\n%s",assist->GetStr(results[i].varItem));
// 			}
// 
// 			if (!s2.empty())
// 				FormatString(s,"产生道具参数[%s]\r\n\r\n------存入变量------\r\n\r\n%s",assist->GetStr(bprCandidates.nmRef),s2.c_str());
// 		}

	}

    BEGIN_GOBJ(CBgpGA_RollItems,1);

		GELEM_OBJ(RollItemParam,param);
			GELEM_EDITOBJ("产生道具参数","产生道具参数");
			GELEM_BVR();

		GELEM_OBJVECTOR(RollItemResult,results);
			GELEM_EDITOBJ("选出的道具","选出的道具")

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(RollItemParam,param);
	std::vector<RollItemResult> results;

};


class CBgnGA_RollItems:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_RollItems);

	CBgnGA_RollItems()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


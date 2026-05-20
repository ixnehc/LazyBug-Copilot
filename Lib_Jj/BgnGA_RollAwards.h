#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorParam.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "records/recordsdefine.h"



struct RollAwardCountEntry
{
	int count;
	float wt;

    BEGIN_GOBJ_PURE(RollAwardCountEntry,1);

		GELEM_VAR_INIT( int,count,1);	
			GELEM_EDITVAR( "数量", GVT_S, GSem_Interger, "数量" );

		GELEM_VAR_INIT(float,wt,1.0f);
			GELEM_EDITVAR("权重",GVT_U,GSem(GSem_Float,"0.01,1000000,0.01"),"权重");


    END_GOBJ();    

};


struct RollAwardEntry
{
	LevelAward::Type tp;
	RecordID idItem;
	RecordID idUpgrade;
	LevelResourceType tpRes;
	float wt;
	int nMin;
	int nMax;

    BEGIN_GOBJ_PURE(RollAwardEntry,1);

		GELEM_VAR_INIT(LevelAward::Type,tp,LevelAward::Item);
			GELEM_EDITVAR("类型",GVT_S,GSem(GSem_Interger,
				"道具:0"		"|升级&资源类型,"
				"升级:1"	"|道具&最小值&最大值&资源类型,"
				"资源:2"		"|升级&道具,"
				"武器普通升级:3"		"|升级&道具&最小值&最大值&资源类型"
				),"奖励类型");

		GELEM_VAR_INIT(RecordID,idItem,RecordID_Invalid);
			GELEM_EDITVAR("道具",GVT_U,GSem(GSem_RecordID,"items"),"哪个道具");

		GELEM_VAR_INIT(RecordID,idUpgrade,RecordID_Invalid);
			GELEM_EDITVAR("升级",GVT_U,GSem(GSem_RecordID,"upgrades"),"哪个升级");

		GELEM_VAR_INIT(LevelResourceType,tpRes,LevelResource_None);
			GELEM_EDITVAR("资源类型",GVT_S,GSem(GSem_Interger,LevelResourceType_SemConstraint),"资源类型");

		GELEM_VAR_INIT(float,wt,1.0f);
			GELEM_EDITVAR("权重",GVT_U,GSem(GSem_Float,"0.01,1000000,0.01"),"权重");

		GELEM_VAR_INIT( int,nMin,1);	
			GELEM_EDITVAR( "最小值", GVT_S, GSem_Interger, "最小值" );

		GELEM_VAR_INIT( int,nMax,5);	
			GELEM_EDITVAR( "最大值", GVT_S, GSem_Interger, "最大值" );

    END_GOBJ();    
};


struct RollAwardParam
{
	std::vector<RollAwardCountEntry> counts;
	std::vector<RollAwardEntry> entries;

	BEGIN_GOBJ_PURE(RollAwardParam,1);

		GELEM_OBJVECTOR(RollAwardCountEntry,counts);
			GELEM_EDITOBJ("数量列表","数量列表")

		GELEM_OBJVECTOR(RollAwardEntry,entries);
			GELEM_EDITOBJ("奖励列表","奖励列表")

	END_GOBJ();

};


class RollAwardsResult:public CBehaviorMemObj
{
	DECLARE_CLASS(RollAwardsResult);

	BEGIN_GOBJ_PURE_UID(RollAwardsResult,1);

		GELEM_VARVECTOR(LevelAward,awards);
			GELEM_UID(1);

	END_GOBJ();

	DWORD GetValidCount()
	{
		DWORD c=0;
		for (int i=0;i<awards.size();i++)
		{
			if (awards[i].bValid)
				c++;
		}
		return c;
	}

	void UpdateExpendable(CLevelPlayer *player);

	void ApplyMerge();

	std::vector<LevelAward> awards;
};

class RollAwardsPrice:public CBehaviorMemObj
{
	DECLARE_CLASS(RollAwardsPrice);

	BEGIN_GOBJ_PURE(RollAwardsPrice,1);

		GELEM_VARVECTOR(LevelAwardPrice,prices);
			GELEM_UID(1);

	END_GOBJ();

	void UpdateAffordable(CLevelPlayer *player);

	std::vector<LevelAwardPrice> prices;
};


class CBgpGA_RollAwards:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_RollAwards);

	virtual const char *GetTypeName()	{		return "随机奖励";	}
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

		FormatString(s,"产生奖励,将结果保存在[%s]中,将价格保存在[%s]中",BVRDESC_StringID(awards),BVRDESC_StringID(prices));


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

    BEGIN_GOBJ_PURE(CBgpGA_RollAwards,1);

		GELEM_OBJ(RollAwardParam,param);
			GELEM_EDITOBJ("产生奖励参数","产生奖励参数");
			GELEM_BVR();

		GELEM_VAR_INIT( StringID,awards,StringID_Invalid);GELEM_BVR();
			GELEM_EDITVAR( "奖励保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "奖励保存在哪个变量里");
		GELEM_VAR_INIT( StringID,prices,StringID_Invalid);
			GELEM_EDITVAR( "奖励的价格保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "奖励价格保存在哪个变量里");

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(RollAwardParam,param);
	DEFINE_BVR(StringID,awards);
	DEFINE_BVR(StringID,prices);


};


class CBgnGA_RollAwards:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_RollAwards);

	CBgnGA_RollAwards()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

	RollAwardsPrice *_EvalPrice(RollAwardsResult *award,CLevelPlayer *player);

};


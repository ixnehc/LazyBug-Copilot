#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"

struct AwardPool_宝藏洞穴
{
	LevelAward::Type tp;
	float wt;
	LevelResourceType tpRes;
	int nMin;
	int nMax;
	RecordID idItem;

	const char *GetBrief(void *param);


	BEGIN_GOBJ_PURE(AwardPool_宝藏洞穴,1);
		GOBJ_GETBRIEF_FUNC(GetBrief);

		GELEM_VAR_INIT(LevelAward::Type,tp,LevelAward::Resource);
			GELEM_EDITVAR("类型",GVT_S,GSem(GSem_Interger,
				"空:16"		"|资源类型&资源最小值&资源最大值&道具,"
				"道具:0"		"|资源类型&资源最小值&资源最大值,"
				"资源:2"		"|道具&权重"
				),"奖励类型");

		GELEM_VAR_INIT(float,wt,1.0f);
			GELEM_EDITVAR("权重",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"权重");

		GELEM_VAR_INIT(RecordID,idItem,RecordID_Invalid);
			GELEM_EDITVAR("道具",GVT_U,GSem(GSem_RecordID,"items"),"哪个道具");

		GELEM_VAR_INIT(LevelResourceType,tpRes,LevelResource_None);
			GELEM_EDITVAR("资源类型",GVT_S,GSem(GSem_Interger,LevelResourceType_SemConstraint),"资源类型");

		GELEM_VAR_INIT( int,nMin,1);	
			GELEM_EDITVAR( "资源最小值", GVT_S, GSem_Interger, "最小值" );

		GELEM_VAR_INIT( int,nMax,5);	
			GELEM_EDITVAR( "资源最大值", GVT_S, GSem_Interger, "最大值" );

	END_GOBJ();    

};

struct AwardEntry_宝藏洞穴
{
	std::vector<AwardPool_宝藏洞穴> pools;
	float difficulty;
	float offDifficulty;

	int CalcAvarageResourceCount(LevelResourceType tpRes);
	void CollectItems(std::unordered_set<RecordID>ids);
	float CalcPossibility(RecordID idItem);


	const char *GetBrief(void *param);

    BEGIN_GOBJ_PURE(AwardEntry_宝藏洞穴,1);

		GOBJ_GETBRIEF_FUNC(GetBrief);

		GELEM_VAR_INIT(float,difficulty,0.2f);
			GELEM_EDITVAR("找到难度",GVT_F,GSem(GSem_Float,"0.0f,1.0,0.05"),"找到难度");

		GELEM_VAR_INIT(float,offDifficulty,0.2f);
			GELEM_EDITVAR("找到难度偏移",GVT_F,GSem(GSem_Float,"0.0f,1.0,0.05"),"找到难度偏移");

		GELEM_OBJVECTOR(AwardPool_宝藏洞穴,pools);
			GELEM_EDITOBJ("奖励池","奖励池");


    END_GOBJ();    


};

struct Param_宝藏洞穴
{
	std::vector<AwardEntry_宝藏洞穴> entries;
	AnimTick durSearch;
	float speedSPCost;

	int CalcAvarageResourceCount(LevelResourceType tpRes);
	void CollectItems(std::unordered_set<RecordID>ids);
	float CalcPossibility(RecordID idItem);

	BEGIN_GOBJ_PURE(Param_宝藏洞穴,1);

		GELEM_VAR_INIT(AnimTick,durSearch,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("搜寻时长",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"搜寻时长");
		GELEM_VAR_INIT(float,speedSPCost,1.0f);
			GELEM_EDITVAR("SP消耗速率",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"Search时每秒消耗多少SP");
		GELEM_OBJVECTOR(AwardEntry_宝藏洞穴,entries);
			GELEM_EDITOBJ("宝藏列表","宝藏列表")

	END_GOBJ();

};



class CBgpGA_宝藏洞穴_Search:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_宝藏洞穴_Search);

	virtual const char *GetTypeName()	{		return "宝藏洞穴_Search";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (varResult!=StringID_Invalid)
			FormatString(s,"搜寻宝藏并将结果保存[%s]中",StrLib_GetStr(varResult));

	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_宝藏洞穴_Search,416,1);
		GELEM_BGP_BASE();

		GELEM_OBJ(Param_宝藏洞穴,param);
			GELEM_EDITOBJ("宝藏参数","宝藏参数");
			GELEM_BVR();

		GELEM_VAR_INIT( StringID,varPercent,StringID_Invalid);
			GELEM_EDITVAR( "搜寻百分比保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:AllNumber"), "搜寻百分比保存在哪个变量里");

		GELEM_VAR_INIT( StringID,varProgress,StringID_Invalid);
			GELEM_EDITVAR( "搜寻进度保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "搜寻进度保存在哪个变量里");

		GELEM_VAR_INIT( StringID,varResult,StringID_Invalid);
			GELEM_EDITVAR( "宝藏保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "搜寻到的宝藏保存在哪个变量里");

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(Param_宝藏洞穴,param);

	StringID varProgress;
	StringID varResult;
	StringID varPercent;

};

class RollAwardsResult;
class CBgnGA_宝藏洞穴_Search:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_宝藏洞穴_Search);

	CBgnGA_宝藏洞穴_Search()
	{
		_tStart=0;
		_tLast=0;
		_tFoundStart=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;
	virtual void Update(BGNOutputs &outputs) override;

public:
	void _AddTreasureInfos();
	void _AddTreasureInfo(CLevelPlayer *player,LevelResourceType tpRes);
	void _CommitFound(AwardEntry_宝藏洞穴 &e,RollAwardsResult *result);

	struct Found
	{
		int idxEntry;
		AnimTick t;
	};

	AnimTick _tStart;
	AnimTick _tLast;

	std::vector<Found> _founds;
	AnimTick _tFoundStart;

};


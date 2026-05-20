#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorParam.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "records/recordsdefine.h"

#include "BgnGA_RollAwards.h"




struct RollSpellEntry
{
	RecordID idUpgrade;
	float wt;

    BEGIN_GOBJ_PURE(RollSpellEntry,1);

		GELEM_VAR_INIT(RecordID,idUpgrade,RecordID_Invalid);
			GELEM_EDITVAR("Spell",GVT_U,GSem(GSem_RecordID,"upgrades"),"哪个升级");

		GELEM_VAR_INIT(float,wt,1.0f);
			GELEM_EDITVAR("权重",GVT_U,GSem(GSem_Float,"0.01,1000000,0.01"),"权重");

    END_GOBJ();    
};


struct RollSpellParam
{
	std::vector<RollAwardCountEntry> counts;
	std::vector<RollSpellEntry> entries;

	BEGIN_GOBJ_PURE(RollSpellParam,1);

		GELEM_OBJVECTOR(RollAwardCountEntry,counts);
			GELEM_EDITOBJ("数量列表","数量列表")
		GELEM_OBJVECTOR(RollSpellEntry,entries);
			GELEM_EDITOBJ("奖励列表","奖励列表")

	END_GOBJ();

};


class CBgpGA_RollSpells:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_RollSpells);

	virtual const char *GetTypeName()	{		return "随机奖励(Spell)";	}
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
		s="随机奖励(Spell)";
	}

    BEGIN_GOBJ_PURE_UID(CBgpGA_RollSpells,1);

		GELEM_OBJ(RollSpellParam,param);
			GELEM_EDITOBJ("产生奖励参数","产生奖励参数");
			GELEM_BVR();

		GELEM_VAR_INIT( StringID,awards,StringID_Invalid);
			GELEM_EDITVAR( "奖励保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "奖励保存在哪个变量里");

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(RollSpellParam,param);
	StringID awards;

};


class CBgnGA_RollSpells:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_RollSpells);

	CBgnGA_RollSpells()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


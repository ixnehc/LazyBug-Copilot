#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorParam.h"

#include "records/recordsdefine.h"

#include "BgnGA_Disciple_Init.h"

#include "BgnGA_RollAwards.h"



class CBgp_Disciple_RollAwards:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Disciple_RollAwards);

	virtual const char *GetTypeName()	{		return "魔法使徒_随机奖励";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";

		if (nmParamVar!=StringID_Invalid)
			FormatString(s,"根据保存在变量[%s]中的参数产生奖励,将结果保存在[%s]中,将价格保存在[%s]中",assist->GetStr(nmParamVar),
			assist->GetStr(awards),assist->GetStr(prices));
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_Disciple_RollAwards,452,1);

		GELEM_BEHAVIORMEM_OBJ(nmParamVar,"参数变量名称","变量名称");

		GELEM_VAR_INIT( StringID,awards,StringID_Invalid);
			GELEM_EDITVAR( "奖励保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "奖励保存在哪个变量里");
		GELEM_VAR_INIT( StringID,prices,StringID_Invalid);
			GELEM_EDITVAR( "奖励的价格保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "奖励价格保存在哪个变量里");

    END_GOBJ();    

public: //当作protected

	StringID nmParamVar;

	StringID awards;
	StringID prices;


};


class CBgn_Disciple_RollAwards:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Disciple_RollAwards);

	CBgn_Disciple_RollAwards()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


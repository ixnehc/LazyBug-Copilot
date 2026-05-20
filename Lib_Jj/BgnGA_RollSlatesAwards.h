#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorParam.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "records/recordsdefine.h"


class CBgpGA_RollSlatesAwards:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_RollSlatesAwards);

	virtual const char *GetTypeName()	{		return "随机奖励(SlatesA)";	}
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
		FormatString(s,"产生奖励,将结果保存在[%s]中",StrLib_GetStr(awards));
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_RollSlatesAwards,482,1);

		GELEM_VAR_INIT( StringID,awards,StringID_Invalid);
			GELEM_EDITVAR( "奖励保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "奖励保存在哪个变量里");

    END_GOBJ();    

public: //当作protected

	StringID awards;

};


class CBgnGA_RollSlatesAwards:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_RollSlatesAwards);

	CBgnGA_RollSlatesAwards()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:


};


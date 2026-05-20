#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_SnailP1舌虫_寻找毒爆点:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_SnailP1舌虫_寻找毒爆点);

	virtual const char *GetTypeName()	{		return "SnailP1舌虫_寻找毒爆点";	}
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
		if (varPos==StringID_Invalid)
			s="n/a";
		else
			FormatString(s,"寻找毒爆点,保存在[%s]中",assist->GetStr(varPos));
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_SnailP1舌虫_寻找毒爆点,407,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_POS(varPos,"[out]位置变量","找到的位置存放在哪里")

    END_GOBJ();    

public: //当作protected
	StringID varPos;

};


class CBgn_SnailP1舌虫_寻找毒爆点:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SnailP1舌虫_寻找毒爆点);

	CBgn_SnailP1舌虫_寻找毒爆点()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



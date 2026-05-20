#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h" 


class CBgpGA_断桥_EndRepair:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_断桥_EndRepair);

	virtual const char *GetTypeName()	{		return "断桥_EndRepair";	}
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
		if (!bAbort)
			s="修复断桥完毕";
		else
			s="修复断桥被打断";
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_断桥_EndRepair,437,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(BOOL,bAbort,TRUE);
			GELEM_EDITVAR("是否中断",GVT_S,GSem_Boolean,"中断");
    END_GOBJ();    

public: //当作protected
	BOOL bAbort;


};

class RollAwardsResult;
class CBgnGA_断桥_EndRepair:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_断桥_EndRepair);

	CBgnGA_断桥_EndRepair()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;

public:

};


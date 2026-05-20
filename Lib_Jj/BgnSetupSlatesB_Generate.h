#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpSetupSlatesB_Generate:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSetupSlatesB_Generate);

	virtual const char *GetTypeName()	{		return "随机生成";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_SlatesB;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="随机生成石板迷宫B";
	}

    BEGIN_GOBJ_PURE_UID2(CBgpSetupSlatesB_Generate,464,1);
		GELEM_BGP_BASE();
		GELEM_OBJ(SlatesB_GenerateParam,param); GELEM_UID(20);
			GELEM_EDITOBJ("生成参数","生成参数");
    END_GOBJ();    

public: //当作protected
	SlatesB_GenerateParam param;

};


class CBgnSetupSlatesB_Generate:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSetupSlatesB_Generate);

	CBgnSetupSlatesB_Generate()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

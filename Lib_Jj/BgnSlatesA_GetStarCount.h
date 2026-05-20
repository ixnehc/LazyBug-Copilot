#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSlatesA_GetStarCount:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSlatesA_GetStarCount);

	virtual const char *GetTypeName()	{		return "得到Star个数";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_SlatesA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"得到Star个数,,结果保存在变量[%s]中",StrLib_GetStr(var));
	}

    BEGIN_GOBJ_PURE_UID2(CBgpSlatesA_GetStarCount,481,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_NUMBER(var,"结果变量","存入哪个变量中")
    END_GOBJ();    

public: //当作protected
	StringID var;

};


class CBgnSlatesA_GetStarCount:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSlatesA_GetStarCount);

	CBgnSlatesA_GetStarCount()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

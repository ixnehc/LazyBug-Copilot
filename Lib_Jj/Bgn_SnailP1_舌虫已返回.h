#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_SnailP1_舌虫已返回:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_SnailP1_舌虫已返回);

	virtual const char *GetTypeName()	{		return "SnailP1_舌虫已返回";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"是");
			STUB_OUT(2,"否");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_SnailP1_舌虫已返回,404,1);
		GELEM_BGP_BASE();
    END_GOBJ();    

public: //当作protected

};


class CBgn_SnailP1_舌虫已返回:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SnailP1_舌虫已返回);

	CBgn_SnailP1_舌虫已返回()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


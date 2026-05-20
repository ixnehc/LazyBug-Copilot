#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_SnailP1_初始化:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_SnailP1_初始化);

	virtual const char *GetTypeName()	{		return "SnailP1_初始化";	}
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
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_SnailP1_初始化,403,1);
		GELEM_BGP_BASE();
    END_GOBJ();    

public: //当作protected

};


class CBgn_SnailP1_初始化:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SnailP1_初始化);

	CBgn_SnailP1_初始化()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


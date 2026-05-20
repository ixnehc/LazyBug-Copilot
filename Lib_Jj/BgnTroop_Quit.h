#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "behaviorgraph/BehaviorCustomConst.h"
#include "behaviorgraph/BehaviorParam.h"

#include "LevelTroops.h"


class CBgpTroop_Quit:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpTroop_Quit);

	virtual const char *GetTypeName()	{		return "退出Troop";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="退出所在Troop";
	}

    BEGIN_GOBJ_PURE_UID2(CBgpTroop_Quit,420,1);

    END_GOBJ();    

public: //当作protected

};


class CBgnTroop_Quit:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_Quit);

	CBgnTroop_Quit()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:


};



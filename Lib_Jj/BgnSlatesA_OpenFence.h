#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSlatesA_OpenFence:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSlatesA_OpenFence);

	virtual const char *GetTypeName()	{		return "打开铁栏";	}
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
		s="n/a";
		FormatString(s,"打开自己对应的铁栏");
	}

    BEGIN_GOBJ_PURE_UID(CBgpSlatesA_OpenFence,1);
		GELEM_BGP_BASE();
    END_GOBJ();    

public: //当作protected

};


class CBgnSlatesA_OpenFence:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSlatesA_OpenFence);

	CBgnSlatesA_OpenFence()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

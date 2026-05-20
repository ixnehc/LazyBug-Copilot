#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSlatesA_FinishProcess:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSlatesA_FinishProcess);

	virtual const char *GetTypeName()	{		return "终止石板Process";	}
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
		s="终止石板Process";
	}

    BEGIN_GOBJ_PURE_UID(CBgpSlatesA_FinishProcess,1);
		GELEM_BGP_BASE();
    END_GOBJ();    

public: //当作protected

};


class CBgnSlatesA_FinishProcess:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSlatesA_FinishProcess);

	CBgnSlatesA_FinishProcess()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

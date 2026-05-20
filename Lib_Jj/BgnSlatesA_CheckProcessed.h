#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSlatesA_CheckProcessed:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSlatesA_CheckProcessed);

	virtual const char *GetTypeName()	{		return "检查石板是否Processed";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_SlatesA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="检查石板是否Processed";
	}

    BEGIN_GOBJ_PURE_UID(CBgpSlatesA_CheckProcessed,1);
		GELEM_BGP_BASE();
    END_GOBJ();    

public: //当作protected

};


class CBgnSlatesA_CheckProcessed:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSlatesA_CheckProcessed);

	CBgnSlatesA_CheckProcessed()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

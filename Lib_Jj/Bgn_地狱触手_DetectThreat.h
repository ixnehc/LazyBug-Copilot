#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_地狱触手_DetectThreat:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_地狱触手_DetectThreat);

	virtual const char *GetTypeName()	{		return "地狱触手_DetectThreat";	}
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

    BEGIN_GOBJ_PURE_UID2(CBgp_地狱触手_DetectThreat,410,1);
		GELEM_BGP_BASE();
    END_GOBJ();    

public: //当作protected

};


class CBgn_地狱触手_DetectThreat:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_地狱触手_DetectThreat);

	CBgn_地狱触手_DetectThreat()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


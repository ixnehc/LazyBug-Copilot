#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgp_Revive:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Revive);

	virtual const char *GetTypeName()	{		return "苏醒自己";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="苏醒";
		if (_nmVar!=StringID_Invalid)
			FormatString(s,"以变量[%s]中的对象为目标进行苏醒",assist->GetStr(_nmVar));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Revive,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_OBJID(_nmVar,"变量名称","苏醒后扑向那个目标")
    END_GOBJ();    

public: //当作protected

	StringID _nmVar;
};


class CBgn_Revive:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Revive);

	CBgn_Revive()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

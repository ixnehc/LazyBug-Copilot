#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgp_SwitchRetinue:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_SwitchRetinue);

	virtual const char *GetTypeName()	{		return "切换随从";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Rtnu;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_nmVar==StringID_Invalid)
			s="n/a";
		else
			FormatString(s,"切换成[%s]中角色的随从",assist->GetStr(_nmVar));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_SwitchRetinue,1);
		GELEM_BGP_BASE();

		GELEM_BEHAVIORMEM_OBJID(_nmVar,"变量名称","切换成哪个对象的随从")
    END_GOBJ();    

public: //当作protected

	StringID _nmVar;
};


class CBgn_SwitchRetinue:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SwitchRetinue);

	CBgn_SwitchRetinue()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

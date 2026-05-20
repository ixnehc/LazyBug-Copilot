#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_CentipedeNode_Activate:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_CentipedeNode_Activate);

	virtual const char *GetTypeName()	{		return "巨蜗_蜈蚣节点_Activate";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CentipedeNode_Activate,1);
		GELEM_BGP_BASE();

		GELEM_BEHAVIORMEM_OBJID(varCentipedeAgent,"蜈蚣对象","蜈蚣对象");

    END_GOBJ();    

public: //当作protected

	StringID varCentipedeAgent;
};


class CBgn_CentipedeNode_Activate:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CentipedeNode_Activate);

	CBgn_CentipedeNode_Activate()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:
};


#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_Centipede_SpawnCyst:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Centipede_SpawnCyst);

	virtual const char *GetTypeName()	{		return "巨蜗_蜈蚣_SpawnCyst";	}
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

    BEGIN_GOBJ_PURE_UID(CBgp_Centipede_SpawnCyst,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(BOOL,bInner,TRUE);GELEM_UID(3)
			GELEM_EDITVAR("内侧还是外侧",GVT_U,GSem(GSem_Interger,"内测:1,外侧:0"),"内侧还是外侧");
    END_GOBJ();    

public: //当作protected
	
	int nStart;
	int nEnd;

	BOOL bInner;

};


class CBgn_Centipede_SpawnCyst:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Centipede_SpawnCyst);

	CBgn_Centipede_SpawnCyst()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:
};


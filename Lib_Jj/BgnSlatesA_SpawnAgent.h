#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSlatesA_SpawnAgent:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSlatesA_SpawnAgent);

	virtual const char *GetTypeName()	{		return "Spawn石板Agent";	}
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
		s="Spawn石板Agent";
	}

    BEGIN_GOBJ_PURE_UID(CBgpSlatesA_SpawnAgent,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,idAgent,RecordID_Invalid);
			GELEM_EDITVAR("Agent",GVT_U,GSem(GSem_RecordID,"agents"),"Agent");
    END_GOBJ();    

public: //当作protected
	RecordID idAgent;

};


class CBgnSlatesA_SpawnAgent:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSlatesA_SpawnAgent);

	CBgnSlatesA_SpawnAgent()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

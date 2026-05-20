#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_CheckAIScenario:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckAIScenario);

	virtual const char *GetTypeName()	{		return "检测AI方案";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (nmScenario==StringID_Invalid)
			s="n/a";
		else
			FormatString(s,"检测当前的AI方案是否为[%s]",assist->GetStr(nmScenario));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckAIScenario,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,nmScenario,StringID_Invalid);
			GELEM_EDITVAR( "AI方案", GVT_U, GSem(GSem_StringID,"AIScenario"), "AI方案" );

	END_GOBJ();    

public: //当作protected

	StringID nmScenario;
};

class CBgn_CheckAIScenario:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckAIScenario);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

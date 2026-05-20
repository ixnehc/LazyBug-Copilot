#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckResiding:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckResiding);

	virtual const char *GetTypeName()	{		return "检查驻留状态";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"驻留");
			STUB_OUT(2,"未驻留");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_idAgent!=RecordID_Invalid)
			FormatString(s,"检测自己是否驻留在[%s]里",assist->GetAgentName(_idAgent));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckResiding,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,_idAgent,RecordID_Invalid);
			GELEM_EDITVAR("Agent",GVT_U,GSem(GSem_RecordID,"agents"),"驻留在什么场所内");
	END_GOBJ();    

public: //当作protected

	RecordID _idAgent;
};


class CBgn_CheckResiding:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckResiding);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

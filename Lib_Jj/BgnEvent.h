#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "enums/enums.h"



enum BgpEvent
{
	BgpEvent_SwitchRetinue,
};

BEGIN_ENUMS(BgpEvent,BgpEvent_)

	ENUM_ENTRY(BgpEvent_SwitchRetinue);

END_ENUMS();


class CBgp_SendEvent:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_SendEvent);

	virtual const char *GetTypeName()	{		return "发送事件";	};
	virtual DWORD GetStubCount()	
	{		
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

    BEGIN_GOBJ_PURE_UID(CBgp_SendEvent,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(BgpEvent,_e,BgpEvent_SwitchRetinue);
			GELEM_EDITVAR("事件",GVT_S,GSem(GSem_Interger,Enums_GetGSemStr(BgpEvent)),"事件");

	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s=Enums_FindName(BgpEvent,_e);
	}

	BgpEvent _e;

};

class CBgn_SendEvent:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SendEvent);

	CBgn_SendEvent()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

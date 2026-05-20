#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_SnailP1_舌被攻击:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_SnailP1_舌被攻击);

	virtual const char *GetTypeName()	{		return "SnailP1_舌被攻击";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"舌节点最近%.2f秒内是否被攻击",ANIMTICK_TO_SECOND(_dur));
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_SnailP1_舌被攻击,406,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(AnimTick,_dur,ANIMTICK_FROM_SECOND(0.1f));
			GELEM_EDITVAR("最近多长时间之内",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"最近多长时间之内");
    END_GOBJ();    

public: //当作protected
	AnimTick _dur;

};


class CBgn_SnailP1_舌被攻击:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SnailP1_舌被攻击);

	CBgn_SnailP1_舌被攻击()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


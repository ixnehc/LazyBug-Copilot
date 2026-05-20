#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorDefines.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LoAgentRef.h"

#include "records/recordsdefine.h"


class CBgpAGA_CheckAgentRefValid:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpAGA_CheckAgentRefValid);

	virtual const char *GetTypeName()	{		return "检测Agent引用是否有效";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_AGA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if ((__bvr_refAgent!=StringID_BhvValInvalidRef)||refAgent.IsValid())
		{
			if (__bvr_refAgent!=StringID_BhvValInvalidRef)
			{
				FormatString(s,"检测Agent(%s)是否为有效引用",StrLib_GetStr(__bvr_refAgent));
			}
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgpAGA_CheckAgentRefValid,414,1);
		GELEM_BGP_BASE();
		GELEM_OBJVAR(LoAgentRef,refAgent);
			GELEM_EDITOBJ_EX("引用","引用",GSem_Unknown);
			GELEM_BVR();
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(LoAgentRef,refAgent);
};


class CBgnAGA_CheckAgentRefValid:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnAGA_CheckAgentRefValid);

	CBgnAGA_CheckAgentRefValid()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


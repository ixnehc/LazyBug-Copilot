#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorDefines.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LoAgentRef.h"

#include "records/recordsdefine.h"


class CBgpAGA_CheckState:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpAGA_CheckState);

	virtual const char *GetTypeName()	{		return "检测AgentState";	}
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
		if (((__bvr_refAgent!=StringID_BhvValInvalidRef)||refAgent.IsValid())&&(nmState!=StringID_Invalid))
		{
			if (__bvr_refAgent!=StringID_BhvValInvalidRef)
			{
				FormatString(s,"检测Agent(%s)是否处于[%s]状态",StrLib_GetStr(__bvr_refAgent),StrLib_GetStr(nmState));
			}
			else
			{
				FormatString(s,"检测指定Agent是否处于[%s]状态",StrLib_GetStr(nmState));
			}
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpAGA_CheckState,1);
		GELEM_OBJVAR(LoAgentRef,refAgent);GELEM_VERSION(2);
			GELEM_EDITOBJ_EX("引用","引用",GSem_Unknown);
			GELEM_BVR();
		GELEM_VAR_INIT( StringID,nmState,StringID_Invalid);	
//			GELEM_EDITVAR( "状态名称", GVT_U, GSem(GSem_StringID,"Agent行为图状态名称引用"), "状态的名称" );
			GELEM_EDITVAR( "状态名称", GVT_U, GSem(GSem_StringID,"行为图状态名称"), "状态的名称" );
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(LoAgentRef,refAgent);
	StringID nmState;
};


class CBgnAGA_CheckState:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnAGA_CheckState);

	CBgnAGA_CheckState()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


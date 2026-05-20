#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelEvents.h"




class CBgp_CheckSkillEvent:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckSkillEvent);

	virtual const char *GetTypeName()	{		return "检测技能事件";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"检测到");
			STUB_OUT(2,"未检测到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		extern const char *GetBVRDesc_AnimTick(AnimTick v,StringID nmRef,FillDescAssist *assist);

		s="n/a";

		if (e!=StringID_Invalid)
		{
			if (!bWait)
				FormatString(s,"侦测当前技能是否触发了动画事件%s ",StrLib_GetStr(e));
			else
				FormatString(s,"持续侦测当前技能是否触发了动画事件%s ",StrLib_GetStr(e));
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_CheckSkillEvent,492,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(BOOL,bWait,TRUE);
			GELEM_EDITVAR("持续检测",GVT_S,GSem_Boolean,"持续检测直至检测到");

		GELEM_VAR_INIT(StringID,e,RecordID_Invalid);
			GELEM_EDITVAR("事件名称",GVT_U,GSem(GSem_StringID,"动画事件"),"事件");

	END_GOBJ();    

public: //当作protected

	StringID e;

	BOOL bWait;

};


class CBgn_CheckSkillEvent:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckSkillEvent);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:
	BOOL _Update(BGNOutputs &outputs);
};

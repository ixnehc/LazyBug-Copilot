#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelEvents.h"




class CBgp_CheckSkillWindow:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckSkillWindow);

	virtual const char *GetTypeName()	{		return "检测技能窗口";	}
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

		if (open!=StringID_Invalid)
		{
			if (!bWait)
			{
				if (close!=StringID_Invalid)
					FormatString(s,"侦测当前技能是否处于窗口 %s ~ %s 之间",StrLib_GetStr(open),StrLib_GetStr(close));
				else
					FormatString(s,"侦测当前技能是否打开了窗口 %s ",StrLib_GetStr(open));
			}
			else
			{
				if (close!=StringID_Invalid)
					FormatString(s,"持续侦测当前技能是否处于窗口 %s ~ %s 之间",StrLib_GetStr(open),StrLib_GetStr(close));
				else
					FormatString(s,"持续侦测当前技能是否打开了窗口 %s ",StrLib_GetStr(open));
			}
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckSkillWindow,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(BOOL,bWait,TRUE);
			GELEM_EDITVAR("持续检测",GVT_S,GSem_Boolean,"持续检测直至检测到");

		GELEM_VAR_INIT(StringID,open,RecordID_Invalid);
			GELEM_EDITVAR("开始事件",GVT_U,GSem(GSem_StringID,"动画事件"),"开始事件");

		GELEM_VAR_INIT(StringID,close,RecordID_Invalid);
			GELEM_EDITVAR("结束事件",GVT_U,GSem(GSem_StringID,"动画事件"),"开始事件");

	END_GOBJ();    

public: //当作protected

	StringID open;
	StringID close;

	BOOL bWait;

};


class CBgn_CheckSkillWindow:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckSkillWindow);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:
	BOOL _Update(BGNOutputs &outputs);
};

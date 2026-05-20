#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_SwitchSkillStage:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_SwitchSkillStage);

	virtual const char *GetTypeName()	{		return "切换Skill阶段";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="终止技能";
		if (nm!=StringID_Invalid)
			FormatString(s,"切换技能为%s阶段",assist->GetStr(nm));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_SwitchSkillStage,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(StringID,nm,StringID_Invalid);GELEM_UID(1);
			GELEM_EDITVAR("阶段名称",GVT_U,GSem(GSem_StringID,"技能阶段"),"阶段名称");

	END_GOBJ();    

public: //当作protected

	StringID nm;

};


class CBgn_SwitchSkillStage:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SwitchSkillStage);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

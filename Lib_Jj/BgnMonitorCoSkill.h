#pragma once

#include "LevelDefines.h"

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_MonitorCoSkill:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_MonitorCoSkill);

	virtual const char *GetTypeName()	{		return "监控主人的协同技能";	}
	virtual DWORD GetStubCount()
	{
		return coskills.size()+1;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		if (idx==0)
			return PadStub("开始",PadStub_In,TRUE);
		idx--;

		static std::string str;

		if (idx<coskills.size())
		{
			RecordID s=coskills[idx];
			if (s==RecordID_Invalid)
				return PadStub("n/a",PadStub_Out,TRUE);
			FormatString(str,"!s!%d",s);
			return PadStub(str.c_str(),PadStub_Out,TRUE);
		}

		return PadStub();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (coskills.size()>0)
			s="监控主人的协同技能";
	}

    BEGIN_GOBJ_PURE_UID(CBgp_MonitorCoSkill,1);
		GELEM_BGP_BASE();

		GELEM_VARVECTOR_INIT(RecordID,coskills,RecordID_Invalid);
			GELEM_EDITVAR("监控的技能",GVT_U,GSem(GSem_RecordID,"skills"),"监控哪些技能");
	END_GOBJ();    

public: //当作protected

	std::vector<RecordID> coskills;



};

class CBgn_MonitorCoSkill:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_MonitorCoSkill);
	CBgn_MonitorCoSkill()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

};

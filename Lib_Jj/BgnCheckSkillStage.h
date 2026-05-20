#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "strlib/strlib.h"



class CBgp_CheckSkillStage:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckSkillStage);

	virtual const char *GetTypeName()	{		return "检测Skill阶段";	}
	virtual DWORD GetStubCount()
	{
		return nms.size()+3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		if (idx==0)
			return PadStub("开始",PadStub_In,1);
		if (idx==1)
			return PadStub("初始阶段",PadStub_Out,1);
		if (idx-1>nms.size()+1)
			return PadStub();
		if (idx-1==nms.size()+1)
			return PadStub("其余阶段",PadStub_Out,1);

		StringID nm=nms[idx-2];
		if (nm==StringID_Invalid)
			return PadStub("n/a",PadStub_Out,1);
		return PadStub(StrLib_GetStr(nm),PadStub_Out,1);
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckSkillStage,1);
		GELEM_BGP_BASE();

		GELEM_VARVECTOR_INIT(StringID,nms,StringID_Invalid);GELEM_UID(2);
			GELEM_EDITVAR("阶段名称",GVT_U,GSem(GSem_StringID,"技能阶段"),"阶段名称");

	END_GOBJ();    

public: //当作protected

	std::vector<StringID> nms;

};


class CBgn_CheckSkillStage:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckSkillStage);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

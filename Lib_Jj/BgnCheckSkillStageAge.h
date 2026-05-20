#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "strlib/strlib.h"



class CBgp_CheckSkillStageAge:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckSkillStageAge);

	virtual const char *GetTypeName()	{		return "检测Skill阶段时间";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"检测技能在当前的阶段里的时间是否介于 %.2f秒~%.2f秒 之间",tMin,tMax);
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckSkillStageAge,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(float,tMin,0.0f);
			GELEM_EDITVAR("最小时间",GVT_F,GSem(GSem_Float,"0.0,100000.0,0.05"),"最小时间");
		GELEM_VAR_INIT(float,tMax,10.0f);
			GELEM_EDITVAR("最大时间",GVT_F,GSem(GSem_Float,"0.0,100000.0,0.05"),"最大时间");

	END_GOBJ();    

public: //当作protected

	float tMin;
	float tMax;

};


class CBgn_CheckSkillStageAge:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckSkillStageAge);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

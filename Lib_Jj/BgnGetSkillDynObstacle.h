#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_GetSkillDynObstacle:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_GetSkillDynObstacle);

	virtual const char *GetTypeName()	{		return "得到技能障碍对象";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		extern const char *GetBgnRegName(StringID nm);

		s="得到最近的技能障碍对象";
		AppendFmtString(s,"\n结果保存在变量[%s]中",assist->GetStr(nmVar));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_GetSkillDynObstacle,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_OBJID(nmVar,"保存变量","对象保存在那个变量中")

	END_GOBJ();    

public: //当作protected

	StringID nmVar;

};


class CBgn_GetSkillDynObstacle:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_GetSkillDynObstacle);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



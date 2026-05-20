#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckSkill:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckSkill);

	virtual const char *GetTypeName()	{		return "检测Skill";	}
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
		std::string ss;
		switch(type)
		{
			case 0:				ss="自己";				break;
			case 1:				ss="Threat";				break;
			case 2:
			{
				ss="[";
				ss+=assist->GetStr(nmLo);
				ss+="]";
				break;
			}
		}
		if (idSkill==RecordID_Invalid)
			FormatString(s,"检测%s是否在施放一个技能",ss.c_str());
		else
			FormatString(s,"是否%s在施放[ %s ]技能",ss.c_str(),assist->GetSkillName(idSkill));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckSkill,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(int,type,0);
			GELEM_EDITVAR("检查谁的Skill",GVT_U,GSem(GSem_Interger,
				"自己:0" "|游戏对象变量,"
				"Threat:1" "|游戏对象变量,"
				"指定对象:2" ""
				),"检查谁的Skill");
		GELEM_BEHAVIORMEM_OBJID(nmLo,"游戏对象变量","检测哪个游戏对象")
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_U,GSem(GSem_RecordID,"skills"),"技能");
			GELEM_BVR();
	END_GOBJ();    

public: //当作protected
	int type;

	StringID nmLo;

	DEFINE_BVR(RecordID,idSkill);
};


class CBgn_CheckSkill:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckSkill);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

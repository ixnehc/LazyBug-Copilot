#pragma once

#include "behaviorgraph/BehaviorGraphPads.h"
#include "LevelBehavior.h"

#include "LevelDefines.h"


class CBgp_JumpAttack:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_JumpAttack);

	virtual const char *GetTypeName()	{		return "跳跃攻击";	}
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
		s="n/a";
		if(_nmVar!=StringID_Invalid)
		{
			std::string nm=assist->GetSkillName(idSkill);
			if (!nm.empty())
			{
				FormatString(s,"针对变量[%s]中的指定目标释放技能[ %s ]",assist->GetStr(_nmVar),nm.c_str());
			}
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_JumpAttack,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("使用技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
		GELEM_VAR_INIT(LevelSkillGrade,grd,LevelSkillGrade_Invalid);
			GELEM_EDITVAR("技能等级",GVT_B,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"该技能的等级");

		GELEM_BEHAVIORMEM_OBJID(_nmVar,"攻击目标变量","攻击目标")

    END_GOBJ();    

public: //当作protected

	RecordID idSkill;
	LevelSkillGrade grd;

	StringID _nmVar;
};


class CBgn_JumpAttack:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_JumpAttack);

	CBgn_JumpAttack()
	{
	}

	virtual void Destroy();


	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);



protected:
};

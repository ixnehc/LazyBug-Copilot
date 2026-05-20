#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_AttackPos:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_AttackPos);

	virtual const char *GetTypeName()	{		return "攻击指定位置(GA)";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"使用位点集释放技能[ %s ]",GetBVRDesc_SkillID(BVR_ARG(idSkill),assist));
	}

    BEGIN_GOBJ_PURE(CBgpGA_AttackPos,1);
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("使用技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
			GELEM_BVR();
		GELEM_VAR_INIT(LevelSkillGrade,grd,LevelSkillGrade_Invalid);
			GELEM_EDITVAR("技能等级",GVT_B,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"该技能的等级");
			GELEM_BVR();
		GELEM_OBJ(BP_MatSet,sites);
			GELEM_EDITOBJ("位点","位点");
			GELEM_BVR();

		GELEM_VAR_INIT(int,nChooses,RecordID_Invalid);
			GELEM_EDITVAR("选择几个位点",GVT_U,GSem_Interger,"选择几个位点,0表示选择所有位点");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(RecordID,idSkill);
	DEFINE_BVR(LevelSkillGrade,grd);

	DEFINE_BVR(BP_MatSet,sites);
	DEFINE_BVR(int,nChooses);
};


class CBgnGA_AttackPos:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_AttackPos);

	CBgnGA_AttackPos()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

};


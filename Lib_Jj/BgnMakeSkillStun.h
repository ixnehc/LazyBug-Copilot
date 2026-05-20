#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelDeal.h"

#include "records/recordsdefine.h"

class CBgp_MakeSkillStun:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_MakeSkillStun);

	enum TargetType
	{
		TargetType_None,
		TargetType_StunSrc,
		TargetType_StunSrcPos,
		TargetType_BackToStunSrc,

		TargetType_ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "触发Skill硬直";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Buff;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_idSkill!=RecordID_Invalid)
			FormatString(s,"施放技能(%s)",assist->GetSkillName(_idSkill));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_MakeSkillStun,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_U,GSem(GSem_RecordID,"skills"),"技能");
		GELEM_VAR_INIT(TargetType,_tpTarget,TargetType_StunSrc);
			GELEM_EDITVAR("技能目标类型",GVT_U,GSem(GSem_Interger,"无目标:0,硬直Src:1,硬直Src位置:2,背对硬直Src:3"),"技能目标类型");
    END_GOBJ();    

public: //当作protected

	RecordID _idSkill;
	TargetType _tpTarget;
};


class CBgn_MakeSkillStun:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_MakeSkillStun);

	CBgn_MakeSkillStun()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

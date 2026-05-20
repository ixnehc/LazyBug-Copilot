#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpThreat_Attack:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_Attack);

	virtual const char *GetTypeName()	{		return "攻击Threat";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Threat;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		std::string nm=assist->GetSkillName(idSkill);
		if (!nm.empty())
		{
			FormatString(s,"[ %s ]技能攻击",nm.c_str());

			if (distKeep>0.0f)
				AppendFmtString(s,"\n保持距离%.2f米,每%.2f秒检查一次",distKeep,ANIMTICK_TO_SECOND(durCheckEscape));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_Attack,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("使用技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
			GELEM_BVR();
		GELEM_VAR_INIT(LevelSkillGrade,grd,LevelSkillGrade_Invalid);
			GELEM_EDITVAR("技能等级",GVT_B,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"该技能的等级");
		GELEM_VAR_INIT(float,distKeep,0.0f);
			GELEM_EDITVAR("保持距离",GVT_F,GSem(GSem_Float,"0,20,0.1"),"在攻击敌人时,保持多远的距离");
		GELEM_VAR_INIT(AnimTick,durCheckEscape,ANIMTICK_FROM_SECOND(5.0f));
			GELEM_EDITVAR("保持距离检测间隔",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"多长时间检测一次保持距离,单位为秒");	
		GELEM_VAR_INIT(BOOL,bKeepFacingThreat,FALSE);
			GELEM_EDITVAR("是否持续面向Threat",GVT_S,GSem_Boolean,"是否持续面向Threat");
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(RecordID,idSkill);
	LevelSkillGrade grd;
	float distKeep;
	AnimTick durCheckEscape;
	BOOL bKeepFacingThreat;
};


struct LevelRecordSkill;
class CBgnThreat_Attack:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_Attack);

	enum State
	{
		None,
		Attack,
		Escape,
	};

	CBgnThreat_Attack()
	{
		_target=NULL;
		_state=None;
		_tLastCheckEscape=0;
		_verCast=0xffffffff;
		_durEscape=0;
	}



	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:
	void _Start(CLevelObj *target,BOOL bCheckEscapce);
	void _Stop();
	BOOL _IsInKeepDist(CLevelObj *lo,CLevelObj *target);

	CLevelObj *_target;

	State _state;
	AnimTick _tStateStart;//当前状态开始的时间
	AnimTick _durEscape;

	AnimTick _tLastCheckEscape;//上一次检查要不要escape的时间
	DWORD _verCast;

};


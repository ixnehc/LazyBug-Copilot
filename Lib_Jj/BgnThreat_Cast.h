#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpThreat_Cast:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_Cast);

	enum TargetType
	{
		Target_Threat,
		Target_Custom,

		Target_ForceDword=0xffffffff,
	};


	virtual const char *GetTypeName()	{		return "向Threat施放技能";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Threat;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"向%s施放[ %s ]技能",BgnLevelSkillTarget::GetDesc(BVR_ARG(target)),GetBVRDesc_SkillID(BVR_ARG(idSkill),assist));
		if (bFinishAtCanCancel)
			s+="\n技能被取消时返回成功";
		if (bFilterWeaks)
		{
			if (durWeaks>0)
				AppendFmtString(s,"\n过滤弱点%.2f秒",ANIMTICK_TO_SECOND(durWeaks));
			else
				s+="\n过滤弱点";
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_Cast,1);
		GELEM_BGP_BASE();

		GELEM_OBJ(BgnLevelSkillTarget,target);GELEM_UID(2); 
			GELEM_EDITOBJ("目标","目标");
			GELEM_BVR();

		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("使用技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
			GELEM_BVR();

		GELEM_VAR_INIT(BOOL,bInterruptCurSkill,FALSE); 
			GELEM_EDITVAR("打断当前技能",GVT_S,GSem_Boolean,"打断当前技能");
		GELEM_VAR_INIT(BOOL,bFinishAtCanCancel,FALSE); GELEM_UID(1); GELEM_VERSION(2);
			GELEM_EDITVAR("能被取消时结束",GVT_S,GSem_Boolean,"能被取消时结束");
		GELEM_VAR_INIT(int,tpSkillTarget,LevelSkillTarget::Target_DefObj);
			GELEM_EDITVAR("技能目标类型",GVT_S,GSem(GSem_Interger,
				"针对目标对象施放:1"		"|固定位置变量&预测时间,"
				"针对目标对象位置施放:2"		"|固定位置变量,"
				"从固定位置对目标对象施放:4"	"|预测时间"
				),"技能目标类型");
		GELEM_BEHAVIORMEM_POS(varFixPos,"固定位置变量","使用那个变量里的位置")
		GELEM_VAR_INIT(float,durPredict,0.0f);
			GELEM_EDITVAR("预测时间",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"预测时间");
		GELEM_VAR_INIT(LevelSkillGrade,grd,LevelSkillGrade_Invalid);
			GELEM_EDITVAR("技能等级",GVT_B,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"该技能的等级");
		GELEM_VAR_INIT(BOOL,bFilterWeaks,FALSE);
			GELEM_EDITVAR("过滤弱点",GVT_S,GSem(GSem_Boolean,"弱点,弱点过滤时间"),"是否要过滤弱点");
		GELEM_OBJ(WeaksEx,weaksFilter);
			GELEM_EDITOBJ("弱点","弱点");
		GELEM_VAR_INIT(AnimTick,durWeaks,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("弱点过滤时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"弱点过滤时间,0表示一直过滤到技能结束");
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(BgnLevelSkillTarget,target);

	DEFINE_BVR(RecordID,idSkill);
	LevelSkillGrade grd;

	BOOL bInterruptCurSkill;
	BOOL bFinishAtCanCancel;

	LevelSkillTarget::Type tpSkillTarget;
	float durPredict;
	StringID varFixPos;

	BOOL bFilterWeaks;
	WeaksEx weaksFilter;
	AnimTick durWeaks;

};


struct LevelRecordSkill;
class CBgnThreat_Cast:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_Cast);

	CBgnThreat_Cast()
	{
		_verCast=0xffffffff;
		_tStart=ANIMTICK_INFINITE;
		_bWeaksFiltered=FALSE;
	}



	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:
	void _Start(CLevelObj *target);
	void _Stop();

	void _ClearWeaksFilter();

	DWORD _verCast;

	AnimTick _tStart;
	BOOL _bWeaksFiltered;

};


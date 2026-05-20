#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgp_Attack:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Attack);

	virtual const char *GetTypeName()	{		return "自由攻击";	}
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
		std::string nm=assist->GetSkillName(idSkill);
		if (!nm.empty())
		{
			if (radiusMin<=0.0f)
				FormatString(s,"[ %s ]技能攻击:\n%.2f米范围内的目标",nm.c_str(),radius);
			else
				FormatString(s,"[ %s ]技能攻击:\n%.2f米~%.2f米范围内的目标",nm.c_str(),radiusMin,radius);

			if (distKeep>0.0f)
				AppendFmtString(s,"\n保持距离%.2f米,每%.2f秒检查一次",distKeep,ANIMTICK_TO_SECOND(durCheckEscape));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Attack,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("使用技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
			GELEM_BVR();
		GELEM_VAR_INIT(LevelSkillGrade,grd,LevelSkillGrade_Invalid);
			GELEM_EDITVAR("技能等级",GVT_B,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"该技能的等级");
		GELEM_VAR_INIT(DetectSightType,tpSight,DetectSightType_Me);
			GELEM_EDITVAR("侦测源",GVT_S,GSem(GSem_Interger,"自己,Owner,Troop"),"以谁的视野进行侦测");
		GELEM_OBJ(LevelDetectWeights,weights);
			GELEM_EDITOBJ("侦测权重","侦测权重");
			GELEM_BVR();
		GELEM_VAR_INIT(float,radius,5.0f);
			GELEM_EDITVAR("侦测最大半径",GVT_F,GSem(GSem_Float,"0,200,0.1"),"搜索敌人时的侦测半径");
			GELEM_BVR();
		GELEM_VAR_INIT(float,radiusMin,0.0f);
			GELEM_EDITVAR("侦测最小半径",GVT_F,GSem(GSem_Float,"0,200,0.1"),"搜索敌人时的侦测半径");
			GELEM_BVR();
		GELEM_VAR_INIT(float,distLock,2.0f);
			GELEM_EDITVAR("锁定距离",GVT_F,GSem(GSem_Float,"0,20,0.1"),"在离目标多大距离内将会锁定它(不寻找其它攻击目标)");
		GELEM_VAR_INIT(float,distKeep,0.0f);
			GELEM_EDITVAR("保持距离",GVT_F,GSem(GSem_Float,"0,20,0.1"),"在攻击敌人时,保持多远的距离");
		GELEM_VAR_INIT(AnimTick,durCheckEscape,ANIMTICK_FROM_SECOND(5.0f));
			GELEM_EDITVAR("保持距离检测间隔",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"多长时间检测一次保持距离,单位为秒");	
		GELEM_BEHAVIORMEM_OBJID(nmVar,"保存变量","侦测到的对象保存在那个变量中")
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(LevelDetectWeights,weights);

	DetectSightType tpSight;

	DEFINE_BVR(RecordID,idSkill);
	LevelSkillGrade grd;
	DEFINE_BVR(float,radius);
	DEFINE_BVR(float,radiusMin);
	float distKeep;
	float distLock;
	AnimTick durCheckEscape;
	StringID nmVar;
};


struct LevelRecordSkill;
class CBgn_Attack:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Attack);

	enum State
	{
		None,
		Attack,
		Escape,
	};

	enum DetectMethod
	{
		Detect_None=0,
		Detect_Sight,//在视野范围内重新搜索
		Detect_Range,//在技能的施放范围内重新搜索
		Detect_Closer,//在比_target更近的范围内重新搜索
	};


	CBgn_Attack()
	{
		_owner=NULL;
		_target=NULL;
		_fail=NULL;
		_state=None;
		_tLastCheckEscape=0;
		_tAttackTarget=0;
		_tFail=0;
		_verCast=0;
		_owner=NULL;
		_distOwnerSight=0.0f;

		_priorityCur=0;
	}



	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:
	void _Start(BGNOutputs &outputs,BOOL bCheckEscapce);

	CLevelObj *_DetectBest(CBgp_Attack*pad,DetectMethod method,DetectSightType tpSight,LevelRecordSkill *recSkill);

	LevelObjMapEnumCallBack _GetDetectDlgt();

	BOOL _CheckOwnerSight(CLevelObj *lo,float dist2);
	void _RecordTarget(CLevelObj *loTarget);

	State _state;
	AnimTick _tStart;//当前状态开始的时间

	AnimTick _durReach;//预期到达目的位置的时间(根据速度计算得到)

	CLevelObj *_target;
	AnimTick _tAttackTarget;//开始以_target为攻击目标的时间
	CLevelObj *_fail;
	AnimTick _tFail;
	AnimTick _tLastCheckEscape;//上一次检查要不要escape的时间
	BYTE _verCast;

	LevelPriority _priorityCur;

	CLevelObj *_owner;//临时指针,只在调用LevelUtil_DetectBestTarget期间有效,不带引用计数
	float _distOwnerSight;
};



class CBgp_AttackNoTarget:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_AttackNoTarget);

	virtual const char *GetTypeName()	{		return "原地施放技能";	}
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
		std::string nm=assist->GetSkillName(idSkill);
		if (!nm.empty())
		{
			FormatString(s,"原地施放技能[ %s ]",nm.c_str());
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_AttackNoTarget,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("使用技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
		GELEM_VAR_INIT(LevelSkillGrade,grd,LevelSkillGrade_Invalid);
			GELEM_EDITVAR("技能等级",GVT_B,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"该技能的等级");
    END_GOBJ();    

public: //当作protected

	RecordID idSkill;
	LevelSkillGrade grd;

};


class CBgn_AttackNoTarget:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_AttackNoTarget);

	CBgn_AttackNoTarget()
	{
		_bCasted=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

	BOOL _bCasted;
};


class CBgp_AttackTargetPos:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_AttackTargetPos);

	virtual const char *GetTypeName()	{		return "对目标位置施放技能";	}
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
		std::string nm=assist->GetSkillName(idSkill);
		if (!nm.empty())
		{
			FormatString(s,"针对目标位置施放技能[ %s ]",nm.c_str());
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_AttackTargetPos,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("使用技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
		GELEM_VAR_INIT(LevelSkillGrade,grd,LevelSkillGrade_Invalid);
			GELEM_EDITVAR("技能等级",GVT_B,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"该技能的等级");
		GELEM_BEHAVIORMEM_POS(varFixPos,"固定位置变量","使用那个变量里的位置")
		GELEM_VAR_INIT(BOOL,bWaitSkillFinish,TRUE);
			GELEM_EDITVAR("等待技能结束",GVT_S,GSem_Boolean,"等待技能结束");
    END_GOBJ();    

public: //当作protected

	RecordID idSkill;
	LevelSkillGrade grd;
	StringID varFixPos;
	BOOL bWaitSkillFinish;

};


class CBgn_AttackTargetPos:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_AttackTargetPos);

	CBgn_AttackTargetPos()
	{
		_bCasted=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

	BOOL _bCasted;
	BYTE _verCast;
};

class CBgp_AttackTargetFace:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_AttackTargetFace);

	virtual const char *GetTypeName()	{		return "对目标朝向施放技能";	}
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
		std::string nm=assist->GetSkillName(idSkill);
		if (!nm.empty())
		{
			FormatString(s,"向偏移%s度施放技能[ %s ]",GetBVRDesc_Float(BVR_ARG(yaw),assist),nm.c_str());
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_AttackTargetFace,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("使用技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
		GELEM_VAR_INIT(LevelSkillGrade,grd,LevelSkillGrade_Invalid);
			GELEM_EDITVAR("技能等级",GVT_B,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"该技能的等级");
		GELEM_VAR_INIT(LevelFaceYaw,yaw,0.0f);
			GELEM_EDITVAR("左右角度",GVT_F,GSem(GSem_Float,"-180,180.0,0.05"),"左右角度");
			GELEM_BVR()
    END_GOBJ();    

public: //当作protected

	RecordID idSkill;
	LevelSkillGrade grd;
	DEFINE_BVR(LevelFaceYaw,yaw);

};


class CBgn_AttackTargetFace:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_AttackTargetFace);

	CBgn_AttackTargetFace()
	{
		_bCasted=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

	BOOL _bCasted;
};



class CBgp_AttackTarget:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_AttackTarget);

	virtual const char *GetTypeName()	{		return "攻击指定目标";	}
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

    BEGIN_GOBJ_PURE_UID(CBgp_AttackTarget,1);
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


class CBgn_AttackTarget:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_AttackTarget);

	CBgn_AttackTarget()
	{
		_bCasted=FALSE;
		_idTarget=LevelObjID_Invalid;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

	BOOL _bCasted;
	LevelObjID _idTarget;
};





class CBgp_AttackHoldPos:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_AttackHoldPos);

	virtual const char *GetTypeName()	{		return "站定攻击";	}
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
		std::string nm=assist->GetSkillName(idSkill);
		if (!nm.empty())
		{
			FormatString(s,"使用[ %s ]技能攻击:\n%s",nm.c_str(),
				LevelDetectTargetFlags_GetName(BVR_ARG(flagsDetect)));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_AttackHoldPos,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("使用技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
			GELEM_BVR();
		GELEM_VAR_INIT(LevelSkillGrade,grd,LevelSkillGrade_Invalid);
			GELEM_EDITVAR("技能等级",GVT_B,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"该技能的等级");
		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"侦测什么类型的单位");
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(std::vector<LevelDetectTargetFlag>,flagsDetect);

	RecordID idSkill;
	LevelSkillGrade grd;
};


class CBgn_AttackHoldPos:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_AttackHoldPos);

	CBgn_AttackHoldPos()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

	BOOL _Check(CLevelObj *lo,float dist2);
};



class CBgp_CancelSkill:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_CancelSkill);

	virtual const char *GetTypeName()	{		return "取消技能";	}
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
		s="取消所有正在施放的技能";
		if (idSkills.size()>0)
		{
			static std::string ss;
			ss="";
			for (int i=0;i<idSkills.size();i++)
			{
				if (ss.empty())
					ss=assist->GetSkillName(idSkills[i]);
				else
					AppendFmtString(ss,",%s",assist->GetSkillName(idSkills[i]));
			}
			s="取消以下正在施放的技能:\n"+ss;
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CancelSkill,1);
		GELEM_BGP_BASE();
		GELEM_VARVECTOR_INIT(RecordID,idSkills,RecordID_Invalid);
			GELEM_EDITVAR("取消哪些技能",GVT_U,GSem(GSem_RecordID,"skills"),"取消哪些技能");
		GELEM_VAR_INIT(BOOL,bResetAct,TRUE);
			GELEM_EDITVAR("是否要重置动作",GVT_S,GSem_Boolean,"是否要重置动作");
    END_GOBJ();    

public: //当作protected

	std::vector<RecordID> idSkills;
	BOOL bResetAct;

};


class CBgn_CancelSkill:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CancelSkill);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:


};


class CBgp_CanCancelSkill:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_CanCancelSkill);

	virtual const char *GetTypeName()	{		return "检查能否取消技能";	}
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
		s="检查是否能够取消所有正在施放的技能";
		if (idSkills.size()>0)
		{
			static std::string ss;
			ss="";
			for (int i=0;i<idSkills.size();i++)
			{
				if (ss.empty())
					ss=assist->GetSkillName(idSkills[i]);
				else
					AppendFmtString(ss,",%s",assist->GetSkillName(idSkills[i]));
			}
			s="检查是否能够取消以下正在施放的技能:\n"+ss;
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CanCancelSkill,1);
		GELEM_BGP_BASE();
		GELEM_VARVECTOR_INIT(RecordID,idSkills,RecordID_Invalid);
			GELEM_EDITVAR("检查取消哪些技能",GVT_U,GSem(GSem_RecordID,"skills"),"检查取消哪些技能");
    END_GOBJ();    

public: //当作protected

	std::vector<RecordID> idSkills;
};


class CBgn_CanCancelSkill:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CanCancelSkill);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:


};

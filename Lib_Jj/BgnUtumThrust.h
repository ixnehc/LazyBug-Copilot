#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


struct CBgp_UtumThrust:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_UtumThrust);

	virtual const char *GetTypeName()	{		return "乌图姆冲刺";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="乌图姆冲刺";
	}

	BEGIN_GOBJ_PURE_UID(CBgp_UtumThrust,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(float,eulerThrust,0.0f);
			GELEM_EDITVAR("冲刺角度",GVT_F,GSem(GSem_Float,"0,6.28,0.1"),"冲刺角度,代表冲刺的方向");
			GELEM_BVR();

		GELEM_VAR_INIT(LevelObjID,idTarget,LevelObjID_Invalid);
			GELEM_EDITVAR("攻击对象ID", GVT_U, GSem_ObjID, "" );
			GELEM_BVR();

	END_GOBJ();

	DEFINE_BVR(float,eulerThrust);
	DEFINE_BVR(LevelObjID,idTarget);


};

struct SkillParam_UtumSummon;
class Buff_UtumBirth;
struct LevelRecordSkill;
class CBgn_UtumThrust:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_UtumThrust);

	enum Stage
	{
		None,
		Birth,
		Thrust,
		Attack,
		PostAttack,
		Return,
		DirectlyReturn,
	};

	CBgn_UtumThrust()
	{
		_tStart=0;
		_owner=NULL;
		_stage=None;
		_paramSummon=NULL;
		_recAttack=NULL;
		_tNextThrustUpdate=0;
		_euler=0.0f;
		_verCast=0;
		_bAttacked=FALSE;
		_durReturnFollow=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);


protected: 
	SkillParam_UtumSummon *_GetUtumSummonParam();
	Buff_UtumBirth *_GetBirthBuff();
	void _Update(AnimTick t,BGNOutputs &outputs);
	void _FireFail(BGNOutputs &outputs);

	CLevelObj *_Detect(BOOL bReturn);
	BOOL _DetectPos(LevelPos &pos);

	BOOL _MakeReturn(CLevelSkillDriver *driver,AnimTick t,BOOL bDirectlyReturn);
	BOOL _MakeAttack(CLevelSkillDriver *driver,CLevelObj *loTarget);

	CLevelObj *_owner;
	AnimTick _tStart;
	AnimTick _tNextThrustUpdate;
	LevelPos _posOrg;//开始冲刺瞬间的位置
	float _euler;
	SkillParam_UtumSummon *_paramSummon;
	LevelRecordSkill *_recAttack;
	Stage _stage;
	DWORD _verCast;
	BOOL _bAttacked;

	AnimTick _durReturnFollow;

};


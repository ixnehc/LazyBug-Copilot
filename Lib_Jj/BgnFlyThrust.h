#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


struct CBgp_FlyThrust:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_FlyThrust);

	virtual const char *GetTypeName()	{		return "飞行冲刺";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if ((_idSkill!=RecordID_Invalid)&&(_nmVar!=StringID_Invalid))
		{
			extern const char *GetBgnRegName(StringID nm);
			FormatString(s,"空中冲刺,使用[%s]技能攻击变量[%s]中的对象\n攻击范围(%.2f米~%.2f米)\n高度范围(%.2f米~%.2f米)",
				assist->GetSkillName(_idSkill),assist->GetStr(_nmVar),
				_rangeMin,_rangeMax,_rangeVerMin,_rangeVerMax);
		}
	}

	BEGIN_GOBJ_PURE_UID(CBgp_FlyThrust,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);
			GELEM_EDITVAR("施放技能",GVT_U,GSem(GSem_RecordID,"skills"),"施放的技能");
		GELEM_VAR_INIT(float,_rangeMin,1.0f);
			GELEM_EDITVAR("最小范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"至少要在这个范围以外");
		GELEM_VAR_INIT(float,_rangeMax,5.0f);
			GELEM_EDITVAR("最大范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"至少要在这个范围以内");
		GELEM_VAR_INIT(float,_rangeVerMin,1.0f);
			GELEM_EDITVAR("最低范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"至少要在这个(相对)高度以上");
		GELEM_VAR_INIT(float,_rangeVerMax,5.0f);
			GELEM_EDITVAR("最高范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"至少要在这个(相对)高度以下");
		GELEM_BEHAVIORMEM_OBJID(_nmVar,"变量名称","从哪个变量中取得冲刺攻击的对象")

	END_GOBJ();

	RecordID _idSkill;
	float _rangeMin;
	float _rangeMax;
	float _rangeVerMin;
	float _rangeVerMax;
	StringID _nmVar;

};


class CBgn_FlyThrust:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_FlyThrust);

	CBgn_FlyThrust()
	{
		_tStart=0;
		_bAttacking=FALSE;
		_owner=NULL;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected: 
	void _Update(AnimTick t,BGNOutputs &outputs);
	void _FireFail(BGNOutputs &outputs);
	CLevelObj *_owner;
	LevelObjID _idTarget;
	AnimTick _tStart;
	BOOL _bAttacking;

};


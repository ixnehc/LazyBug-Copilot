#pragma once

#include "behaviorgraph/BehaviorGraphPads.h"
#include "LevelBehavior.h"

#include "LevelDefines.h"

extern const char *GetAgentsSemConstraint();

struct CBgp_Reside:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Reside);

	virtual const char *GetTypeName()	{		return "驻留";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"驻留成功");
			STUB_OUT(2,"无法驻留");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if ((_idAgent!=RecordID_Invalid)&&(_idSkill!=RecordID_Invalid))
		{
			extern const char *AgentNameFromUID(int uid);
			FormatString(s,"尝试驻留进%.2f米范围内的[%s]\n采用技能:[%s]\n尝试%.2f秒",_radius,assist->GetAgentName(_idAgent),
				assist->GetSkillName(_idSkill),ANIMTICK_TO_SECOND(_tSearching));
		}
	}

public:

	RecordID _idAgent;
	RecordID _idSkill;
	float _radius;
	AnimTick _tSearching;

	BEGIN_GOBJ_PURE_UID(CBgp_Reside,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,_idAgent,RecordID_Invalid);
			GELEM_EDITVAR("驻留对象",GVT_U,GSem(GSem_RecordID,"agents"),"驻留对象");
		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);
			GELEM_EDITVAR("驻留使用的技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
		GELEM_VAR_INIT(float,_radius,12.0f);
			GELEM_EDITVAR("侦测半径",GVT_F,GSem(GSem_Float,"0,20,0.1"),"搜索驻留对象时的侦测半径");
		GELEM_VAR_INIT(AnimTick,_tSearching,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("搜索时间",GVT_U,GSem(GSem_AnimTick,"1,100,0.1"),"搜索多长时间找不到算失败,单位为秒");
	END_GOBJ();
};




//攻击
class CBgn_Reside:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Reside);

	CBgn_Reside()
	{
		_tStart=0;
		_token=LevelObjSeatToken_Invalid;
		_target=NULL;
	}

	virtual void Destroy();

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);

protected:

	BOOL _DoReside();

	void _ClearTarget();
	CLevelObj *_target;
	LevelObjSeatToken _token;
	AnimTick _tStart;



};


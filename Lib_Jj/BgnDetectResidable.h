#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


extern const char *GetAgentsSemConstraint();
extern const char *AgentNameFromUID(int uid);

class CBgp_DetectResidable:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_DetectResidable);

	virtual const char *GetTypeName()	{		return "侦测可驻留对象";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"侦测到");
			STUB_OUT(2,"未侦测到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		const char *GetBgnRegName(StringID nm);
		if (_idAgent!=RecordID_Invalid)
			FormatString(s,"侦测%.2f米范围内的[%s]",_radius,assist->GetAgentName(_idAgent));

		if (_nmVar!=StringID_Invalid)
		{
			AppendFmtString(s,"\n结果保存在变量[%s]中",assist->GetStr(_nmVar));
		}

	}

    BEGIN_GOBJ_PURE_UID(CBgp_DetectResidable,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,_idAgent,RecordID_Invalid);
			GELEM_EDITVAR("驻留对象",GVT_U,GSem(GSem_RecordID,"agents"),"驻留对象");
		GELEM_VAR_INIT(float,_radius,12.0f);
			GELEM_EDITVAR("侦测半径",GVT_F,GSem(GSem_Float,"0,20,0.1"),"搜索驻留对象时的侦测半径");
		GELEM_BEHAVIORMEM_OBJID(_nmVar,"保存变量","侦测到的对象保存在那个变量中")
	END_GOBJ();    

public: //当作protected
	float _radius;
	RecordID _idAgent;
	StringID _nmVar;
};


class CBgn_DetectResidable:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_DetectResidable);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

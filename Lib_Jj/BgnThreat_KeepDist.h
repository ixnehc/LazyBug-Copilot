#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpThreat_KeepDist:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_KeepDist);

	virtual const char *GetTypeName()	{		return "与Threat保持距离";	}
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
		if (distMin<distMax)
		{
			FormatString(s,"\n保持距离%.2f米~%.2f米,每%.2f秒检查一次",distMin,distMax,ANIMTICK_TO_SECOND(durCheck));
			if(bKeepFacingThreat)
				s+="\n保持面朝Threat";
			if (radiusToOwner>0.0f)
				AppendFmtString(s,"限制在主人%.2f米范围内",radiusToOwner);
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_KeepDist,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(float,distMin,3.0f);
			GELEM_EDITVAR("最小距离",GVT_F,GSem(GSem_Float,"0,20,0.1"),"与Threat的最小距离");
		GELEM_VAR_INIT(float,distMax,6.0f);
			GELEM_EDITVAR("最大距离",GVT_F,GSem(GSem_Float,"0,20,0.1"),"与Threat的最大距离");
		GELEM_VAR_INIT(AnimTick,durCheck,ANIMTICK_FROM_SECOND(5.0f));
			GELEM_EDITVAR("保持距离检测间隔",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"多长时间检测一次保持距离,单位为秒");	
		GELEM_VAR_INIT(BOOL,bKeepFacingThreat,TRUE);
			GELEM_EDITVAR("是否持续面向Threat",GVT_S,GSem_Boolean,"是否持续面向Threat");
		GELEM_VAR_INIT(float,radiusToOwner,0.0f);
			GELEM_EDITVAR("到主人的距离限制",GVT_F,GSem(GSem_Float,"0,20,0.1"),"最为随从,到主人的距离限制,0表示没有限制");
    END_GOBJ();    

public: //当作protected

	float distMin;
	float distMax;
	AnimTick durCheck;
	BOOL bKeepFacingThreat;
	float radiusToOwner;
};


struct LevelRecordSkill;
class CBgnThreat_KeepDist:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_KeepDist);

	enum State
	{
		None,
		Follow,
		Escape,
	};

	CBgnThreat_KeepDist()
	{
		_target=NULL;
		_state=None;
		_tLastCheck=0;
	}



	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:
	void _Start(CLevelObj *target,BOOL bCheckEscapce);
	void _Stop();
	BOOL _IsTooClose(CLevelObj *lo,CLevelObj *target);
	BOOL _IsTooFar(CLevelObj *lo,CLevelObj *target);
	BOOL _IsFarEnough(CLevelObj *lo,CLevelObj *target);
	BOOL _IsCloseEnough(CLevelObj *lo,CLevelObj *target);

	void _Respond(CLevelObj *lo,CLevelObj *target);

	CLevelObj *_target;

	State _state;
	AnimTick _tStateStart;//当前状态开始的时间

	AnimTick _tLastCheck;//上一次检查要不要escape的时间

};


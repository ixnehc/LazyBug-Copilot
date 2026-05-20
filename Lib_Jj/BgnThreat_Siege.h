#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpThreat_Siege:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_Siege);

	virtual const char *GetTypeName()	{		return "对Threat包围";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
			STUB_OUT(2,"中断");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Threat;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";

		FormatString(s,"尝试在[%.2f米~%.2f米]范围内包围Threat%.2f(+/-%.2f)秒,移动速度为:%.2f米/秒",
			distMin,distMax,ANIMTICK_TO_SECOND(dur),ANIMTICK_TO_SECOND(durVary),speed);
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_Siege,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(float,distMin,3.0f);
			GELEM_EDITVAR("最小距离",GVT_F,GSem(GSem_Float,"0,20,0.1"),"与Threat的最小距离");
		GELEM_VAR_INIT(float,distMax,6.0f);
			GELEM_EDITVAR("最大距离",GVT_F,GSem(GSem_Float,"0,20,0.1"),"与Threat的最大距离");

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(4.0f));
			GELEM_EDITVAR("包围持续多长时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"包围持续多长时间,单位为秒");
		GELEM_VAR_INIT(AnimTick,durVary,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("包围持续时间浮动值",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"包围持续时间的浮动值");
		GELEM_VAR_INIT(float,speed,2.0f);
			GELEM_EDITVAR("包围时的移动速度",GVT_F,GSem(GSem_Float,"0,100,0.1"),"包围时的移动速度");

		GELEM_VAR_INIT(BOOL,bFaceTarget,TRUE);
			GELEM_EDITVAR("面向Threat",GVT_S,GSem_Boolean,"是否面向Threat");
    END_GOBJ();    

public: //当作protected

	float distMin;
	float distMax;

	AnimTick dur;
	AnimTick durVary;
	float speed;
	BOOL bFaceTarget;
};

struct AttrNodeBase;
class CBgnThreat_Siege:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_Siege);

	CBgnThreat_Siege()
	{
		_target=NULL;
		_attrnode=NULL;
		_bRight=0;
		_bSiegePos=0;

		_radius=0.0f;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:

	void _Finish(BOOL bStopMove);

	BOOL _Step(BOOL bFirstStep);

	CLevelObj *_target;
	AttrNodeBase *_attrnode;

	LevelPos _posTargetRecent;

	BYTE _bRight;//往哪边包围
	BYTE _bSiegePos;//_posSiege是否有效
	LevelPos _posSiege;
	float _radSiegeTolerance;//要转到的角度

	AnimTick _tStart;
	AnimTick _dur;


	float _radius;

};


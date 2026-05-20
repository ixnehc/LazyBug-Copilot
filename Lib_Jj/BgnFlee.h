#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "attr/attr.h"


class CBgp_Flee:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Flee);

	virtual const char *GetTypeName()	{		return "逃跑";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (dur!=0)
			FormatString(s,"在%.2f米的范围内躲避:\n%s\n逃跑距离:%.2f米\n持续%.2f秒",
			radius,LevelDetectTargetFlags_GetName(BVR_ARG(flagsDetect)),dist,ANIMTICK_TO_SECOND(dur));
		else
			FormatString(s,"在%.2f米的范围内躲避:\n%s\n逃跑距离:%.2f米\n永久持续",
			radius,LevelDetectTargetFlags_GetName(BVR_ARG(flagsDetect)),dist);
		if (speed>0.0f)
			AppendFmtString(s,"\n移动速度:%.2f米/秒",speed);
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Flee,1);
		GELEM_BGP_BASE();
		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"侦测什么类型的单位");
			GELEM_BVR();
		GELEM_VAR_INIT(float,radius,5.0f);
			GELEM_EDITVAR("侦测半径",GVT_F,GSem(GSem_Float,"0,50,0.1"),"敌人进入多远范围内开始逃跑");
		GELEM_VAR_INIT(float,dist,3.0f);
			GELEM_EDITVAR("逃跑距离",GVT_F,GSem(GSem_Float,"0,50,0.1"),"每次逃多远的距离.");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续逃跑多长时间,0表示永久持续");
		GELEM_VAR_INIT(float,radiusFollow,0.0f);
			GELEM_EDITVAR("跟随半径(LockPlayer)",GVT_F,GSem(GSem_Float,"0,20,0.1"),"跟随在LockPlayer多大的半径范围内,0表示不跟随");
		GELEM_VAR_INIT(BOOL,bQuitWhenOutOfRange,FALSE);
			GELEM_EDITVAR("跑出范围后立即结束",GVT_S,GSem_Boolean,"跑出范围后立即结束");
		GELEM_VAR_INIT(float,speed,0.0f);
			GELEM_EDITVAR("速度",GVT_F,GSem(GSem_Float,"0,20,0.1"),"移动速度,0表示使用缺省速度");
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(std::vector<LevelDetectTargetFlag>,flagsDetect);
	float radius;
	float dist;
	AnimTick dur;
	float radiusFollow;
	BOOL bQuitWhenOutOfRange;
	float speed;

};

class CBgn_Flee:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Flee);

	CBgn_Flee()
	{
		_tStart=0;
		_bFinished=FALSE;
		_signAvoid=0;
		_nCDFrames=0;
		_tResetAvoidSign=0;
		_anSpeed=NULL;
	}

	virtual void Destroy();

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

	BOOL IsTimeUp()	{		return _bFinished;	}


protected:

	void _UpdateFlee(AnimTick t);

	AnimTick _tStart;
	BOOL _bFinished;
	int _signAvoid;

	int _nCDFrames;
	AnimTick _tResetAvoidSign;

	AttrNodeFloat *_anSpeed;


};

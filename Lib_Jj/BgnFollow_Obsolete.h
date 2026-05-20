#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

class CBgp_Follow_Obsolete:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Follow_Obsolete);

	virtual const char *GetTypeName()	{		return "跟随-Obsolete";	}
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
		if (bLockPlayer)
		{
			if (dur!=0)
				FormatString(s,"跟随%.2f米范围内锁定的玩家\n跟随距离:%.2f米\n持续%.2f秒",
				radius,dist,ANIMTICK_TO_SECOND(dur));
			else
				FormatString(s,"跟随%.2f米范围内锁定的玩家\n跟随距离:%.2f米\n永久持续",
				radius,dist);
		}
		else
		{
			if (dur!=0)
				FormatString(s,"跟随%.2f米范围内的:\n%s\n跟随距离:%.2f米\n持续%.2f秒",
				radius,LevelDetectTargetFlags_GetName(BVR_ARG(flagsDetect)),dist,ANIMTICK_TO_SECOND(dur));
			else
				FormatString(s,"跟随%.2f米范围内的:\n%s\n跟随距离:%.2f米\n永久持续",
				radius,LevelDetectTargetFlags_GetName(BVR_ARG(flagsDetect)),dist);
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Follow_Obsolete,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(BOOL,bLockPlayer,0);
			GELEM_EDITVAR("跟随锁定的玩家",GVT_S,GSem_Boolean,"是否跟随锁定的玩家,如果为FALSE,则自动侦测跟随的对象");
		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"侦测什么类型的单位");
			GELEM_BVR();
		GELEM_VAR_INIT(float,radius,10.0f);
			GELEM_EDITVAR("侦测半径",GVT_F,GSem(GSem_Float,"0,20,0.1"),"敌人进入多远范围内开始跟随");
		GELEM_VAR_INIT(float,dist,5.0f);
			GELEM_EDITVAR("跟随距离",GVT_F,GSem(GSem_Float,"0,20,0.1"),"跟随时保持的距离");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续跟随多长时间,0表示永久持续");
    END_GOBJ();    

public: //当作protected

	BOOL bLockPlayer;
	DEFINE_BVR(std::vector<LevelDetectTargetFlag>,flagsDetect);
	float radius;
	float dist;
	AnimTick dur;
};


class CBgn_Follow_Obsolete:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Follow_Obsolete);

	CBgn_Follow_Obsolete()
	{
		_tStart=0;
		_bTimeUp=FALSE;
		_idFollow=LevelObjID_Invalid;
		_tFollow=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

	BOOL IsTimeUp()	{		return _bTimeUp;	}

	AnimTick GetIdleTime(AnimTick t)//返回有多长时间没有找到逃避的对象了
	{
		return ANIMTICK_SAFE_MINUS(t,_tFollow);
	}


protected:

	void _UpdateFollow(AnimTick t);

	AnimTick _tFollow;//上一次丢失攻击对象的时间
	AnimTick _tStart;
	BOOL _bTimeUp;

	LevelObjID _idFollow;

};


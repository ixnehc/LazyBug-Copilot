#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

class CBgp_Follow:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Follow);

	virtual const char *GetTypeName()	{		return "跟随";	}
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
			FormatString(s,"跟随 %s \n跟随距离:%.2f米\n持续%.2f秒",assist->GetStr(nmVar),dist,ANIMTICK_TO_SECOND(dur));
		else
			FormatString(s,"跟随 %s \n跟随距离:%.2f米\n永久持续",assist->GetStr(nmVar),dist);
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Follow,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_OBJID(nmVar,"跟随目标变量","跟随目标")
		GELEM_VAR_INIT(float,dist,5.0f);
			GELEM_EDITVAR("跟随距离",GVT_F,GSem(GSem_Float,"0,20,0.1"),"跟随时保持的距离");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续跟随多长时间,0表示永久持续");
		GELEM_VAR_INIT(BOOL,bClosestFollow,TRUE);
			GELEM_EDITVAR("跟随时尽量靠近目标",GVT_S,GSem_Boolean,"跟随时尽量靠近目标");
		GELEM_VAR_INIT(BOOL,bFinishWhenReached,FALSE);
			GELEM_EDITVAR("到达范围内结束",GVT_S,GSem_Boolean,"进入范围后结束");
    END_GOBJ();    

public: //当作protected

	float dist;
	AnimTick dur;
	BOOL bFinishWhenReached;
	BOOL bClosestFollow;

	StringID nmVar;

};


class CBgn_Follow:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Follow);

	CBgn_Follow()
	{
		_tStart=0;
		_idFollow=LevelObjID_Invalid;
	}

	void Destroy() override;

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

	AnimTick GetIdleTime(AnimTick t)//返回有多长时间没有找到逃避的对象了
	{
		return ANIMTICK_SAFE_MINUS(t,_tFollow);
	}


protected:

	void _UpdateFollow(AnimTick t);

	AnimTick _tFollow;//上一次丢失攻击对象的时间
	AnimTick _tStart;

	LevelObjID _idFollow;

};


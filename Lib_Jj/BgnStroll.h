#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_Stroll:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Stroll);

	virtual const char *GetTypeName()	{		return "闲逛";	}
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
		FormatString(s,"间隔%.2f (+/- %.2f)秒移动一次\n范围:%.2f米",ANIMTICK_TO_SECOND(gap),
			ANIMTICK_TO_SECOND(gapVary),range);
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Stroll,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(AnimTick,gap,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("间隔时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"每次移动的间隔时间");
		GELEM_VAR_INIT(AnimTick,gapVary,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("间隔时间上下浮动",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"每次移动的间隔时间的浮动值");
		GELEM_VAR_INIT(float,range,2.0f);
			GELEM_EDITVAR("移动距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"每次移动的距离的范围");
    END_GOBJ();    

public: //当作protected

	AnimTick gap;
	AnimTick gapVary;
	float range;

};


class CBgn_Stroll:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Stroll);

	CBgn_Stroll()
	{
		_tNextMove=ANIMTICK_INFINITE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

	AnimTick _tNextMove;

};

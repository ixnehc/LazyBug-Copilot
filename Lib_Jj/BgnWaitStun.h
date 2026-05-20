#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_WaitStun:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_WaitStun);

	virtual const char *GetTypeName()	{		return "等待硬直结束";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Buff;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_dur<=0)
			s="永久等待硬直结束";
		else
			FormatString(s,"等待硬直结束,等待%.2f秒",ANIMTICK_TO_SECOND(_dur));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_WaitStun,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(AnimTick,_dur,ANIMTICK_FROM_SECOND(5.0f));
			GELEM_EDITVAR("等待时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"0表示永远等待");
		GELEM_OBJ(WeaksEx,_weaksFilter);
			GELEM_EDITOBJ("等待时的弱点","等待时的弱点");
	END_GOBJ();    

public: //当作protected
	AnimTick _dur;
	WeaksEx _weaksFilter;
};


class CBgn_WaitStun:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_WaitStun);

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;
	virtual void Update(BGNOutputs &outputs) override;
	virtual void Break(BGNOutputs &outputs) override;

protected:
	BOOL _CheckStun();
	void _ClearFilter();

	AnimTick _tStart;

};

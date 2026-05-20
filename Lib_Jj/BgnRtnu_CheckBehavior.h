#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgpRtnu_CheckBehavior:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpRtnu_CheckBehavior);

	virtual const char *GetTypeName()	{		return "检查随从Behavior";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"是");
			STUB_OUT(2,"否");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Rtnu;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"检查当前随从的Behavior是否为:%s",GetRtnuBehaviorName(_bhv));
	}

    BEGIN_GOBJ_PURE_UID(CBgpRtnu_CheckBehavior,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(LevelRtnuBehavior,_bhv,LevelRtnuBehavior_Accompany);
			GELEM_EDITVAR("Behavior",GVT_U,GSem(GSem_Interger,LevelRtnuBehavior_SemConstraint),"要检查的随从Behavior");

    END_GOBJ();    

public: //当作protected
	LevelRtnuBehavior _bhv;
};

struct AttrNodeBase;
class CBgnRtnu_CheckBehavior:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnRtnu_CheckBehavior);

	CBgnRtnu_CheckBehavior()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);


protected:



};

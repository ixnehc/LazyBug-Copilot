#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgpRtnu_Accompany:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpRtnu_Accompany);

	virtual const char *GetTypeName()	{		return "伴随";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Rtnu;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"持续伴随锁定的玩家");
	}

    BEGIN_GOBJ_PURE_UID(CBgpRtnu_Accompany,1);
		GELEM_BGP_BASE();

    END_GOBJ();    

public: //当作protected
};

struct AttrNodeBase;
class CBgnRtnu_Accompany:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnRtnu_Accompany);

	CBgnRtnu_Accompany()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Destroy();
	virtual void Break(BGNOutputs &outputs);


protected:



};

#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgpRtnu_CheckIsRtnu:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpRtnu_CheckIsRtnu);

	virtual const char *GetTypeName()	{		return "检查是否为随从";	}
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
		s="检查自己是否为随从";
	}

    BEGIN_GOBJ_PURE_UID2(CBgpRtnu_CheckIsRtnu,438,1);
		GELEM_BGP_BASE();


    END_GOBJ();    

public: //当作protected
};

struct AttrNodeBase;
class CBgnRtnu_CheckIsRtnu:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnRtnu_CheckIsRtnu);

	CBgnRtnu_CheckIsRtnu()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);


protected:



};

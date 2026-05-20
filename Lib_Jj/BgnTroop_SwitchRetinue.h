#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "LevelAIContext.h"

struct AttrNodeFloat;
class CBgpTroop_SwitchRetinue:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpTroop_SwitchRetinue);

	virtual const char *GetTypeName()	{		return "Troop切换随从";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"将Troop(%s)内单位切换成随从",
			GetBVRDesc_StringID(BVR_ARG(_troop),assist));
	}

    BEGIN_GOBJ_PURE_UID(CBgpTroop_SwitchRetinue,1);
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","组建哪个Troop");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(StringID,_troop);

};


class CBgnTroop_SwitchRetinue:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_SwitchRetinue);

	CBgnTroop_SwitchRetinue()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Destroy();

protected:

};


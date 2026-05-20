#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_RevealMap:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_RevealMap);

	virtual const char *GetTypeName()	{		return "打开地图区域";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

    BEGIN_GOBJ_PURE(CBgpGA_RevealMap,1);

		GELEM_OBJ(BP_Area,area);
			GELEM_EDITOBJ("打开区域","打开区域");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(BP_Area, area);

};


class CBgnGA_RevealMap:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_RevealMap);

	CBgnGA_RevealMap()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


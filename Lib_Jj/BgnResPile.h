#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgp_ResPile:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_ResPile);

	virtual const char *GetTypeName()	{		return "生成资源堆";	}
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
		if (tpRes==LevelResource_None)
			s="n/a";
		else
		{
			FormatString(s,"生成一个属于自己的资源堆(%s个%s)",GetBVRDesc_Int(BVR_ARG(amount),assist),NameFromResourceType(tpRes));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_ResPile,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelResourceType,tpRes,LevelResource_None);
			GELEM_EDITVAR("资源类型",GVT_S,GSem(GSem_Interger,LevelResourceType_SemConstraint),"资源类型");
		GELEM_VAR_INIT(int,amount,1);
			GELEM_EDITVAR("数量",GVT_S,GSem_Interger,"数量");
			GELEM_BVR();
    END_GOBJ();    

public: //当作protected

	LevelResourceType tpRes;
	DEFINE_BVR(int,amount);
};


class CBgn_ResPile:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_ResPile);

	CBgn_ResPile()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

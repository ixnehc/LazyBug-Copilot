#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgp_CheckResPile:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_CheckResPile);

	virtual const char *GetTypeName()	{		return "检测资源堆";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (tpRes==LevelResource_None)
			s="n/a";
		else
		{
			FormatString(s,"检测是否有一个属于自己的资源堆(%s)",NameFromResourceType(tpRes));
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_CheckResPile,434,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelResourceType,tpRes,LevelResource_None);
			GELEM_EDITVAR("资源类型",GVT_S,GSem(GSem_Interger,LevelResourceType_SemConstraint),"资源类型");
    END_GOBJ();    

public: //当作protected

	LevelResourceType tpRes;
};


class CBgn_CheckResPile:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckResPile);

	CBgn_CheckResPile()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

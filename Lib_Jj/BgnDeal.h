#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelDeal.h"


class CBgp_Deal:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Deal);

	virtual const char *GetTypeName()	{		return "结算";	}
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
		extern const char *GetBgnRegName(StringID nm);

		s="结算";

	}

    BEGIN_GOBJ_PURE_UID(CBgp_Deal,1);
		GELEM_BGP_BASE();

		GELEM_OBJVECTOR(DealEntry,deals); 
			GELEM_EDITOBJ("结算列表","多个结算");

	END_GOBJ();    

public: //当作protected
	std::vector<DealEntry> deals;

};


class CBgn_Deal:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Deal);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_Centipede_Stretch:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Centipede_Stretch);

	virtual const char *GetTypeName()	{		return "巨蜗_蜈蚣_Stretch";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_bStretchOut)
			s="伸出蜈蚣";
		else
			s="缩回蜈蚣";
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Centipede_Stretch,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(BOOL,_bStretchOut,TRUE);
			GELEM_EDITVAR("伸展出来",GVT_S,GSem_Boolean,"伸展出来");
    END_GOBJ();    

public: //当作protected
	BOOL _bStretchOut;
	

};


class CBgn_Centipede_Stretch:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Centipede_Stretch);

	CBgn_Centipede_Stretch()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:
};


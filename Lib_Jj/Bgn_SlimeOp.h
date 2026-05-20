#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_SlimeOp:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_SlimeOp);

	virtual const char *GetTypeName()	{		return "SlimeOp";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	enum Type
	{
		CheckReady,

		ForceDword=0xffffffff,
	};

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (tp==CheckReady)
			s="检测是否处于Ready状态";
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_SlimeOp,486,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(DWORD,tp,CheckReady);
			GELEM_EDITVAR("Op类型",GVT_U,GSem(GSem_Interger,"坚持是否Ready状态"),"Op类型");

    END_GOBJ();    

public: //当作protected

	Type tp;

};


class CBgn_SlimeOp:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SlimeOp);

	CBgn_SlimeOp()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:
};


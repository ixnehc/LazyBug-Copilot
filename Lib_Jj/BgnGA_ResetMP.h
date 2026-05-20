#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_ResetMP:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_ResetMP);

	virtual const char *GetTypeName()	{		return "重置MP";	}
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
		s="重置MP";
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_ResetMP,453,1);
	END_GOBJ();    

public: //当作protected

};


class CBgnGA_ResetMP:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_ResetMP);

	CBgnGA_ResetMP()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


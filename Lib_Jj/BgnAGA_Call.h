#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"



class CBgpAGA_Call:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpAGA_Call);

	virtual const char *GetTypeName()	{		return "调用Agent函数";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_AGA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

    BEGIN_GOBJ_PURE_UID(CBgpAGA_Call,1);
    END_GOBJ();    

public: //当作protected

};


class CBgnAGA_Call:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnAGA_Call);

	CBgnAGA_Call()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


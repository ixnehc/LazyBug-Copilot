#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_Revive:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_Revive);


	virtual const char *GetTypeName()	{		return "复活自己";	}
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
		s="复活自己";
	}

    BEGIN_GOBJ_PURE_UID(CBgpGA_Revive,1);
	END_GOBJ();    

public: //当作protected
};


class CBgnGA_Revive:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_Revive);

	CBgnGA_Revive()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


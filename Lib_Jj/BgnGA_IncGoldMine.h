#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorParam.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "records/recordsdefine.h"


class CBgpGA_IncGoldMine:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_IncGoldMine);

	virtual const char *GetTypeName()	{		return "增加金矿";	}
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
		s="增加一个金矿";

	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_IncGoldMine,432,1);


    END_GOBJ();    

public: //当作protected


};


class CBgnGA_IncGoldMine:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_IncGoldMine);

	CBgnGA_IncGoldMine()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


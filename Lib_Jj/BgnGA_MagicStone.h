#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgpGA_MagicStoneReward:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_MagicStoneReward);

	virtual const char *GetTypeName()	{		return "魔法石_奖励";	}
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
		s="魔法石奖励(针对TalkPlayer)";
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_MagicStoneReward,454,1);

    END_GOBJ();    

public: //当作protected

};


class CBgnGA_MagicStoneReward:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_MagicStoneReward);

	CBgnGA_MagicStoneReward()
	{
	}

	void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:



};


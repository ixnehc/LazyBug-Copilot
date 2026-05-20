#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_ExchangeItem:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_ExchangeItem);

	virtual const char *GetTypeName()	{		return "交换道具";	}
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
		FormatString(s,"将道具[%s]赋予TalkPlayer,如果需要,会将玩家身上的装备交换到变量[%s]中",assist->GetStr(varItem),assist->GetStr(varItem));
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_ExchangeItem,478,1);

		GELEM_BEHAVIORMEM_ITEMRECORD(varItem,"道具变量","道具的ID保存在那个变量中")

    END_GOBJ();    

public: //当作protected

	StringID varItem;
};


class CBgnGA_ExchangeItem:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_ExchangeItem);

	CBgnGA_ExchangeItem()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


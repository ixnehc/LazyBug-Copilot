#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h" 


class CBgpGA_断桥_StartRepair:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_断桥_StartRepair);

	virtual const char *GetTypeName()	{		return "断桥_StartRepair";	}
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

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="修复断桥";

	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_断桥_StartRepair,436,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(int,_nRequiredLabor,20);
			GELEM_EDITVAR("需要Labor个数",GVT_S,GSem_Interger,"需要Labor个数");

    END_GOBJ();    

public: //当作protected
	int _nRequiredLabor;


};

class RollAwardsResult;
class CBgnGA_断桥_StartRepair:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_断桥_StartRepair);

	CBgnGA_断桥_StartRepair()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;

public:

};


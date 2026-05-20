#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_Centipede_GetInfo:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Centipede_GetInfo);

	virtual const char *GetTypeName()	{		return "巨蜗_蜈蚣_GetInfo";	}
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
		UnbrokenRate,

		ForceDword=0xffffffff,
	};

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (var!=StringID_Invalid)
		{
			if (tp==UnbrokenRate)
				s="得到蜈蚣未打断部分的比率";

			AppendFmtString(s,",保存在变量[%s]中",StrLib_GetStr(var));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Centipede_GetInfo,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(DWORD,tp,UnbrokenRate);
			GELEM_EDITVAR("Info类型",GVT_U,GSem(GSem_Interger,"未打断部分的比率"),"Info类型");

		GELEM_BEHAVIORMEM_NUMBER(var,"保存变量","存入哪个变量中")
    END_GOBJ();    

public: //当作protected

	Type tp;
	
	StringID var;

};


class CBgn_Centipede_GetInfo:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Centipede_GetInfo);

	CBgn_Centipede_GetInfo()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:
};


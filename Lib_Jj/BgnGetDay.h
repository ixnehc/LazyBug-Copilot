#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_GetDay:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_GetDay);


	virtual const char *GetTypeName()	{		return "得到当前天数";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (var==StringID_Invalid)
			s="n/a";
		else
		{
			FormatString(s,"当前天数存入变量%s",assist->GetStr(var));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_GetDay,1);
		GELEM_BGP_BASE();

		GELEM_BEHAVIORMEM_NUMBER(var,"游戏对象变量","存入哪个变量中")
	END_GOBJ();    

public: //当作protected
	StringID var;
};


class CBgn_GetDay:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_GetDay);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgp_RandomizeVar:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_RandomizeVar);

	virtual const char *GetTypeName()	{		return "产生随机数";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (nm!=StringID_Invalid)
		{
			if (!bFloat)
				FormatString(s,"产生随机数保存到变量(%s), [%d,%d]",assist->GetStr(nm),low,hi);
			else
				FormatString(s,"产生随机数保存到变量(%s), [%.3f,%.3f]",assist->GetStr(nm),fLow,fHi);
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_RandomizeVar,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_NUMBER(nm,"名称","行为图内存变量名称")
		GELEM_VAR_INIT(BOOL,bFloat,FALSE);
			GELEM_EDITVAR("数值类型",GVT_S,GSem(GSem_Interger,
				"整数:0"		"|最小值(浮点数)&最大值(浮点数),"
				"浮点数:1"	"|最小值&最大值"
				),"数值类型");
		GELEM_VAR_INIT(int,low,0);
			GELEM_EDITVAR("最小值",GVT_S,GSem_Interger,"最小值");
		GELEM_VAR_INIT(int,hi,5);
			GELEM_EDITVAR("最大值",GVT_S,GSem_Interger,"最大值");
		GELEM_VAR_INIT(float,fLow,0.0f);
			GELEM_EDITVAR("最小值(浮点数)",GVT_F,GSem(GSem_Float,"-10000,10000,0.001"),"最小值");
		GELEM_VAR_INIT(float,fHi,5.0f);
			GELEM_EDITVAR("最大值(浮点数)",GVT_F,GSem(GSem_Float,"-10000,10000,0.001"),"最大值");
    END_GOBJ();    

public: //当作protected

	StringID nm;
	BOOL bFloat;
	int low;
	int hi;
	float fLow;
	float fHi;
};


class CBgn_RandomizeVar:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_RandomizeVar);

	CBgn_RandomizeVar()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

